/* funCall.c */
/*
  *i and a end up in the same equiv set during FIAlias.
*/

void makeAlias(int **i, int **j)
{
  *i = *j;
}

int *array;

int main()
{
  int loc;
#if 0
  int *a;
  int *b;

  a = b = &loc;
#else
  int *a;
  int *b;

  b = &loc;
  makeAlias(&a, &b);
#endif
  *a;
  *b;
  
  return 0;
}
