struct S
{
  int a;
  int b;
};

int main(int x, int y)
{
  S s;
  s.a=x;
  s.b=x+y;
  return s.a+s.b;
  
}
