
/*  Purpose:  Test parameter bindings for method invocations. */

class Foo {

public:
  Foo() { }
  void someMethod(int *x) { }
};

int main()
{
    Foo f;
    int x;

    f.someMethod(&x);
    return 0;
}
