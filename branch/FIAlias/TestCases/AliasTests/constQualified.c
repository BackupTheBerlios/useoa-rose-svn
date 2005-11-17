/* constQualified.c */

extern int g(int i, int j);

const int *const_array;

int main()
{
  int *p;
  int i;

  int x, y;
  const int *q = &const_array[i];
  x = *q;
  *p = 5;
  y = *q;
  g (x, y);

  return 0;
}
