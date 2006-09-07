
/* Purpose:  Test the invocation of a base class' virtual method
 *           on an object receiver, when the method is overloaded.
 */

// implicit assignments should occur at class definition for vtable model.
class Base {
  Base() { }
  Base(Base &b) { }
  ~Base() { }
  operator=(Base &b) { }
  virtual virtMethodBase() { } 
  virtual virtMethod2(int x) { }
};

// implicit assignments should occur at class definition for vtable model.
class Foo {
public:
  // Define all of the special methods, since we do not want to
  // test normalization in this example.
  Foo() { }
  Foo(Foo &f) { }
  ~Foo() { }
  operator=(Foo &f) { }
  virtual virtMethod1() { }
  virtual virtMethod2(int x) { }
};

int main()
{
   // implicit assignments should occur here for non-vtable model--
   // we should get assignments for virtMethod1, virtMethod2, and
   // virtMethodBase.  The vtable model should have a single
   // virtual function table ptr assignment for the
   // vtable model.
   Foo f;
   int x;

   // Call handle should be a NamedRef, as this may be statically 
   // resolved.
   f.virtMethod2(x);
   return 0;
}
