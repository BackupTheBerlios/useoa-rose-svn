
int main()
{
  int loc, loc2;
  int *locPtr;
  int *a;
  int *b;

  locPtr = &loc;

  a = locPtr;
  locPtr = &loc2;
  b = locPtr;

  *a;
  *b;
  return 0;
}
