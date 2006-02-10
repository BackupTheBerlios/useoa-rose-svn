
/*
  diffArrays.c

  Test command: 
    OATest -edg:w --oa-FIAlias -c TestCases/AliasTests/diffArrays.c

  Goal: 

  Output: see TestResults/FIAlias
*/

#include <stdlib.h>

//int *a, *b;

//int *b; int a[10];
// int b[10]; int a[10];

int main()
{
#if 1
  int i, j;
  //  a = new int[5];
  int a[100];
  int *b;
  b = new int[5];

  int *p, *q;
  int x, y;
  p = &a[i];
  q = &b[j];
  x = *(q + 3);
  *p = 5;
  y = *(q + 3);

  *(a+i);
  a[i];
  b[i];
  a;
  *a;
#else
  int i, j;
  a = new int[5];

  int *p;
  p = &a[i];

  a[i];
  a;
  *a;
  *p;
  //  (a + j + j);
  //  *(a + j);
#endif

  return 0;
}
