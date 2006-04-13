/*
  parambinds-formals.C

  Testing the AliasIRInterface implementations for C/C++ compilers.
  Specifically looking at possible actual parameters that could
  be involved in pointer parameter bindings and formal parameters.
*/

class Base {
  public:
    Base *next;
};

class SubClass : public Base {
};

class SubSubClass : public SubClass {
};

// should also have a multiple inheritance example

// procedure prototypes
void foo_ref1(int& x, Base& b, SubClass& s, SubSubClass& sc);
{
}

Base& foo_ref2(double& z, SubSubClass& sc)
{
  return sc;
}

void foo_ptr1(int* x, Base* b, SubClass* s, SubSubClass* sc)
{
}

SubClass* foo_ptr2(char [] s, SubSubClass *sc)
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

char * bar_arrays( char [][] c)
{
  return c[0];
}

int * bar_arrays2( int[5] a )
{
  return a[3];
}

void main()
{
    int x, y, z;
    Base b;
    SubClass s;
    SubSubClass sc;
    Base* bp = new Base;
    SubClass* sp = new SubClass;
    SubSubClass* scp = new SubSubClass;

    foo_ref1
}
