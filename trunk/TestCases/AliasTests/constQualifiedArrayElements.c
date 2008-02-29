/* constQualified.c */
/*
    *q should access a subset of array_with_const_elements
*/

extern int g(int i, int j);

int a[20];
const int *array_with_const_elements = a;

int main()
{
  int *p;
  int i;

  int x, y;
  const int *q = &array_with_const_elements[i];
  x = *q;
  p = &i;
  *p = 5;
  y = *q;
  g (x, y);
  array_with_const_elements[i];

  return 0;
}
