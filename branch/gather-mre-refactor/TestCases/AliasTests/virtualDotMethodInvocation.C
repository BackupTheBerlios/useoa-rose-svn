
/* Purpose:  Test a virtual method invocation.
 */

// implicit assignments should occur at class definition for vtable model.
class Foo {
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
   // implicit assignments should occur here for non-vtable model, with
   // only the single virtual function table ptr assignment for the
   // vtable model.
   Foo f;
   f.virtMethod1();
   return 0;
}
