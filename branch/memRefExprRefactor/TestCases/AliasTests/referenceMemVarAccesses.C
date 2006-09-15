
/**
 *  Purpose:  test reference member variable accesses.
 */

class Foo {
public:
  Foo(int x) : mX(x) { }
  Foo(Foo &f) : mX(f.mX) { }
  Foo &operator=(Foo &f) { return *this; }
  ~Foo() { }

  int &mX;
  Foo *mFooPtr;
};

int main()
{
  int x;
  Foo *f = new Foo(x);
  f->mFooPtr = new Foo(x);
  f->mFooPtr->mX = x;

  Foo obj(x);
  obj.mX = x;  
  return 0;
}
