/* multi_func_calls_2.c
 *
 * includes a function call with a function call as an actual parameter
 */

#include <math.h>

int foo (int a, int d); // function prototype
int bar (int b); // function prototype
int zoo (int c);
int read();      // function prototype

int main() {

  int x = read();

  x = foo(bar(x),zoo(x));

  return 0;
}

int foo (int a, int d) {
  int res;
  if (a > 0) {
    res = 1+d;
  } else {
    res = 5+d;
  }
  return res;
}

int bar (int b) {
  int res;
  if (b > 180) {
    res = (int)(5*cos(b));
  } else {
    res = 0;
  }
  return res;
}

int zoo (int c) {
  int res;
  if (c < 10) {
    res = 3;
  } else {
    res = 5;
  }
  return res;
}
