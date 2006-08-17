/*
  t3.c

  Test command: OATest --oa-MemRefExpr TestCases/MemRefExprTests/t3.c

  Goal: Test memory references to a parameter pointer variable.  There should
        be a Deref( NamedRef(x) ) and a NamedRef(x) for the stmt *x = 4.

  Output: see TestResults/MemRefExpr
*/
int * foo(int * x)
{
  *x=4;
  return x;
}
