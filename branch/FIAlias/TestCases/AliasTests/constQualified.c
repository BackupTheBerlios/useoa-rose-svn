/* constQualified.c */
/*
    *q should access a subset of const_array
*/

extern int g(int i, int j);

const int *const_array;

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

  return 0;
}
