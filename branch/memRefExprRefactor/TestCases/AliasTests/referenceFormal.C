
void nonConstArg(int &x)
{

}

void constArg(const int& x)
{

}

int main()
{
  int x;
  int &xRef = x;

  nonConstArg(x);
  nonConstArg(xRef);

  constArg(5);
  constArg(x);
  constArg(xRef);

  return 0;
}
