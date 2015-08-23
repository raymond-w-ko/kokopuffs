#pragma once

#include <stdint.h>
#include <memory>
#include <exception>
#include <cassert>

#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(_DEBUG) || defined(DEBUG) && !defined(NDEBUG) && !defined(_NDEBUG)
#define KOKOPUFFS_DEBUG
#endif
#define KOKOPUFFS_MAP_COLLISION_DEBUG

// default factor used in Google's densehashtable.h
#define KOKOPUFFS_MAP_INTIAL_SIZE 16
#define KOKOPUFFS_MAP_DEFAULT_MAX_LOAD_FACTOR 0.5f
#define KOKOPUFFS_MAP_DEFAULT_MIN_LOAD_FACTOR 0.2f


namespace kokopuffs {

template<typename Key, typename Value>
class map {
 public:
  struct Entry {
    Key key;
    Value value;
#ifdef KOKOPUFFS_MAP_COLLISION_DEBUG
    size_t intended_bucket;
#endif
  };

  map(const size_t initial_table_size = KOKOPUFFS_MAP_INTIAL_SIZE)
      : bucket_count_(initial_table_size),
        item_count_(0),
        max_load_factor_(KOKOPUFFS_MAP_DEFAULT_MAX_LOAD_FACTOR),
        min_load_factor_(KOKOPUFFS_MAP_DEFAULT_MIN_LOAD_FACTOR)
#ifdef KOKOPUFFS_DEBUG
        , has_set_empty_key_(false)
        , has_set_deleted_key_(false)
#endif
  {
    table_ = create_table(bucket_count_);
  }

  map(const map<Key, Value>& other)
      : bucket_count_(other.bucket_count_),
        item_count_(0),
        max_load_factor_(other.max_load_factor_),
        min_load_factor_(other.min_load_factor_)
#ifdef KOKOPUFFS_DEBUG
        , has_set_empty_key_(other.has_set_empty_key_)
        , has_set_deleted_key_(other.has_set_deleted_key_)
#endif
  {
    table_ = create_table(bucket_count_);

    if (other.empty_key_) {
      empty_key_ = std::unique_ptr<Key>(new Key(*other.empty_key_));
    }
    if (other.deleted_key_) {
      deleted_key_ = std::unique_ptr<Key>(new Key(*other.deleted_key_));
    }
    _copy_elements_from_table(
        other.table_, other.bucket_count_,
        *other.empty_key_,
        static_cast<bool>(other.deleted_key_),
        other.deleted_key_.get());
  }

  map<Key, Value>& operator=(const map<Key, Value>& other) {
    if (&other == this)
      return *this;

#ifdef KOKOPUFFS_DEBUG
    if (!has_set_empty_key_)
      throw std::runtime_error("kokopuffs::map.operator=(map&) empty_key_ not set");
#endif
    delete_table(table_, bucket_count_,
                 *empty_key_,
                 static_cast<bool>(deleted_key_),
                 deleted_key_.get());

    bucket_count_ = other.bucket_count_;
    item_count_ = 0;
    max_load_factor_ = other.max_load_factor_;
    min_load_factor_ = other.min_load_factor_;
    table_ = create_table(bucket_count_);
#ifdef KOKOPUFFS_DEBUG
    has_set_empty_key_ = other.has_set_empty_key_;
    has_set_deleted_key_ = other.has_set_deleted_key_;
#endif

    if (other.empty_key_) {
      empty_key_ = std::unique_ptr<Key>(new Key(*other.empty_key_));
    }
    if (other.deleted_key_) {
      deleted_key_ = std::unique_ptr<Key>(new Key(*other.deleted_key_));
    }
    _copy_elements_from_table(
        other.table_, other.bucket_count_,
        *other.empty_key_,
        static_cast<bool>(other.deleted_key_),
        other.deleted_key_.get());

    return *this;
  }

  map(map<Key, Value>&& other)
      : bucket_count_(other.bucket_count_),
        item_count_(other.item_count_),
        max_load_factor_(other.max_load_factor_),
        min_load_factor_(other.min_load_factor_),
        table_(other.table_)
#ifdef KOKOPUFFS_DEBUG
        , has_set_empty_key_(other.has_set_empty_key_)
        , has_set_deleted_key_(other.has_set_deleted_key_)
#endif
  {
    other.table_ = nullptr;
    // this cheat will basically skip over key and value destructor, and we are
    // luck that free() works with null pointers.
    other.bucket_count_ = 0;

    other.empty_key_.swap(empty_key_);
    other.deleted_key_.swap(deleted_key_);
  }

