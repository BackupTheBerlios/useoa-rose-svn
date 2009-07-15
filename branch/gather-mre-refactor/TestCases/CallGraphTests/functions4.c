/* functions4.c
 * 
 * converted from UseOA-Open64/TestCases/CallGraph/functions4.f90
 */

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!
//! A simple program with a function (instead of subroutine)
//!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <math.h>

int bar(int n); // prototype for function bar

int foo(int n) {
  int res;
  if (n == 1) {
    res = 1;
  } else {
    res = 5;
  }
  return res;
}

void functiontest() {

  int n = 7;
  float x = 4.5;

  n = foo(n);
  //! call foo(n) ! what happens to return?

  x = sin(x);

}

int bar(int n) {
  int res;
  if (n == 1) {
    res = 1;
  } else {
    res = 5;
  }
  return res;
}



