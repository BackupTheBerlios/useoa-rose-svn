/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 1c.
*/

/*
  RyderEtAlFig1c.c

  Test command: 
    OATest -edg:w --oa-FIAlias -c TestCases/AliasTests/RyderEtAlFig1c.c

  Goal: 

  Output: see TestResults/FIAlias
*/

int main()
{
  int s;
  int *r;
  int **q;
  int ***p;

  if ( s < 3 )
    p = &q;
  else
    r = &s;
  q = &r;
  
  ***p = 5;
  **q;
  *r;
  return 0;
}
