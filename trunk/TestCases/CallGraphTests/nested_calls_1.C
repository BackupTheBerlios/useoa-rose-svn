/* nested_calls_1.c
 * 
 * converted from UseOA-Open64/TestCases/CallGraph/nested_calls_1.f
 */

#include <math.h>

void foo(double a, double b, double& c);  // prototype for foo()
void bar(double& a);                      // prototype for bar()

void head(double x[4], double y[4]) {

  //c$openad INDEPENDENT(x)
  foo(x[1],x[2],y[1]);
  y[2]=sin(x[1]*x[2]);
  y[3]=sin(x[3]);
  y[4]=cos(x[4]);
  //c$openad DEPENDENT(y)
}

void foo(double a, double b, double &c) {

  bar(b);
  c=a*a+b;
}

void bar(double &a) {

  a=cos(a*a);

}
