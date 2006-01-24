/* from Pointer Analysis for Programs with Structures and Casting
 * Suan Hsi Yong, Susan Horwitz, Thomas Reps.
 * SIGPLAN Conference on Programming Language Design and Implementation 1999 
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
  return 0;
}
