
/*  Purpose:  Test parameter bindings for method invocations. */

class Foo {

public:
  Foo() { }
  ~Foo() { }
};

int main()
{
    Foo *f = new Foo();
    delete f;

    return 0;
}
