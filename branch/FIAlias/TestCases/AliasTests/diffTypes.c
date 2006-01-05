/* diffTypes.c */

extern void g(int i, int j);

int main()
{
  double *ps, x;
  int *pi;

  int i, j;
  pi = &j;
  ps = &x;
  i = *pi;
  *ps = 3;
  j = *pi;
  g (i, j);

  return 0;
}

