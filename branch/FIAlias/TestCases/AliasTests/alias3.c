
int main()
{
  int loc;
  int *locPtr;
  int *a;
  int *b;
  int *c;
  int loc2;

  locPtr = &loc;

  a = locPtr;
  b = locPtr;
  c = locPtr;
  a = b = c;
  a = &loc2;

  return 0;
}
