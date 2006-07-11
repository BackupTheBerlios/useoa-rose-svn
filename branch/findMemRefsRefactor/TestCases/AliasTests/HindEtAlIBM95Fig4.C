/*
 * from Flow-Sensitive Interprocedural Type Analysis for C++
 * Paul Carini, Michael Hind, and Harini Srinivasan
 * IBM Research Report #20267, November 1995. 
 * Figure 4.
 */

#include <stdio.h>

class B {
 public:
  int data;
  virtual B *createObj();
  virtual void changeData(B &q);
};

B *B::createObj() {
  return (new B);
}

void B::changeData(B &q) {
  q.data = 40;
}

class C : public B {
 public:
  int Cdata;
  void changeData(B &q);
};

void C::changeData(B &q) {
  q.data = 50;
}

class D : public B {
 public:
  B *createObj();
  void changeData(B &q);
};

B *D::createObj() {
  return (new D);
}

void D::changeData(B &q) {
  q.data = 20;
}

class E : public D {
 public:
  B *createObj();
};

B* E::createObj() {
  return (new E);
}

int main()
{
  B q, *r, *b, **p;
  C c;
  int cond = (r != b);
  p = &r;
  if ( cond > 23 ) {
    b = new D;
    r = b->createObj();
  } else {
    b = new E;
    r = b->createObj();
  }
  (*p)->changeData(q);
  printf("%d", q.data);
  return 0;
}
