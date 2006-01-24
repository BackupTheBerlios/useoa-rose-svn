
class foo {
  foo(int x) : someRef(x), someOtherRef(someRef) { }
 private:
  int &someRef;
  int &someOtherRef;
};
