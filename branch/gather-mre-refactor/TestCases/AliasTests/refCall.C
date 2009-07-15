
int c;

void foo(int &a, int *b)
{
  a = c;
}

int main()
{
  int x;
  foo(x, &x);
  return 0;
}
