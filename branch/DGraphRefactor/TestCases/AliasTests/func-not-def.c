/*
 * A program that makes some calls to undefined functions.
 */

int* foo(int*);

int main()
{
  int *a;
  int *b;

  a = b;

  b = foo(a);

  return 0;
}
