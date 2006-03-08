/*
  t5.c

  Test command: OATest --oa-MemRefExpr TestCases/MemRefExprTests/t5.c

  Goal: Testing memory reference expressions that involve an 
        address of operation.

  Output: see TestResults/MemRefExpr
*/
int i;

int main(int b)
{
  int * p=&i;
  *p=b;
  return *p;
}
