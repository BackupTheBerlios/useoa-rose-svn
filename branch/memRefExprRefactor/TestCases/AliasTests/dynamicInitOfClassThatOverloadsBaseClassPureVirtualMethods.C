
/* Purpose:  Test that implicit assignments for virtual method fields
 *           are being generated for a dynamic variable initialization,
 *           when the base class has virtual methods (one of which is pure), 
 *           which are overloaded in the derived class.
 */

// implicit assignments should occur at class definition for vtable model.
class Base {
  Base() { }
  Base(Base &b) { }
  ~Base() { }
  operator=(Base &b) { }
  virtual virtMethodBase() { } 
  virtual virtMethod2(int x) = 0;
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
   Foo *f = new Foo;
   return 0;
}
