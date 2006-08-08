// NOTE: although tecnically *a and *b will alias each other the location they
// refer to is unknown and as such the FIAlias algorithm won't recognize the
// alias.

int main()
{
  int *a;
  int *b;

  a = b;

  return 0;
}

