#include "kokopuffs/map.hpp"

#include <string>
#include <iostream>

int main() {
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
  copy1.set_empty_key("");
  copy1 = m;
  kokopuffs::map<std::string, int> copy2(std::move(m));
  kokopuffs::map<std::string, int> copy3;
  copy3.set_empty_key("");
  copy3 = std::move(copy1);

  kokopuffs::map<std::string, std::string> string_map;
  string_map.set_empty_key("");
  string_map._debug();

  return 0;
}