  map<Key, Value>& operator=(map<Key, Value>&& other) {
    if (&other == this)
      return *this;

#ifdef KOKOPUFFS_DEBUG
    if (!has_set_empty_key_)
      throw std::runtime_error("kokopuffs::map.operator=(map&&) empty_key_ not set");
#endif
    delete_table(table_, bucket_count_,
                 *empty_key_,
                 static_cast<bool>(deleted_key_),
                 deleted_key_.get());

    bucket_count_ = other.bucket_count_;
    item_count_ = other.item_count_;
    max_load_factor_ = other.max_load_factor_;
    min_load_factor_ = other.min_load_factor_;
    table_ = other.table_;
#ifdef KOKOPUFFS_DEBUG
    has_set_empty_key_ = other.has_set_empty_key_;
    has_set_deleted_key_ = other.has_set_deleted_key_;
#endif

    other.table_ = nullptr;
    // see move constructor above
    other.bucket_count_ = 0;

    other.empty_key_.swap(empty_key_);
    other.deleted_key_.swap(deleted_key_);

    return *this;
  }

  ~map() {
#ifdef KOKOPUFFS_DEBUG
    if (!has_set_empty_key_)
      throw std::runtime_error("kokopuffs::map.~map() empty_key_ not set");
#endif
    delete_table(table_, bucket_count_,
                 *empty_key_,
                 static_cast<bool>(deleted_key_),
                 deleted_key_.get());
  }

  void set_empty_key(const Key& key) {
    empty_key_ = std::unique_ptr<Key>(new Key(key));
#ifdef KOKOPUFFS_DEBUG
    has_set_empty_key_ = true;
#endif

    for (size_t i = 0; i < bucket_count_; ++i) {
      Entry& entry = table_[i];
      new (&entry.key) Key(*empty_key_);
    }
  }


  void set_deleted_key(const Key& key) {
    deleted_key_ = std::unique_ptr<Key>(new Key(key));
#ifdef KOKOPUFFS_DEBUG
    has_set_deleted_key_ = true;
#endif
  }

  Value& operator[](const Key& key) {
#ifdef KOKOPUFFS_DEBUG
    if (!has_set_empty_key_)
      throw std::runtime_error("kokopuffs::map.operator[] empty_key_ not set");
#endif

    const uint32_t hash = get_hash(key);
    return _find_or_insert(key, hash);
  }

  size_t erase(const Key& key) {
#ifdef KOKOPUFFS_DEBUG
    if (!has_set_deleted_key_)
      throw std::runtime_error("kokopuffs::map.erase() deleted_key_ not set");
#endif

    const uint32_t hash = get_hash(key);
    size_t index = (size_t)-1;
    if (!_find_bucket(key, hash, index)) {
      return 0;
    }

    Entry& entry = table_[index];
    entry.key = *deleted_key_;
    entry.value.~Value();

    --item_count_;

    return 1;
  }

  void _debug() {
#ifdef KOKOPUFFS_DEBUG
    if (!has_set_empty_key_)
      throw std::runtime_error("kokopuffs::map._debug() empty_key_ not set");
#endif

    using namespace std;
    stringstream ss;
    /* ss << hex; */
    ss << "sizeof(Entry): " << sizeof(Entry) << "\n";
    ss << "sizeof(Key): " << sizeof(Key) << "\n";
    ss << "sizeof(Value): " << sizeof(Value) << "\n";
    ss << "item_count_: " << item_count_ << "\n";
    ss << "bucket_count_: " << bucket_count_ << "\n";
    ss << "load: " << load_factor() << "\n";
    for (size_t i = 0; i < bucket_count_; ++i) {
      Entry& entry = table_[i];
      ss << "bucket " << i << ": ";
      if (entry.key == *empty_key_) {
          ss << "empty ";
      } else if (deleted_key_ && entry.key == *deleted_key_) {
          ss << "deleted ";
      } else {
        ss << "key: " << entry.key << " ";
        ss << "value: " << entry.value << " ";
#ifdef KOKOPUFFS_MAP_COLLISION_DEBUG
        ss << "intended_bucket: " << entry.intended_bucket << " ";
#endif
      }
      ss << "\n";
    }
    ss << "----------------------------------------\n";
    cout << ss.str();
  }

  uint32_t fnv1a(const uint8_t* data, size_t n) {
    uint32_t hash = 2166136261; // offset_basis
    for (size_t i = 0; i < n; ++i) {
      hash ^= data[i];
      hash *= 16777619; // FNV_prime
    }
    return hash;
  }

  uint32_t get_hash(const Key& key) {
    return fnv1a(reinterpret_cast<const uint8_t*>(&key[0]), key.size());
  }

  size_t size() const noexcept {
    return item_count_;
  }

  float load_factor() const noexcept {
    return this->size() / static_cast<float>(bucket_count_);
  }

  void max_load_factor(float z) {
    max_load_factor_ = std::max(0.001f, std::min(z, 1.0f));
    _maybe_resize();
  }

  void min_load_factor(float z) {
    min_load_factor_ = std::max(0.0f, std::min(z, max_load_factor_));
    _maybe_resize();
  }

 private:
  static Entry* create_table(size_t bucket_count) {
    size_t n = bucket_count * sizeof(Entry);
    Entry* table  = (Entry*)malloc(n);
    ::memset(table, 0, n);
    return table;
  }

