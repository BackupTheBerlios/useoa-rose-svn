
/*  Purpose:  Test a member object definition with an implicit receiver
 *            within a constructor initializer list.
 *            UseOA should make the receiver explicit. 
 */

class Bar {
public:
  Bar(Bar &b) { }
  Bar() { }
  ~Bar() { }
};

class Foo {

public:
  Foo(Bar b) : mBar(b) { }
  ~Foo() { }
  Bar mBar;
};

int main()
{
    return 0;
}
