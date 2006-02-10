/* from Pointer Analysis for Programs with Structures and Casting
 * Suan Hsi Yong, Susan Horwitz, Thomas Reps.
 * SIGPLAN Conference on Programming Language Design and Implementation 1999 
 */

/*
  YongEtAlPLDI99.c

  Test command: 
    OATest -edg:w --oa-FIAlias -c TestCases/AliasTests/YongEtAlPLDI99.c

  Goal: 

  Output: see TestResults/FIAlias
*/

struct S { int *s1; int *s2; } s;

int main()
{
  int x, y, *p;
  s.s1 = &x;
  s.s2 = &y;
  p = s.s1;
  // MMS, added some more refs
  *(s.s1);
  *(s.s2);
  *p;
  return 0;
}
