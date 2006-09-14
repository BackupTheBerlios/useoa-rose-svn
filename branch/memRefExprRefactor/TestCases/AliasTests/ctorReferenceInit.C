
/*
  Purpose:  test the initialization of reference variables in 
            a constructor initialization list.
*/

class foo {
 public:
  foo(int x) : someRef(x), someOtherRef(someRef), constRef(5) { }
  ~foo() { }

  foo &operator=(foo &f) { return *this; }
  // *this is the object that this points to
  // we are returning a reference to it, which we model as a pointer
  // therefore we end up with two memory reference expressions to this:
  //    this and &*this (which gets normalized to this)
  // Having both uses is a little redundant but will not effect the results.
  
 private:
  int &someRef;
  int &someOtherRef;
  const int& constRef;
};

int main()
{
  return 0;
}
