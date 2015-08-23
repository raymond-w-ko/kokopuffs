#include "kokopuffs/map.hpp"
#include "kokopuffs/max_heap.hpp"
#include "kokopuffs/min_heap.hpp"
#include "kokopuffs/algorithm.hpp"
#include "Stopwatch.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <random>

using namespace kokopuffs;

void test_map() {
  kokopuffs::map<std::string, int> m;
  m.set_empty_key("");
  m.set_deleted_key("<deleted>");

  m["foo"] = 1;
  m._debug();
  m["bar"] = 2;
  /* m._debug(); */
  m["fizz"] = 3;
  m["buzz"] = 4;
  m._debug();
  m["bazz"] = 5;
  m._debug();

  m["alpha"] = 6;
  m._debug();
  m["beta"] = 7;
  /* m._debug(); */
  m["delta"] = 8;
  m._debug();

  m["delta"] = 9999;
  m._debug();

  // crash if you uncomment this without resizing
  m["epsilon"] = 9;
  m._debug();
  m["chi"] = 10;
  m._debug();
  m["rho"] = 11;
  m._debug();
  m["upsilon"] = 12;
  m._debug();
  m["omega"] = 13;
  m["terran"] = 14;
  m["protoss"] = 15;
  m["zerg"] = 16;
  m._debug();

  m.erase("foo");
  m._debug();
  m.erase("bar");
  m._debug();
  m.erase("fizz");
  m._debug();
  m.erase("buzz");
  m._debug();
  m.erase("bazz");
  m._debug();
  m.erase("alpha");
  m.erase("beta");
  m.erase("delta");
  m.erase("epsilon");
  m._debug();
  m.erase("omega");
  m.erase("terran");
  m.erase("zerg");
  m.erase("protoss");
  m._debug();

  std::cout << m["foo"] << "\n";
  std::cout << m["bar"] << "\n";
  m._debug();

  kokopuffs::map<std::string, int> copy1(m);
  copy1 = m;
  kokopuffs::map<std::string, int> copy2(std::move(m));
  kokopuffs::map<std::string, int> copy3;
  copy3.set_empty_key("");
  copy3 = std::move(copy1);

  kokopuffs::map<std::string, std::string> smap;
  smap.set_empty_key("");
  smap._debug();
  smap["foo"] = "bar";
  smap["fizz"] = "buzz";
  smap._debug();
  smap["fizz"] = "soda";
  smap._debug();
}

void test_sort() {
  Stopwatch watch;
  
  static const int n = 1000000;
  std::minstd_rand re(42);  
  std::uniform_int_distribution<int> uniform_dist;
  
  watch.Start();
  std::vector<int> orignums;
  orignums.reserve(n);
  for (int i = 0; i < n; ++i) {
    orignums.push_back(uniform_dist(re));
  }
  std::cout << "created " << n << " random nums in " << watch.StopResultMilliseconds() << " ms\n";
  
  std::vector<int> std_sorted_nums(orignums);
  
  watch.Start();
  std::sort(std_sorted_nums.begin(), std_sorted_nums.end());
  std::cout << "sorted via std::sort in " << watch.StopResultMilliseconds() << " ms\n";
  
  std::vector<int> test{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  max_heap<int> heap0(test);
  for (int num : heap0.array_) {
    std::cout << num << " ";
  }
  std::cout << "\n";
  
  while (!heap0.empty()) {
    std::cout << heap0.extract() << " ";
  }
  std::cout << "\n";
  
  std::vector<int> min_heap_sorted_nums(orignums);
  watch.Start();
  kokopuffs::heapsort(min_heap_sorted_nums);
  std::cout << "heapsorted in " << watch.StopResultMilliseconds() << " ms\n";
  
  if (min_heap_sorted_nums != std_sorted_nums) {
    throw std::runtime_error("heapsorted sorted numbers mismatch");
  }
  
  std::vector<int> quicksorted_nums(orignums);
  watch.Start();
  kokopuffs::quicksort(quicksorted_nums);
  if (quicksorted_nums != std_sorted_nums) {
    throw std::runtime_error("quicksort sorted numbers mismatch");
  }
  std::cout << "sorted via quicksort in " << watch.StopResultMilliseconds() << " ms\n";
  
  std::vector<int> mergesorted_nums(orignums);
  watch.Start();
  kokopuffs::mergesort(mergesorted_nums);
  if (mergesorted_nums != std_sorted_nums)
    throw std::runtime_error("mergesort sorted numbers mismatch");
  std::cout << "sorted via mergesort in " << watch.StopResultMilliseconds() << " ms\n";
}

int main() {
  /* test_map(); */
  test_sort();
  return 0;
}
