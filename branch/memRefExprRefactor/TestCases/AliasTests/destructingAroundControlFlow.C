
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           that destructors are being properly inserted where
 *           they are implicit.  Here we test that we insert
 *           destructors properly in the presence of complex control
 *           flow-- break, continue, goto.
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
    bool cond1 = false;
    bool cond2 = true;

    while( Foo b = f ) {
        continue;
        b.mVar = 5;
    }

    while( Foo().mVar != 5 ) {
        break;
    }

    // Intended conversion:
    // 
    // From:
    //  
    // while (Stmt(sc)) {
    //     Stmts;
    // }
    //
    // To:
    //
    // while (true) {
    //    if (Stmt(sc)) {
    //        destroy temporary objects from Stmt(sc) here
    //    } else {
    //        destroy temporary objects from Stmt(sc) here
    //        destroy named objects from Stmt(sc) here
    //        break;
    //    }
    //    Stmts;
    //    destroy named objects from Stmt(sc) here
    // }
 
#if 0
    // Can not do this, according to Peter:
    // This applies to if, for and while conditionals (but not do/while, which
    // may not hold a statement conditional).
    do {
 
    } while ( Foo b = f );
#endif

    do {
        continue;        
    } while (Foo().mVar != 5);

    // Intended conversion:
    // 
    // From:
    //  
    // do {
    //     Stmts;
    // } while (Expr(sc));
    //
    // To:
    //
    // do {
    //    Stmts;
    //    if (Expr(sc)) {
    //        destroy temporary objects from Expr(sc) here
    //        destroy named objects from Expr(sc) here
    //    } else {
    //        destroy temporary objects from Expr(sc) here
    //        destroy named objects from Expr(sc) here
    //        break;
    //    }
    // } while (true);

    for(; Foo(); ) {
        continue;    
    }

    for(; Foo b = f; ) {
        b.mVar = 1;
        switch(b.mVar) {
        case 1:
            {
                break;
            }
        default:
            {
                break;
            }

        }
        break;
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
