/* 
 * from Conditional Pointer Aliasing and Constant Propagation.
 * Anthony Pioli.
 * Technical Report #99-102.
 * State University of New York at New Paltz.
 * January 31, 1999
 * Figure 1.
 */

#include <stdio.h>

int main()
{
  int a, b;
  int *p;

  a = 5;
  b = 10;
  p = &a;

  if ( *p > 5)
    p = &b;
  else 
    b = 20;

  printf("%d", b);
  printf("%d", *p);

  return 0;
}
