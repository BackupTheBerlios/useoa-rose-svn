
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           that destructors are being properly inserted where
 *           they are implicit-- here when a temporary
 *           variable goes out of scope.
 */

class Foo {
public:
  // Define all of the special methods, since we do not want to
  // test normalization in this example.
  Foo() { }
  Foo(Foo &f) { }
  ~Foo() { }
  Foo &operator=(Foo &f) { }
};

int main()
{
    Foo f(Foo());
    // temp needs to be destroyed after here.
    return 0;
    // f.~Foo(); should be inserted around here.
}
