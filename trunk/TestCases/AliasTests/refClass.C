
/*
  refClass.C

  Test command: 
    OATest -edg:w --oa-FIAlias -c TestCases/AliasTests/refClass.C

  Goal: Test the use of a reference in forming an alias
        when used in a constructor initializer.

  Output: see TestResults/FIAlias
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

  return 0;
}
