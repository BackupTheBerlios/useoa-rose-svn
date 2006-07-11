/* functions2.c
 * 
 * converted from UseOA-Open64/TestCases/CallGraph/functions2.f
 */
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!
//! A simple program with a recursive function
//!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

int factorial(int n) {
  int res;
  if (n == 1) {
    res = 1;
  } else {
    res = n*factorial(n-1);
  }
  return res;
}


int main () {

  int n = 7;

  n = factorial(n);

  return(0);
  
}
