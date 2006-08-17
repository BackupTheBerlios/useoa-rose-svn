/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 1a.
*/

int main()
{
  int *x;
  int y;
  int z;
  int **p;

  x = &y;
  p = &x;
  x = &z;
  **p = 5;

  return 0;
}
