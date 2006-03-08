/*
 * from Reuse-Driven Interprocedural Slicing in the Presence of
 * Pointers and Recursion.
 * Donglin Liang and Mary Jean Harrold.
 * Proceedings of the International Conference on Software Maintanence'99.
 * Figure 2.
 */

#include <stdio.h>

int j, sum;

void incr(int *q)
{
  *q = *q + 1;
}

void reset(int *s) 
{
  *s = 0;
}

void f(int *p)
{
  if (j < 0)
    incr(&j);
  sum = sum + j;
  incr(p);
  scanf("%d", &j);
}

int main()
{
  int sum1, i1, i2;
  reset(&sum);
  reset(&i1);
  scanf("%d", &j);
  while(i1 < 10)
    f(&i1);
  sum1 = sum;
  reset(&i2);
  while(i2 < 20)
    f(&i2);
  printf("%d", sum1);
  return 0;
}

