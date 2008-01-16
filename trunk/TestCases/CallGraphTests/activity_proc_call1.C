/* activity_proc_call1.c
 *
 * copied from UseOA-Open64/TestCases/CallGraph/activity_proc_call1.f
 */

void bar(double a, double& b); // prototype for bar()

void head(double x, double f) {
  double t1, t2, t3;

  // c$openad INDEPENDENT(x)
  t1 = x * f;
  bar(t1, t2);
  t3 = f * 30;
  f = t1 + t2;
  // c$openad DEPENDENT(f)

}

void bar(double a, double& b) {

  b = a;
  return;
}


