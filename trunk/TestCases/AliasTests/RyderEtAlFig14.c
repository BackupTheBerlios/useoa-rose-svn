/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 14.
*/

struct s { int a, b, c; } s1, s2, s3;

/* s1:  struct-assignments only */
/* s2:  field-assignments only */
/* s3:  mixed assignments */

void p() {
  s1 = s2;
}

void q() {
  s2.a = 4;
  s3 = s1;
}

void r() {
  s3 = s2;
  s3.a = 4;
}

int main()
{
  s3.a = 2;
  s3.b = 3;
  s3.c = 4;
  p();
  q();
  r();

  return 0;
}
