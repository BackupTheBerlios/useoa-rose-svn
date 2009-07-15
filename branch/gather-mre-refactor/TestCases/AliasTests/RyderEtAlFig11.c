/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 11.
*/

int x, y, k;

void R(int *b)
{
  if (*b) {
    b = &k;
    *b = 0;
  }
  (*b)++;
}

int main()
{
  R(&x);
  R(&y);

  return 0;
}
