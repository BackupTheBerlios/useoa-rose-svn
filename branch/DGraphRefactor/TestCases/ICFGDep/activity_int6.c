/*
 * activity_int6.c
 * 
 * converted from UseOA-Open64/TestCases/ICFGDep/activity_int6.f
 */


void head() {
  double x[3];
  double x1, x2;
  double y[3];
  double y1, y2;
  int i;

  i = 1;
  x1 = 3;
  x1 = 4 + x1;
  y2 = x1;
  y2 = x1 + y2;
  x[1] = 5;
  x[1] = 6 + x[1];
  x[2] = 7 + x[1];
  y[1] = x[1];
  y[2] = x[i];
  y[i] = x[2];
  y[1] = x[i+1];
  y[i] = y[i] + x[i];

  if (i < 2) {
    y2 = x1;
  } else {
    y1 = x2;
  }

  y2 = y1 * x1 + 3;
  y2 = y1 * x1 + 3 + y2;
}

int main() {
  head();
  return (0);
}

