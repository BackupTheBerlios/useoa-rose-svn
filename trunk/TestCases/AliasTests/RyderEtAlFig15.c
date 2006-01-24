/* from A Schema for Interprocedural Modification Side-Effect Analysis
   with Pointer Aliasing, Barbara G. Ryder, William A. Landi,
   Philip A. Stocks, Sean Zhang, and Rita Altucher.  TOPLAS 2001.
   Figure 15.
*/

#include <stdlib.h>

struct S { int a, b, c; } x, *s;
struct T { float f, g; } y, *t;

/* x and y are initialized elsewhere in the program */

char *my_malloc(int size)
{
  char *p = (char *)malloc(size);
  return p;
}

void a()
{
  s = (struct S *)my_malloc(sizeof(struct S));
  *s = x;
}

void b()
{
  t = (struct T *)my_malloc(sizeof(struct T));
  *t = y;
}


