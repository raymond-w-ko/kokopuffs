#include <stdint.h>
#include <memory>
#include <exception>
#include <cassert>

#include <iostream>
#include <iomanip>

namespace kokopuffs {

template<typename Key, typename Value>
class map {
  enum {
    kInitialSize = 16,
  };
  struct Entry {
    bool value_initialized;
    bool busy;
    uint32_t hash;
    Key key;
    Value value;
    /* size_t intended_bucket; */
  };
  
 private:
  static void delete_table(Entry* table, size_t bucket_count) {
    for (size_t i = 0; i < bucket_count; ++i) {
      Entry& entry = table[i];
      entry.key.~Key();
      if (entry.value_initialized) {
        entry.value.~Value();
      }
    }
    free(table);
  }
  
  static Entry* create_table(size_t bucket_count) {
    Entry* table  = (Entry*)malloc(bucket_count * sizeof(Entry));
    for (size_t i = 0; i < bucket_count; ++i) {
      Entry& entry = table[i];
      entry.value_initialized = false;
      entry.busy = false;
      entry.hash = 0;
      new (&entry.key) Key();
    }
    return table;
  }
  
 public:
  map(size_t initial_table_size = kInitialSize, float load_factor = 0.5)
      : load_factor_(load_factor),
        item_count_(0)
  {
    bucket_count_ = initial_table_size;
    table_ = create_table(bucket_count_);
  }
  
  ~map() {
    delete_table(table_, bucket_count_);
  }
  
  Value& operator[](const Key& key) {
    uint32_t hash = get_hash(key);
    size_t index = (size_t)-1;
    
refind_slot:
    if (_find(key, hash, index)) {
      return table_[index].value;
    }
    
    if (maybe_resize()) {
      goto refind_slot;
    }
    
    Entry& entry = table_[index];
    if (!entry.value_initialized) {
      item_count_++;
      entry.key.~Key();
      entry.hash = hash;
      new (&entry.key) Key(key);
      new (&entry.value) Value();
      entry.value_initialized = true;
      entry.busy = true;
      /* entry.intended_bucket = hash % bucket_count_; */
    }
    return entry.value;
  }
  
  size_t erase(const Key& key) {
    uint32_t hash = get_hash(key);
    size_t index = (size_t)-1;
    if (!_find(key, hash, index)) {
      return 0;
    }
    
    Entry& entry = table_[index];
    entry.hash = 0;
    entry.value_initialized = false;
    entry.key = invalid_key_;
    entry.value.~Value();
    
    --item_count_;
    
    return 1;
  }
  
  void _debug() {
    using namespace std;
    cout << "sizeof(Entry): " << sizeof(Entry) << "\n";
    cout << "sizeof(bool): " << sizeof(bool) << "\n";
    cout << "sizeof(uint32_t): " << sizeof(uint32_t) << "\n";
    cout << "sizeof(Key): " << sizeof(Key) << "\n";
    cout << "sizeof(Value): " << sizeof(Value) << "\n";
    cout << "item_count_: " << item_count_ << "\n";
    cout << "bucket_count_: " << bucket_count_ << "\n";
    cout << "load: " << (float)item_count_ / bucket_count_ << "\n";
    cout << "invalid_key_: *" << invalid_key_ << "*\n";
    for (size_t i = 0; i < bucket_count_; ++i) {
      Entry& entry = table_[i];
      cout << "bucket " << i << ": " << " busy " << entry.busy;
      if (entry.key != invalid_key_) {
        cout << " key: " << entry.key << " "
            << "hash: " << entry.hash << " "
            /* << "intended_bucket: " << entry.intended_bucket << " " */
            ;
        if (entry.value_initialized) {
          cout << "value: " << entry.value << "";
        }
      }
      cout << "\n";
    }
    cout << "----------------------------------------\n";
  }
  
  uint32_t get_hash(const Key& key) {
    /* return 1; */
    uint32_t hash = 5381;
    for (size_t i = 0; i < key.size(); ++i) {
      hash = hash * 31 + key[i];
    }
    return hash;
  }
  
 private:
  bool maybe_resize() {
    if (((float)item_count_ / bucket_count_) <= load_factor_)
      return false;
    
    Entry* oldtable = table_;
    size_t oldcount = bucket_count_;

    bucket_count_ *= 2;
    item_count_ = 0;
    table_ = create_table(bucket_count_);

    for (size_t i = 0; i < oldcount; ++i) {
      Entry& oldentry = oldtable[i];
      if (oldentry.key != invalid_key_) {
        assert(oldentry.value_initialized);
        (*this)[oldentry.key] = oldentry.value;
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
      
      if (!entry.value_initialized && !found_empty_index) {
        found_empty_index = true;
        emtpy_index = index;
        /* std::cout << "empty " << index << "\n"; */
      }
      
      if (entry.hash == hash && entry.key == key) {
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
  float load_factor_;
  size_t item_count_;
  Key invalid_key_;
  Entry* table_;
};

}