  static void delete_table(Entry* table,
                           const size_t bucket_count,
                           const Key& empty_key,
                           const bool has_delete,
                           const Key* deleted_key)
  {
    for (size_t i = 0; i < bucket_count; ++i) {
      Entry& entry = table[i];
      if (entry.key != empty_key) {
        if (!has_delete || (has_delete && entry.key != *deleted_key)) {
          entry.value.~Value();
        }
      }
      entry.key.~Key();
    }
    ::free(table);
  }

  Value& _find_or_insert(const Key& key, uint32_t hash) {
refind_slot:
    size_t index = (size_t)-1;
    if (_find_bucket(key, hash, index)) {
      return table_[index].value;
    }

    if (_maybe_resize()) {
      goto refind_slot;
    }

    Entry& entry = table_[index];
    _emplace_entry(entry, key, hash);
    return entry.value;
  }

  template <typename... Args>
  void _emplace_entry(Entry& entry,
                      const Key& key, uint32_t hash, Args&&... args) {
#ifdef KOKOPUFFS_DEBUG
    if (!has_set_empty_key_)
      throw std::runtime_error("kokopuffs::map.deleted_key_ not set");
#endif

    if (entry.key != *empty_key_) {
      if (!deleted_key_) {
        return;
      } else {
        if (entry.key != *deleted_key_) {
          return;
        }
      }
    }

    item_count_++;
    entry.key.~Key();
    new (&entry.key) Key(key);
    new (&entry.value) Value(args...);
#ifdef KOKOPUFFS_MAP_COLLISION_DEBUG
    entry.intended_bucket = hash & (bucket_count_ - 1);
#endif
  }

  bool _maybe_resize() {
    size_t new_bucket_count;
    const float load = this->load_factor();
    if (load > max_load_factor_) {
      new_bucket_count = bucket_count_ * 2;
    } else if (bucket_count_ > KOKOPUFFS_MAP_INTIAL_SIZE &&
               load < min_load_factor_) {
      new_bucket_count = bucket_count_ / 2;
    } else {
      return false;
    }

    Entry* old_table = table_;
    const size_t old_bucket_count = bucket_count_;

    bucket_count_ = new_bucket_count;
    item_count_ = 0;
    table_ = create_table(bucket_count_);
    set_empty_key(*empty_key_);

    _copy_elements_from_table(
        old_table, old_bucket_count,
        *empty_key_,
        static_cast<bool>(deleted_key_),
        deleted_key_.get());
    delete_table(old_table, old_bucket_count,
                 *empty_key_,
                 static_cast<bool>(deleted_key_),
                 deleted_key_.get());

    return true;
  }

  void _copy_elements_from_table(Entry* old_table,
                                 const size_t old_bucket_count,
                                 const Key& old_empty_key,
                                 const bool old_has_delete,
                                 const Key* old_deleted_key) {
    for (size_t i = 0; i < old_bucket_count; ++i) {
      const Entry& old_entry = old_table[i];
      if (old_entry.key == old_empty_key)
        continue;
      if (old_has_delete && old_entry.key == *old_deleted_key)
        continue;

      const uint32_t hash = get_hash(old_entry.key);
      size_t new_index = (size_t)-1;
      // assume that this will always be successful since we are resizing and
      // this it will always fit
      _find_bucket(old_entry.key, hash, new_index);
      Entry& new_entry = table_[new_index];
      _emplace_entry(new_entry, old_entry.key, hash, old_entry.value);
    }
  }

  bool _find_bucket(const Key& key, const uint32_t hash, size_t& found_index) {
    const size_t mask = bucket_count_ - 1;
    const size_t start_index = hash & mask;
    size_t probe_count = 0;
    size_t deleted_index = (size_t)-1;
    bool found_deleted_index = false;

    while (probe_count <= bucket_count_) {
      size_t triangle_number = (probe_count * (probe_count + 1)) / 2;
      size_t index = (start_index + triangle_number) & mask;
      Entry& entry = table_[index];

      if (entry.key == *empty_key_) {
        if (found_deleted_index)
          found_index = deleted_index;
        else
          found_index = index;
        return false;
      }

      if (!found_deleted_index && deleted_key_ && entry.key == *deleted_key_) {
        found_deleted_index = true;
        deleted_index = index;
      }

      if (entry.key == key) {
        found_index = index;
        return true;
      }

      ++index;
      if (index == bucket_count_)
        index = 0;
      ++probe_count;
    }

    if (found_deleted_index) {
      found_index = deleted_index;
    } else {
      throw std::runtime_error("rko_map hash table completely full");
    }
    return false;
  }

  size_t bucket_count_;
  size_t item_count_;
  float max_load_factor_;
  float min_load_factor_;
  std::unique_ptr<Key> empty_key_;
  std::unique_ptr<Key> deleted_key_;
  Entry* table_;
#ifdef KOKOPUFFS_DEBUG
  bool has_set_empty_key_;
  bool has_set_deleted_key_;
#endif
};

#ifdef KOKOPUFFS_DEBUG
#undef KOKOPUFFS_DEBUG
#endif

}
