#pragma once

#include <vector>

namespace kokopuffs {

template <typename T>
class max_heap {
 public:
  std::vector<T> array_;
  
  max_heap(const std::vector<T>& intial_values)
      : array_(intial_values) {
    size_t n = array_.size();
    for (int i = (n - 1) / 2; i >= 0; --i) {
      max_heapify(i);
    }
  }
  
  void insert(const T& value) {
    array_.push_back(value);
    
    size_t i = array_.size() - 1;
    size_t parent = (i - 1) / 2;
    
    while (parent > 0 && array_[i] > array_[parent]) {
      std::swap(array_[i], array_[parent]);
      i = parent;
      parent = (i - 1) / 2;
    }
  }
  
  T extract() {
    T biggest = array_[0];
    T last = array_.back();
    array_.pop_back();
    array_[0] = last;
    max_heapify(0);
    return biggest;
  }
  
  bool empty() const {
    return array_.empty();
  }
  
 private:
  void max_heapify(size_t i) {
    const size_t parent = i;
    const size_t left = 2 * i + 1;
    const size_t right = 2 * i + 2;
    
    size_t largest = parent;
    
    if (left < array_.size() && array_[left] > array_[largest]) {
      largest = left;
    }
    if (right < array_.size() && array_[right] > array_[largest]) {
      largest = right;
    }
    
    if (largest != parent) {
      std::swap(array_[largest], array_[parent]);
      max_heapify(largest);
    }
  }
};

}
