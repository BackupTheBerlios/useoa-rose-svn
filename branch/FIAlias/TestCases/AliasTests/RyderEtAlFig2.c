/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 2.
*/

int **p, *q, *r;
int *x, y;

void A()
{
  *p = x;
}

int main()
{
  p = &r;
  if ( y < 3 ) {
    p = &q;
    A();
  } else {
    x = &y;
    A();
  }

  return 0;
}
