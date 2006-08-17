
int main()
{
  int loc;
  int *locPtr;
  int *a;
  int *b;

  locPtr = &loc;

  a = locPtr;
  b = locPtr;

  *a;
  *b;
  return 0;
}
