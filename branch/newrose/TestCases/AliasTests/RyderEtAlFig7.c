/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 7.
*/

int main()
{
  int b, *a, **x, **y;

  a = &b;
  x = &a;
  y = x;
  *x = 0;
  **x;
  **y;

  return 0;
}
