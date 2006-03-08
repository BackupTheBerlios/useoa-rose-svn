/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 9.
*/

int *p, *q, *r;
int a, b;

void proc1()
{
  r = q;
  *r = 1;
}

void proc2(int *f)
{
  *f = 0;
}

int main()
{
  q = &a;
  p = q;
  *p = 1;
  proc1();
  q = &b;
  *q = 1;
  *r = 2;
  proc1();
  proc2(&a);

  return 0;
}
