/* diffTypes.c */

extern void g(int i, int j);

int main()
{
  double *ps;
  int *pi;

  int i, j;
  i = *pi;
  *ps = 3;
  j = *pi;
  g (i, j);

  return 0;
}

