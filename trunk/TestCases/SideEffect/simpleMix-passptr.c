/*
  simpleMix.c

  Has main just calling one function, but there are a mix of parameter
  types.

  At the f(x, &c) call, should end up with c being used by not modified.
  x should not be used or modified at the call.  x is used when it is
  passed as an actual parameter, so it is locally used within main.
*/
#include<stdio.h>

void f(int n, int *q) {
  int p;
  n++;
  p = n - 1 + *q;
}

int main() {
  int x;
  int a, b, c;
  f(x, &c);
  return 0;
}
