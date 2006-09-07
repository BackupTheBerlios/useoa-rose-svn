
/*  Purpose:  Test a destructor invocation with an implicit receiver.  ROSE should make the receiver explicit. */

class Base {
public:
  Base() { }
  ~Base() { }
};

class Foo : public Base {

public:
  Foo(int x) { }
  ~Foo() { Base::~Base(); }
};

int main()
{
    return 0;
}
