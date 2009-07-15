
/*  Purpose:  Test parameter bindings for method invocations through a ptr. */

class Foo {

public:
  Foo() { }
  void someMethod(int *x) { }
};

int main()
{
    Foo *f = new Foo;
    int x;

    f->someMethod(&x);
    return 0;
}
