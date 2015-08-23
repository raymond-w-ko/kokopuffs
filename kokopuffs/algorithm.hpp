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

////////////////////////////////////////////////////////////////////////////////

template <typename T>
void _merge(std::vector<T>& arr, int64_t begin, int64_t mid, int64_t end,
            std::vector<T>& scratch) {
  int64_t list0 = begin;
  int64_t list1 = mid;
  for (int64_t i = begin; i < end; ++i) {
    if (list0 < mid && (list1 >= end || arr[list0] < arr[list1])) {
      scratch[i] = arr[list0];
      ++list0;
    } else {
      scratch[i] = arr[list1];
      ++list1;
    }
  }
}

template <typename T>
void _copy(std::vector<T>& src, int64_t begin, int64_t end, std::vector<T>& dst) {
  for (size_t i = begin; i < end; ++i) {
    dst[i] = src[i];
  }
}

template <typename T>
void _mergesort(std::vector<T>& arr, int64_t begin, int64_t end,
                std::vector<T>& scratch) {
  // trivially sorted since it is 1
  if (end - begin <= 1)
    return;
  int64_t mid = (begin + end) / 2;
  _mergesort(arr, begin, mid, scratch);
  _mergesort(arr, mid, end, scratch);
  _merge(arr, begin, mid, end, scratch);
  _copy(scratch, begin, end, arr);
}

template <typename T>
void mergesort(std::vector<T>& arr) {
  if (arr.size() == 0)
    return;
  std::vector<T> scratch(arr.size());
  _mergesort(arr, 0, arr.size(), scratch);
}

template <typename T>
void _maxheapify(std::vector<T>& arr, const int64_t n, const int64_t parent) {
  const int64_t left = 2 * parent + 1;
  const int64_t right = 2 * parent + 2;
  int64_t largest = parent;

  if (left < n && arr[left] > arr[largest])
    largest = left;
  if (right < n && arr[right] > arr[largest])
    largest = right;

  if (largest != parent) {
    std::swap(arr[largest], arr[parent]);
    _maxheapify(arr, n, largest);
  }
}

template <typename T>
void heapsort(std::vector<T>& arr) {
  if (arr.size() == 0)
    return;
  // build heap
  for (int64_t i = (arr.size() - 1) / 2; i >= 0; --i) {
    _maxheapify(arr, arr.size(), i);
  }

  for (int64_t i = arr.size() - 1; i >= 0; --i) {
    std::swap(arr[0], arr[i]);
    _maxheapify(arr, i, 0);
  }
}

}
