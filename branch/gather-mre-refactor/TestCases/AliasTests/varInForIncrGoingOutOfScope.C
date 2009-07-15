
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           that destructors are being properly inserted where
 *           they are implicit-- here when a variable declared 
 *           in a loop incrementer goes out of scope.
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
    Foo f;

    for(;;Foo()) {

    }

    // Intended conversion:
    // 
    // From:
    //  
    // for (Stmts1(sc); Stmt2(sc); Stmt3(sc)) {
    //     Stmts4;
    // }
    //
    // To:
    //
    // {
    //     Stmts1(sc);
    //     destroy temporary objects from Stmts1(sc)
    //     for(;;) {
    //         if (Stmt2(sc)) {
    //             destroy temporary objects from Stmt2(sc) here
    //         } else {
    //             destroy temporary objects from Stmt2(sc) here
    //             destroy named objects from Stmt2(sc) here
    //             break;
    //         }
    //         Stmts4;
    //         destroy temporary objects from Stmts4;
    //         destroy named objects from Stmts4;
    //         Stmt3(sc);
    //         destroy temporary from Stmt3;
    //         destroy named from Stmt3;
    //         destroy named objects from Stmt2(sc) here
    //     }
    //     destroy named objects from Stmts1(sc)
    // }

    return 0;
}
