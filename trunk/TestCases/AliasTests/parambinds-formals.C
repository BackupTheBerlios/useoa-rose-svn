/*
  parambinds-formals.C

  Testing the AliasIRInterface implementations for C/C++ compilers.
  Specifically looking at possible actual parameters that could
  be involved in pointer parameter bindings and formal parameters.

  Notes:
  http://www.newty.de/fpt/fpt.html, notes on function pointers
*/

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
using namespace std;

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

class B;
class C;

class A {
  public:
    B *bptr;
};

class B {
  public:
    C *cptr;
};

class C {
  public:
    Base * baseptr[7];
};

// should also have a multiple inheritance example
// can't this to compile yet when I pass a ptr to SubClassMulti into Base*
/*
class SubClassSide : public Base {
  public:
    SubClassSide(Base & par) : myParent(par) {}
    ~SubClassSide() {}
    Base & myParent;
};

class SubClassMulti : public SubClass, public virtual SubClassSide {
  public:
    SubClassMulti(Base &par) : SubClass(par) {}
    ~SubClassMulti() {}
};
*/

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

// not legal to pass in local objects to var arg calls
// must be a POD type, plain old datatype which includes
// the base types and ptrs and references
void ellipsis_baserefs(int x, ...)
{
}


// if we pass in an int& can that be picked up as an int? yes
// but we don't think that the second parameter to va_arg can
// be a reference
void ellipsis_int(int i, ...)
{
    int j;
    va_list ap;
    va_start(ap,i);

    // g++ only accepts basic types or ptr types for the second param
    j = va_arg(ap,int);
    printf("j=%d\n",j);

    va_end(ap);
}

// C, for function ptr example
int DoIt  (float a, char b, char c){ printf("DoIt\n");   return (int)a+(int)b+(int)c; }
int DoMore(float a, char b, char c){ printf("DoMore\n"); return (int)a-(int)b+(int)c; }

char * bar_arrays( char c[][10])
{
  return c[0];
}

int * bar_arrays2( int a[5] )
{
  return &a[3];
}

void PassPtr(int (*pt2Func)(float, char, char))
{
   int result = (*pt2Func)(12, 'a', 'b');     // call using function pointer
   cout << result << endl;
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
    //SubClassMulti* smp = new SubClassMulti(*bp);
    A aclass;

    // should have 4 param bindings involving references modeled as ptrs
    foo_ref1(x,b,s,sc);
    foo_ref1(sc.myInt,b,s,sc);
    foo_ref1(x,sc,sc,sc);
    foo_ref1(((SubSubClass*)(scp->next))->myInt,s.myParent,s,sc);

    Base& bref = foo_ref2(f, sc);

    foo_ptr1(&x,&b,&s,&sc);
    foo_ptr1(&sc.myInt,&b,&s,&sc);
    foo_ptr1(&x,&sc,&sc,&sc);
    //foo_ptr1(&x,smp,&sc,&sc);
    foo_ptr1(&x, aclass.bptr->cptr->baseptr[3], sp, scp);

    char hi[]= "hello";
    Base* bptr = foo_ptr2(hi, &sc);

    // variable numbers of parameters
    ellipsis_intptrs(3, &x, &y, &z);
    ellipsis_intptrs(5, &x, &y, &z, &z, &(scp->myInt));

    ellipsis_baseptrs(3, bp, sp, scp);
    b.next = &sc;
    ellipsis_baseptrs(4, &b, &s, b.next, &sc);

    //ellipsis_baserefs(2, b, s, sc);
    ellipsis_baserefs(1, *(b.next));

    x = 7;
    int &q = x;
    ellipsis_int(1,q); 

    // passing in arrays
    char bye[][10] = {"hi", "bye", "adios"};
    char *cptr = bar_arrays(bye);
    int a[5] = {1,2,3,4,5};
    int *iptr = bar_arrays2(a);

    // actuals with a number of dereferences, field accesses and array accesses

    // function ptrs
    // got examples from http://www.newty.de/fpt/fpt.html
    int (*pt2Function)(float, char, char) = NULL;                        // C

    PassPtr(&DoIt);

    pt2Function = DoIt;      // short form
    pt2Function = &DoMore;   // correct assignment using address operator

    PassPtr(pt2Function);

    return 0;
}


