
/*  Purpose:  Test a method invocation with an implicit receiver.  ROSE should add one. */

class Foo {

public:
  Foo() { }
  void someOtherMethod() { }
  void someMethod(int *x) { someOtherMethod(); }
};

int main()
{
    return 0;
}
