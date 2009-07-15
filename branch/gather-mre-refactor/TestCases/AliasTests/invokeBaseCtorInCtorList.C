
/*  Purpose:  Test an invocation of a base constructor with an implicit 
 *            receiver.  UseOA should make the receiver explicit. 
 */

class Base {
public:
  Base() { }
  Base(Base &b) { }
  ~Base() { }
};

class Foo : public Base {

public:
  Foo(Base &b) : Base(b) { }
  ~Foo() { }
};

int main()
{
    return 0;
}
