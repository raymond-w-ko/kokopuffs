# kokopuffs
Some (hopefully) fast and efficient C++11 data structures

Mostly written as interview practice, but they might be useful someday.

## Requirements
1. Header Only (no compiled portions)
2. Zero Config (no platform specific config.hpp file needed like Google's sparsehash)
3. Similar to standard C++ STL's interface as much as possible (so you only relearn as few things as possible).

```cpp
#include <kokopuffs/map.hpp>
```

After watching https://www.youtube.com/watch?v=fHNmRkzxHWs , I wanted to write an open-address hash table that I could actually use in my future projects.

According to Chandler Carruth of Google who works on the Clang compiler and libraries, this is what you want to use most of them time. The standard library's ```<map>``` and ```<unorderd_map>``` are very cache hostile due to fact that the former is a linked list that need to be rebalanced, and the latter's table entries are implemented as linked lists.
