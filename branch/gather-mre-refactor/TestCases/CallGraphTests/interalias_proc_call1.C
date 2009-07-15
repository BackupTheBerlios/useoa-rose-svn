/* interalias_proc_call1.c
 * 
 * converted from UseOA-Open64/TestCases/CallGraph/interalias_proc_call1.f
 */
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!
//! A simple program that involves aliasing due to a reference parameter.
//!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

void bar(double a, double& b); // prototype for function bar

void head(double x, double& f) {
  double t1, t2, t3;

  t1=x*f;
  bar(t1,t1);
  t3=f*30;
  f=t1+t2;
}

void bar(double a, double& b) {

  b = a;
  return;
}
