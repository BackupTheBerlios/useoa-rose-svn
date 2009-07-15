#include <unistd.h>

class foo {
public:
  virtual void virt() { }
  void *operator new(size_t size, char *buf, int n) { return buf; }
  void operator delete(void *p) { }
};

int main()
{
  char *buf  = new char[1000];   // pre-allocated buffer
  foo *f = new (buf, 2) foo;  // placement new
  ((foo *)buf)->virt();
  f->virt();
  return 0;
}
