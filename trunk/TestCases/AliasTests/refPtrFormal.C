
/**
 *  Purpose:  Test handling of param bindings for reference formal.
 */

void nonConstRefPtrArg(int *&arg) { }

void constRefPtrArg(int *const &arg) { }

int main()
{
  int x;
  int *xPtr = &x;
  int *&xRefPtr = xPtr;   // reference to a pointer.

  // ptr assign: <arg, &xPtr>
  nonConstRefPtrArg(xPtr);
  // ptr assign: <arg, xRefPtr>
  nonConstRefPtrArg(xRefPtr);

  // ptr assign: <*arg, &x>  // &x is not an lval, so can not take its address.
  constRefPtrArg(&x);
  // ptr assign: <arg, xPtr>
  constRefPtrArg(xPtr);
  // ptr assign: <arg, xRefPtr>
  constRefPtrArg(xRefPtr);

  return 0;
}
