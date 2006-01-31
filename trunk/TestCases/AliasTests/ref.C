/*
  ref.C

  Test command: 
    OATest -edg:w --oa-FIAlias -c TestCases/AliasTests/ref.C

  Goal: Test the use of a reference in forming an alias
        when used in an assignment.

  Output: see TestResults/FIAlias
*/

int main()
{
  int m;
  int *ptr = &m;

  int *& refPtr = ptr;    // read as int **refPtr = &ptr;
  *ptr;
  *refPtr;                // read as **refPtr;

  return 0;
}
