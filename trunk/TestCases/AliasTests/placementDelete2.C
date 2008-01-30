//#include <unistd.h>
#include <iostream>

class foo {
public:
  foo() { }
  virtual ~foo() { }
  virtual void virt() { }
  void *operator new(size_t size, char *buf, int n) { return buf; }
  void operator delete(void *p) { std::cout << "my delete" << std::endl; }
};

int main()
{
  char *buf  = new char[1000];   // pre-allocated buffer
  foo *f = new (buf, 2) foo;  // placement new
  delete f;
  return 0;
}
