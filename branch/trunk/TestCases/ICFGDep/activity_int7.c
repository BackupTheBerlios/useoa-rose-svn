//----------------
// activity_int7.c
//
// converted from UseOA-Open64/TestCases/ICFGDep/activity_int7.f

double foo() {
  double x1, y1, x2, y2;

  x1 = 3;
  x1 = 4 + x1;
  y2 = x1;
  y2 = x1 + y2;
  y2 = y1 * x1 + 3;
  y2 = y1 * x1 + 3 * y2;

  return y2;
}

void head() {
  double x[3]; // was x(2), don't want to change subscripts ...
  double y[3];
  int i;

  i = 1;
  x[1] = 5;
  x[1] = 6 + x[1];
  x[2] = 7 + x[1];
  y[1] = x[1];
  y[2] = x[i];
  y[i] = x[2];
  y[1] = x[i+1];
  y[i] = x[i] + y[i];
  y[2] = foo();
}       

int main() {
  head();
  return (0);
}
       
