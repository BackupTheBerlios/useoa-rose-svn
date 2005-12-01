/* funCall.c */
/*
  Although *i and a end up in the same union-find set during FIAlias analysis,
  in the results they end up in different equiv sets because the *i memory
  reference occurs in the makeAlias procedure and a occurs in the main 
  procedure.  Each procedure gets its own equiv set for each union-find set
  so that the memory references only map to locations visible within 
  a particular procedure.
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
