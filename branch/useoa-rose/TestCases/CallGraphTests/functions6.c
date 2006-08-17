/* functions6.c
 * 
 * converted from UseOA-Open64/TestCases/CallGraph/functions6.f90
 */

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!
//! A simple program with a function (instead of subroutine)
//! Uses multi?-recursion
//!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#include <math.h>

int foo(int); // prototype of function foo

int bar(int n) {
  int res;
  if (n == 1) {
    res = 1;
  } else {
    res = foo(5);
  }
  return res;
}

int foo(int n) {
  int res;
  if (n == 1) {
    res = bar(1);
  } else {
    res = 5;
  }
}

void functiontest() {

  int n = 7;
  float x = 4.5;

  n = foo(n);
  //  ! call foo(n) ! what happens to return?

  x = sin(x);
}



