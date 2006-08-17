/* from the OpenAnalysis Wiki.  evidently from Jean Utke's 
   basic block flattening paper. */

int main()
{
  double x, z, y;
  double *p;

  p = &x;
  z = *p;
  p = &z;
  y = *p/2;

  return 0;
}
