/* notConstQualified.c */

extern int g(int i, int j);

int *array;

int main()
{
  int *p;
  int i;

  int x, y;
  int *q = &array[i];
  x = *q;
  *p = 5;
  y = *q;
  g (x, y);

  return 0;
}
