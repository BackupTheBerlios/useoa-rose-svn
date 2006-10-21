
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           that implicit invocation of base class special
 *           methods are made explicit under inheritance
 *           and only where they are not already defined.
 */


// Should create the following:
//     Foo(Foo &rhs) { }
//     ~Foo() { }
//     Foo &operator=(Foo &rhs) {
//         if (this == &rhs) {
//             return *this;
//         } else {
//         }
//         return *this;
//     }
class Foo {
 public:
  Foo() { }
};

// Should create the following:
//     ~Bar() { }
//     Bar &operator=(Bar &rhs) {
//         if (this == &rhs) {
//             return *this;
//         } else {
//         }
//         return *this;
//     }
class Bar {
 public:
  Bar() { }
  Bar(Bar &b) { }
};

// Should create the following:
//     FooBar() : Foo() { }
//     FooBar(FooBar &rhs) : Foo(rhs) { }
//     ~FooBar() { ((Foo*)this)->~Foo(); }
//     FooBar &operator=(FooBar &rhs) {
//         *((Foo *)this) = (Foo &)rhs;
//         if (this == &rhs) {
//             return *this;
//         } else {
//         }
//         return *this;
//     }
class FooBar : public Foo {
 public:
};

int main()
{
  FooBar fb;
  FooBar fb2(fb);
  return 0;
}
