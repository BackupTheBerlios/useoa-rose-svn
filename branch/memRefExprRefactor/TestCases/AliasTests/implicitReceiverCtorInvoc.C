
/*  Purpose:  Test a constructor invocation with an implicit receiver.  UseOA should make the receiver explicit. */

class Base {
public:
  Base() { }
  ~Base() { }
};

class Foo : public Base {

public:
  Foo() : Base() { }
  ~Foo() { }
};

int main()
{
    return 0;
}
