
/**
 *  Purpose:  test whether locations are being correctly
 *            marked as local/non-local
 */

// foo should be non-local in main.
int foo() { }

int main()
{
  int x;    // x should be local in main.
  x = 3;
  return 0;
}
