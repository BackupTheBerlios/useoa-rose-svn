/*
  Testing MemRefExprs for some of the SomethingAssignOps.
  For example, SgPlusAssignOp, SgAndAssignOp, etc.
*/

int main() {
    int a, b;
    a = 20;

    a++;

    b = ++a;
    b = a++;

    a += 45;

    return a = b -= a;

}
