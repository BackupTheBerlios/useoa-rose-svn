#include <new>

class foo { };

int main()
{
  char *buf  = new char[1000];   // pre-allocated buffer
  // The following invokes 
  // inline void *operator new(size_t ,void *__p) throw(){}
  // which is defined in new (at least with gcc 4.1.2).
  // It is not a builtin new.
  foo *p = new (buf) foo; // placement new
  *p;
  *buf;
  return 0;
}
