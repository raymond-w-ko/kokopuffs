#pragma once

#include <stdint.h>
#include <memory>
#include <exception>
#include <cassert>

#include <iostream>
#include <iomanip>
#include <sstream>

#if defined(_DEBUG) || defined(DEBUG) && !defined(NDEBUG) && !defined(_NDEBUG)
#define KOKODEBUG
#endif

namespace kokopuffs {

template<typename Key, typename Value>
class map {
  enum {
    kInitialSize = 16,
  };
  struct Entry {
    bool initialized;
    bool busy;
    uint32_t hash;
    Key key;
    Value value;
#ifdef KOKODEBUG
    size_t intended_bucket;
#endif
  };
  
 private:
  static Entry* create_table(size_t bucket_count) {
    size_t n = bucket_count * sizeof(Entry);
    Entry* table  = (Entry*)malloc(n);
    /*
    for (size_t i = 0; i < bucket_count; ++i) {
      Entry& entry = table[i];
      entry.initialized = false;
      entry.busy = false;
      entry.hash = 0;
#ifdef KOKODEBUG
      entry.intended_bucket = 0;
#endif
    }
    */
    // since all fields are mostly equivalent to 0, we can cheat and use
    // memset, however, in case we need to use structures that aren't
    // equivalent to zero, then we have to delete below and initialize in a for
    // loop above.
    ::memset(table, 0, n);
    return table;
  }
  
  static void delete_table(Entry* table, size_t bucket_count) {
    for (size_t i = 0; i < bucket_count; ++i) {
      Entry& entry = table[i];
      if (entry.initialized) {
        entry.key.~Key();
        entry.value.~Value();
      }
    }
    free(table);
  }
  
 public:
  map(size_t initial_table_size = kInitialSize)
      : bucket_count_(kInitialSize),
        item_count_(0),
        // default factor used in Google's densehashtable.h
        max_load_factor_(0.5f)
  {
    table_ = create_table(bucket_count_);
  }
  
  ~map() {
    delete_table(table_, bucket_count_);
  }
  
  Value& operator[](const Key& key) {
    uint32_t hash = get_hash(key);
    return _find_or_insert(key, hash);
  }
  
  size_t erase(const Key& key) {
    uint32_t hash = get_hash(key);
    size_t index = (size_t)-1;
    if (!_find(key, hash, index)) {
      return 0;
    }
    
    Entry& entry = table_[index];
    entry.hash = 0;
    entry.initialized = false;
    entry.key.~Key();
    entry.value.~Value();
    
    --item_count_;
    
    return 1;
  }
  
  void _debug() {
    using namespace std;
    stringstream ss;
    /* ss << hex; */
    ss << "sizeof(Entry): " << sizeof(Entry) << "\n";
    ss << "sizeof(bool): " << sizeof(bool) << "\n";
    ss << "sizeof(uint32_t): " << sizeof(uint32_t) << "\n";
    ss << "sizeof(Key): " << sizeof(Key) << "\n";
    ss << "sizeof(Value): " << sizeof(Value) << "\n";
    ss << "item_count_: " << item_count_ << "\n";
    ss << "bucket_count_: " << bucket_count_ << "\n";
    ss << "load: " << load_factor() << "\n";
    for (size_t i = 0; i < bucket_count_; ++i) {
      Entry& entry = table_[i];
      ss << "bucket " << i << ": " << " busy " << entry.busy << " "
           << "hash " << std::hex << entry.hash << " "
#ifdef KOKODEBUG
           << "intended_bucket: " << entry.intended_bucket << " "
#endif
            ;
      if (entry.initialized) {
        ss << "key: " << entry.key << " ";
        ss << "value: " << entry.value << "";
      }
      ss << "\n";
    }
    ss << "----------------------------------------\n";
    cout << ss.str();
  }
  
  uint32_t get_hash(const Key& key) {
    /* return 1; */
    uint32_t hash = 5381;
    for (size_t i = 0; i < key.size(); ++i) {
      hash = hash * 31 ^ key[i];
    }
    return hash;
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

 private:
  Value& _find_or_insert(const Key& key, uint32_t hash) {
refind_slot:
    size_t index = (size_t)-1;
    if (_find(key, hash, index)) {
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
    if (entry.initialized)
      return;
    
    item_count_++;
    entry.hash = hash;
    new (&entry.key) Key(key);
    new (&entry.value) Value(args...);
    entry.initialized = true;
    entry.busy = true;
#ifdef KOKODEBUG
    entry.intended_bucket = hash % bucket_count_;
#endif
  }
  
  bool _maybe_resize(bool force = false) {
    if (!force && this->load_factor() <= max_load_factor_)
      return false;
    
    Entry* oldtable = table_;
    const size_t oldcount = bucket_count_;

    bucket_count_ *= 2;
    item_count_ = 0;
    table_ = create_table(bucket_count_);

    for (size_t i = 0; i < oldcount; ++i) {
      const Entry& oldentry = oldtable[i];
      if (oldentry.initialized) {
#ifdef KOKODEBUG
        assert(oldentry.initialized);
#endif
        /* (*this)[oldentry.key] = oldentry.value; */
        size_t newindex = (size_t)-1;
        // assume that this will always be successful since we are resizing and
        // this it will always fit
        _find(oldentry.key, oldentry.hash, newindex);
        Entry& newentry = table_[newindex];
        _emplace_entry(newentry, oldentry.key, oldentry.hash, oldentry.value);
      }
    }

    delete_table(oldtable, oldcount);
    
    return true;
  }
  
  bool _find(const Key& key, uint32_t hash, size_t& found_index) {
    size_t index = hash % bucket_count_;
    /* std::cout << "_find " << key << " " << hash << " " << index << "\n"; */
    size_t probe_count = 0;
    size_t emtpy_index = (size_t)-1;
    bool found_empty_index = false;
    
    while (probe_count <= bucket_count_) {
      Entry& entry = table_[index];
      
      if (!entry.busy) {
        if (found_empty_index)
          found_index = emtpy_index;
        else
          found_index = index;
        /* std::cout << "found_index " << found_index << "\n"; */
        return false;
      }
      
      if (!entry.initialized && !found_empty_index) {
        found_empty_index = true;
        emtpy_index = index;
        /* std::cout << "empty " << index << "\n"; */
      }
      
      if (entry.initialized && entry.hash == hash && entry.key == key) {
        found_index = index;
        return true;
      }
      
      ++index;
      if (index == bucket_count_)
        index = 0;
      ++probe_count;
    }
    
    throw std::runtime_error(
        "rko_map hash table completely full, this should not have happened.");
    return false;
  }
  
  size_t bucket_count_;
  size_t item_count_;
  float max_load_factor_;
  Entry* table_;
};

#ifdef KOKODEBUG
#undef KOKODEBUG
#endif

}
