/* pg. 853 in Stroustrup
   For testing SgDotStarOp and SgArrowStarOp
*/
#include <iostream>
using namespace std;

struct C {
    char * val;
    int i;
    void print(int x) { cout << val << x << "\n"; }
    void f1();
    void f2();
    C(char* v) { val = v; }
};

typedef void (C::*PMFI)(int);
typedef char* C::*PM;

void f(C& z1, C& z2) 
{
    C* p = &z2;
    PMFI pf = &C::print;
    PM pm = &C::val;

    z1.print(1);
    (z1.*pf)(2);
    z1.*pm = "nv1";
    p->*pm = "nv2";
    z2.print(3);
    (p->*pf)(4);

}
