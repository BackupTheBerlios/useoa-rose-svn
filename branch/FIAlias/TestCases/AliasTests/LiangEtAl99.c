/* from Equivalence Analysis:  A General Technique to Improve the
 * Efficiency of Data-flow Analyses in the Presence of Pointers.
 * Donglin Liang and Mary Jean Harrold.  
 * Workshop on Program Analysis for Software Tools and Engineering
 * (PASTE '99)
 * Figure 1.
 */

int g, g1;

void f2(int *q) {
  *q = g;
  g1++;
}

void f1(int *p) {
  *p = g + 1;
  if ( g > 0 )
    p = &g;
  f2(p);
  *p++;
}

int main() {
  int x, y, z;
  f1(&x);
  f1(&y);
  f2(&z);

  x;
  y;
  z;

  return 0;
}
