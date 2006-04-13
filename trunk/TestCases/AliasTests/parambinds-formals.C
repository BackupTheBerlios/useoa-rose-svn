/*
  parambinds-formals.C

  Testing the AliasIRInterface implementations for C/C++ compilers.
  Specifically looking at possible actual parameters that could
  be involved in pointer parameter bindings and formal parameters.
*/

#include <stdarg.h>

class Base {
  public:
    Base *next;
};

class SubClass : public Base {
  public:
    SubClass(Base & par) : myParent(par) {}
    ~SubClass() {}
    Base & myParent;
};

class SubSubClass : public SubClass {
  public:
    SubSubClass(Base &par, int & aint) : SubClass(par), myInt(aint) {}
    ~SubSubClass() {}
    int & myInt;
};

// should also have a multiple inheritance example

// procedure definitions
void foo_ref1(int& x, Base& b, SubClass& s, SubSubClass& sc)
{
}

Base& foo_ref2(double& f, SubSubClass& sc)
{
  return sc;
}

void foo_ptr1(int* x, Base* b, SubClass* s, SubSubClass* sc)
{
}

SubClass* foo_ptr2(char s[], SubSubClass *sc)
{ 
  return sc;
}

// going to pass in ptrs to ints
void ellipsis_intptrs(int x, ...)
{
}

void ellipsis_baseptrs(int x, ...)
{
}

void ellipsis_baserefs(int x, ...)
{
}

char * bar_arrays( char c[][10])
{
  return c[0];
}

int * bar_arrays2( int a[5] )
{
  return &a[3];
}

int main()
{
    int x, y, z;
    double f;
    Base b;
    SubClass s(b);
    SubSubClass sc(s,y);
    Base* bp = new Base;
    SubClass* sp = new SubClass(sc);
    SubSubClass* scp = new SubSubClass(*bp,y);

    // should have 4 param bindings involving references modeled as ptrs
    foo_ref1(x,b,s,sc);
    foo_ref1(sc.myInt,b,s,sc);
    foo_ref1(x,sc,sc,sc);

    Base& bref = foo_ref2(f, sc);

    foo_ptr1(&x,&b,&s,&sc);
    foo_ptr1(&sc.myInt,&b,&s,&sc);
    foo_ptr1(&x,&sc,&sc,&sc);

    char hi[]= "hello";
    Base* bptr = foo_ptr2(hi, &sc);

    // variable numbers of parameters
    ellipsis_intptrs(3, &x, &y, &z);
    ellipsis_intptrs(5, &x, &y, &z, &z, &(scp->myInt));

    return 0;
}
