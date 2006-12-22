// NOTE: *a and *b will alias each other but they do not map to any locations

int main()
{
  int *a;
  int *b;

  a = b;

  return 0;
}

