
/*
  Purpose:  test the initialization of reference variables in 
            a constructor initialization list.
*/

class foo {
 public:
  foo(int x) : someRef(x), someOtherRef(someRef), constRef(5) { }
  ~foo() { }
  foo &operator=(foo &f) { return *this; }
 private:
  int &someRef;
  int &someOtherRef;
  const int& constRef;
};

int main()
{
  return 0;
}
