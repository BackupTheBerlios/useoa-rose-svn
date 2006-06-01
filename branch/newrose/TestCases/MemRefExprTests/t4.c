/*
  t4.c

  Test command: OATest --oa-MemRefExpr TestCases/MemRefExprTests/t2.c

  Goal: Test memory references involving multiple dereferences.

  Output: see TestResults/MemRefExpr
*/
struct mystruct 
{ 
  int x; 
  struct mystruct *q, *r; 
}; 
 
int main(struct mystruct * p)
{
  p->q->r->x=9;
  return p->q->x;
}
