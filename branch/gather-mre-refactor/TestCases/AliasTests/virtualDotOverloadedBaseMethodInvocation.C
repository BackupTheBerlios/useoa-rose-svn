
/* Purpose:  Test the invocation of a base class' virtual method
 *           on an object receiver, when the method is overloaded.
 */

// implicit assignments should occur at class definition for vtable model.
class Base {
public:
  Base() { }
  Base(Base &b) { }
  ~Base() { }
  Base &operator=(Base &b) { return *this; }
  virtual void virtMethodBase() { } 
  virtual void virtMethod2(int x) { }
};

// implicit assignments should occur at class definition for vtable model.
class Foo : public Base {
public:
  // Define all of the special methods, since we do not want to
  // test normalization in this example.
  Foo() { }
  Foo(Foo &f) { }
  ~Foo() { }
  Foo &operator=(Foo &f) { return *this; }
  virtual void virtMethod1() { }
  virtual void virtMethod2(int x) { }
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
