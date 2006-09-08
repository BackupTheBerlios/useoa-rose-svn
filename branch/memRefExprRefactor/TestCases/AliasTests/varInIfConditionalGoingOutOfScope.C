
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           that destructors are being properly inserted where
 *           they are implicit-- here when a variable declared 
 *           in an if conditional goes out of scope.
 */

class Foo {
public:
  // Define all of the special methods, since we do not want to
  // test normalization in this example.
  Foo() : mVar(0) { }
  Foo(Foo &f) { }
  ~Foo() { }
  Foo &operator=(Foo &f) { }
  // This is required so that we create a
  // named object within conditionals, as in:
  //     while (Foo b = 5) { }
  operator bool() { }   
  int mVar;
};

int main()
{
    Foo b;

    if ( !(Foo().mVar == 5) ) {

    }

    if ( Foo f = b ) {

    }

    // Intended conversion:
    // 
    // From:
    //  
    // if (!Expr1) {
    //     Stmts1;
    // } else {
    //     Stmts2;
    // }
    //
    // To:
    //
    // if (Expr1) {
    //     destroy temporary objects from Expr1(sc)
    //     Stmts2;
    // } else {
    //     destroy temporary objects from Expr1(sc)
    //     Stmts1;
    // }
    // destroy named objects from Expr1(sc)

    return 0;
}
