
/** Purpose:  Test ROSE normalization. In particular, test that
 *            special method invocation "stubs" are fixed.  i.e.,
 *            ensure that call sites which invoke underfined special
 *            methods are patched up so that they refer to the
 *            newly-defined methods.
 */

// Should insert following special methods:
//   NoDefinedSpecialMethods() { }
//   NoDefinedSpecialMethods(NoDefinedSpecialMethods &rhs) { }
//   ~NoDefinedSpecialMethods() { }
//   NoDefinedSpecialMethods &operator=(NoDefinedSpecialMethods &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     return *(this);
//   }
class NoDefinedSpecialMethods {
 public:
  
};


int main()
{
  // In all cases, we should see Call handles/Call MREs for a
  // special method.

  // Invoke undefined constructor for stack variable.
  NoDefinedSpecialMethods ndsm;

  // Invoke undefined constructor for dynamic allocation.
  NoDefinedSpecialMethods *ndsmPtr = new NoDefinedSpecialMethods;

  // Invoke undefined copy constructor on stack variable.
  NoDefinedSpecialMethods ndsmCopy(ndsm);

  // Invoke undefined copy constructor for dynamic allocation.
  NoDefinedSpecialMethods *ndsmPtrCopy = new NoDefinedSpecialMethods(ndsm);

  // Invoke undefined assignment operator.
  ndsmCopy = ndsm;

  // Invoke undefined destructor on ndsmPtr.
  delete ndsmPtr;

  // Invoke undefined destructor on ndsm as it goes out of scope.
  return 0;
}
