/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 1c.
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
  return 0;
}
