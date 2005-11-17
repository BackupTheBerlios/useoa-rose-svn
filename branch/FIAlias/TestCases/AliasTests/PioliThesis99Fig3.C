/* 
 * from Conditional Pointer Aliasing and Constant Propagation.
 * Anthony Pioli.
 * Technical Report #99-102.
 * State University of New York at New Paltz.
 * January 31, 1999
 * Figure 3.
 */

#include <stdio.h>

int main()
{
  int a, b;
  int *p;

  a = 5;
  b = 10;
  p = &a;

  while ( *p > 5) {
    p = &b;
    b = 20;
  }

  printf("%d", b);
  printf("%d", *p);

  return 0;
}
