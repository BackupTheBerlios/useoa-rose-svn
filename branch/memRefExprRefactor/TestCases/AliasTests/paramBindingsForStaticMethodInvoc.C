
/*  Purpose:  Test parameter bindings for static method invocations. */

class Foo {

public:
  Foo() { }
  static void someMethod(int *x) { }
};

int main()
{
    int x;

    Foo::someMethod(&x);
    return 0;
}
