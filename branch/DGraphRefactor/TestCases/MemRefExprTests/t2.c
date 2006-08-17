/*
  t2.c

  Test command: OATest --oa-MemRefExpr TestCases/MemRefExprTests/t2.c

  Goal: Test memory references to structures.

  Output: see TestResults/MemRefExpr
*/
struct S
{
  int a;
  int b;
};

int main(int x, int y)
{
  S s;
  s.a=x;
  s.b=x+y;
  return s.a+s.b;
  
}
