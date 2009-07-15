/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 3.
*/

int *p, q, r;

void B()
{

}

void A()
{
  B();
}

int main()
{
  p = &q;
  A();
  p = &r;
  A();
  return 0;
}
