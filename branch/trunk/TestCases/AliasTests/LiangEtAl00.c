/* from Towards Efficient and Accurate Program Analysis Using
 * Light-Weight Context Recovery.
 * Donglin Liang and Mary Jean Harrold.
 * International Conference on Software Engineering.
 * pp 366-376.  June, 2000
 * Figure 1.
 */

#include<stdio.h>

int x;

void f(int *p) {
  *p = *p + x;
}

void f1(int *q) {
  int n = 0;
  f(&n);
  f(q);
}

int y;

int main() {
  int w = 1;
  x = 1;
  y = 1;
  f1(&y);
  f(&w);
  printf("%d",w);
  return 0;
}
