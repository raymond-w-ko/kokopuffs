# kokopuffs
Some (hopefully) fast and efficient C++11 data structures

Mostly written as interview practice, but they might be useful someday.

```cpp
#include <kokopuffs/map.hpp>
```

After watching https://www.youtube.com/watch?v=fHNmRkzxHWs , I wanted to write an open-address hash table that I could potentially actually use in my future projects.

According to Chandler Carruth of Google who works on the Clang compiler and libraries, this is what you want to use most of them time. The standard library's ```<map>``` and ```<unorderd_map>``` are very cache hostile due to fact that the former is a linked list that need to be rebalanced, and the latter'stable entries are implemented as linked lists.
