
/*
  refParam.C

  Test command: 
    OATest -edg:w --oa-FIAlias -c TestCases/AliasTests/refParam.C

  Goal: Test the use of a reference parameter in forming an alias
        when used in an assignment.

  Output: see TestResults/FIAlias
*/


int *b;

// a is a reference to a pointer.
void foo(int *&a)
{
  a = b;
}

int main()
{
  int m;
  b = &m;
  int *x;
  foo(x);
  *b;
  return 0;
}
