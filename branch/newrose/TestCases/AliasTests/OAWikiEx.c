#include<stdlib.h>

int g;

int foo(int *a) 
{
  int y;
  int *p;

  y = 5;
  g = 6;

  p = a;

 S4:
  p = (int *)malloc(sizeof(int)*y);

  *a = 7;
  a = p;
  *a = 8;

  return 0;
}
