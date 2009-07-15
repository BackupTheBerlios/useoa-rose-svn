/* constQualified.c */
/*
    *q should access a subset of const_array
*/

extern int g(int i, int j);

int a[20];
int *const const_array = a;

int main()
{
  int *p;
  int i;

  int x, y;
  const int *q = &const_array[i];
  x = *q;
  p = &i;
  *p = 5;
  y = *q;
  g (x, y);
  const_array[i];

  return 0;
}
