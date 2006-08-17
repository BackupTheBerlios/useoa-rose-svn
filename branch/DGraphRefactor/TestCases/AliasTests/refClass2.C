/*
  refClass2.C

  Goal: Test the use of a reference in forming an alias
        when used in a constructor initializer.
        Also want to verify that the correct MREs are generated when a constructor is called.

*/

class foo {
 public:
  foo(int *x) : someRef(x), someOtherRef(someRef) { *x; *someRef; *(this->someOtherRef); }
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

  int z;
  foo* fooptr = new foo(&z);

  return 0;
}
