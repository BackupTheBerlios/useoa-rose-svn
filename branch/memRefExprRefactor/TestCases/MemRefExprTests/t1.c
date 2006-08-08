/*
  t1.c

  Test command: OATest --oa-MemRefExpr TestCases/MemRefExprTests/t1.c

  Goal: Makes sure that pre and post increment and decrement memory reference
        expressions returned by the AliasIRInterface are correct.

  Output: see TestResults/MemRefExpr
*/
int main()
{
  int j=3;
  int k=2;
  int i=5;
  for(j=1; j<10; j=j+k)
  {
     while(i)
    {
      i--;
      k = --i;
      k = i++;
      ++i;
    }
  }
  return j-k;
 
}
