/* multi_func_calls.c
 *
 * includes a function call with a function call as an actual parameter
 */

#include <math.h>

int foo (int a); // function prototype
int bar (int b); // function prototype
int read();      // function prototype

int main() {

  int x = read();

  x = foo(bar(x));

  return 0;
}

int foo (int a) {
  int res;
  if (a > 0) {
    res = 1;
  } else {
    res = 5;
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
