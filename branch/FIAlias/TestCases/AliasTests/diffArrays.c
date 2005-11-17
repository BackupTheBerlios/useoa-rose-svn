/* diffArrays.c */

extern void g(int i, int j);

int *a, *b;

int main()
{
  int i, j;

  int *p, *q;
  int x, y;
  p = &a[i];
  q = &b[j];
  x = *(q + 3);
  *p = 5;
  y = *(q + 3);
  g (x, y);

  return 0;
}
