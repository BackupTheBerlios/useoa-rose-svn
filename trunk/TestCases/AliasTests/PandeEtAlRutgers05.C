/* from Static Type Determination and Aliasing for C++,  
 * Hemant Pande and Barbara G. Ryder, 
 * Laboratory of Computer Science Research Technical Report,  
 * Number LCSR-TR-250-A, October, 1995.
 */

#include <stdio.h>

class Base {
 public:
  virtual void foo();
  virtual void bar();
  virtual void baz();
} *a, *b, *p, *q;

void Base::foo() {
  a = new Base;
}

void Base::bar() {
  
}

void Base::baz() {

}

class Derived : public Base {
 public:
  void foo();
  void bar();
} r, *s;

void Derived::foo() {
  a = new Derived;
  printf("hello world!\n");
  b = new Derived;
}

void Derived::bar() {
  
}

int main(int argc, char **argv)
{
  if (argc < 2 ) {
    p = new Base;
    q = new Derived;
    b = new Base;
  } else {
    p = new Derived;
    q = new Base;
  }
  s = &r;
  if (argv != NULL)
    s->Derived::bar();
  p->foo();
  q->foo();
  q = new Base;
  q->foo();
  a->bar();
  p->baz();
  return 0;
}
