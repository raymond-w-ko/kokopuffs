#pragma once

namespace kokopuffs {
template <typename T>
int64_t lomuto_partition(std::vector<T>& arr, const int64_t lo, const int64_t hi) {
  const T& pivot_value = arr[hi];  
  int64_t i = lo;
  for (int64_t j = lo; j < hi; ++j) {
    if (arr[j] <= pivot_value) {
      std::swap(arr[i], arr[j]);
      ++i;
    }
  }
  std::swap(arr[hi], arr[i]);
  return i;
}

template <typename T>
int64_t hoare_partition(std::vector<T>& arr, const int64_t lo, const int64_t hi) {
  const T& pivot_value = arr[lo];
  int64_t i = lo - 1;
  int64_t j = hi + 1;
  while (true) {
    do {
      --j;
    } while (arr[j] > pivot_value);
    do {
      ++i;
    } while (arr[i] < pivot_value);
    if (i < j)
      std::swap(arr[i], arr[j]);
    else
      return j;
  }
}

template <typename T>
void _quicksort(std::vector<T>& arr, const int64_t lo, const int64_t hi) {
#if 0
  if (lo < hi) {
    int64_t mid = lomuto_partition(arr, lo, hi);
    _quicksort(arr, lo, mid-1);
    _quicksort(arr, mid+1, hi);
  }
#else
  if (lo < hi) {
    int64_t mid = hoare_partition(arr, lo, hi);
    _quicksort(arr, lo, mid);
    _quicksort(arr, mid+1, hi);
  }
#endif
}

template <typename T>
void quicksort(std::vector<T>& arr) {
  if (arr.size() == 0)
    return;
  _quicksort(arr, 0, arr.size() - 1); 
}

}
