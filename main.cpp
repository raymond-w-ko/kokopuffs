#include "kokopuffs/map.hpp"
#include <string>
#include <iostream>

int main() {
  kokopuffs::map<std::string, int> m(8);
  
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
  m["epsilon"] = 8;
  m._debug();
  /* m["chi"] = 9; */
  /* m["rho"] = 10; */
  
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
  m._debug();
  m.erase("beta");
  m._debug();
  m.erase("delta");
  m._debug();
  m.erase("epsilon");
  m._debug();
  
  std::cout << m["foo"] << "\n";
  std::cout << m["xyzzy"] << "\n";
  m._debug();
  
  kokopuffs::map<int, int> m2;
  
  return 0;
}
