/*
  refClass2.C

  Goal: Test the use of a reference in forming an alias
        when used in a constructor initializer.
        Also want to verify that the correct MREs are generated when a constructor is called.
*/

int x;
int y;
int *xPtr = &x;
int *yPtr = &y;

class foo {
 public:
  foo(int *x) : someRef(x), someOtherRef(someRef) { *x; *someRef; *(this->someOtherRef); }
  foo() : someRef(xPtr), someOtherRef(yPtr) { }
  foo(foo &f) : someRef(xPtr), someOtherRef(yPtr) { }
  foo &operator=(foo &f) { return *this; }
 private:
  int *&someRef;
  int *&someOtherRef;
};

int main()
{

  int y;
  int *ptr = &y;
  foo f(ptr);
  *ptr;

  //foo& fooref(ptr);    // this does not compile
  foo& fooref(foo);      // this is viewed as a function prototype
  //foo& fooref(foo) {}  // a function definition is not allowed
  foo& fooref2 = f;  // call to a copy constructor?

  int z;
  foo* fooptr = new foo(&z);

  return 0;
}
