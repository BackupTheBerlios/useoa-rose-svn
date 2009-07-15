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
  //int& x = 4;           // does not compile  
  //int& x;               // this doesn't compile either
  //x = 4;
  *ptr;
  *refPtr;                // read as **refPtr;

  return 0;
}
