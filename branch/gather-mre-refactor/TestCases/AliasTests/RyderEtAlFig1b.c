/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 1b.
*/

/*
  RyderEtAlFig1b.c

  Test command: 
    OATest -edg:w --oa-FIAlias -c TestCases/AliasTests/RyderEtAlFig1b.c

  Goal: 

  Output: see TestResults/FIAlias
*/

int main()
{
  int *q;
  int y;
  int z;
  int **p;

  q = &z;
  if ( y < 3 )
    p = &q;
  else
    q = &y;
  **p = 5;

  z;
  *q;

  return 0;
}
