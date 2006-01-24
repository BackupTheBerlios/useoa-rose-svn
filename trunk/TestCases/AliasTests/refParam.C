
int *b;

// a is a reference to a pointer.
void foo(int *&a)
{
  a = b;
}

int main()
{
  int *x;
  foo(x);
  return 0;
}
