
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           that destructors are being properly inserted where
 *           they are implicit-- here when a variable declared 
 *           in a return goes out of scope.
 */

class Foo {
public:
  // Define all of the special methods, since we do not want to
  // test normalization in this example.
  Foo() : mVar(0) { }
  Foo(const Foo &f) { }
  ~Foo() { }
  Foo &operator=(const Foo &f) { }
  int mVar;
};

Foo returnTmpFoo()
{
    return Foo();
}

Foo returnNamedFoo()
{
    Foo f;
    return f;
}

int main()
{
    Foo b = returnTmpFoo();

    b = returnNamedFoo();

    return 0;
}
