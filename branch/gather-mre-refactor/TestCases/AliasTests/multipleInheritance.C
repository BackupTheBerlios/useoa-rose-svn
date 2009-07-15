
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           that implicit invocation of base class special
 *           methods are made explicit, under multiple inheritance
 *           and only where they are not already defined.
 */

// Should create the following:
//     BaseWithNoDefaults() { }
//     BaseWithNoDefaults(BaseWithNoDefaults &rhs) { }
//     ~BaseWithNoDefaults() { }
//     BaseWithNoDefaults &operator=(BaseWithNoDefaults &rhs) {
//         if (this == &rhs) {
//             return *this;
//         } else {
//         }
//         return *this;
//     }
class BaseWithNoDefaults {

};

// Everything is already defined.
class BaseWithDefaults {
 public:
  BaseWithDefaults() { }
  BaseWithDefaults(BaseWithDefaults &bwd) { }
  BaseWithDefaults &operator=(BaseWithDefaults &rhs) { }
  ~BaseWithDefaults() { }
};

class ChildDefault : public BaseWithNoDefaults, 
                     public BaseWithDefaults
{
 public:
// Should create the following:
//     ChildDefault() : BaseWithNoDefaults(), BaseWithDefaults() { }
//     ChildDefault(ChildDefault &rhs) 
//         : BaseWithNoDefaults(rhs), BaseWithDefaults(rhs) { }
//     ~ChildDefault() { 
//         ((BaseWithNoDefaults *)this)->~BaseWithNoDefaults();
//         ((BaseWithDefaults *)this)->~BaseWithDefaults();
//     }
//     ChildDefault &operator=(ChildDefault &rhs) {
//         *((BaseWithNoDefaults *)this) = (BaseWithDefaults &)rhs;
//         *((BaseWithDefaults *)this) = (BaseWithNoDefaults &)rhs;
//         if (this == &rhs) {
//             return *this;
//         } else {
//         }
//         return *this;
//     }
};

class ChildInvokeNone : public BaseWithNoDefaults, 
                        public BaseWithDefaults
{
 public:
  // Should add : BaseWithNoDefaults(), BaseWithDefaults() to 
  // constructor initializer list.
  ChildInvokeNone() { }

  // Actually, I don't think we should add anything here.
  ChildInvokeNone &operator=(ChildInvokeNone &rhs) { 
      // Should add:
      //     *((BaseWithNoDefaults *)this) = (BaseWithDefaults &)rhs;
      //     *((BaseWithDefaults *)this) = (BaseWithNoDefaults &)rhs;
  }
// Should create the following:
//     ChildInvokeNone(ChildInvokeNone &rhs) 
//         : BaseWithNoDefaults(rhs), BaseWithDefaults(rhs) { }
//     ~ChildInvokeNone() { 
//         ((BaseWithNoDefaults *)this)->~BaseWithNoDefaults();
//         ((BaseWithDefaults *)this)->~BaseWithDefaults();
//     }
};

class ChildInvokeOne : public BaseWithNoDefaults, 
                        public BaseWithDefaults
{
 public:
  // Should add : BaseWithDefaults() to 
  // constructor initializer list.
  ChildInvokeOne() : BaseWithNoDefaults() { }

  // Should add : BaseWithDefaults(cio) to 
  // constructor initializer list.
  ChildInvokeOne(ChildInvokeOne &cio) : BaseWithNoDefaults(cio) { }

// Should create the following:
//     ~ChildInvokeOne() { 
//         ((BaseWithNoDefaults *)this)->~BaseWithNoDefaults();
//         ((BaseWithDefaults *)this)->~BaseWithDefaults();
//     }
//     ChildInvokeOne &operator=(ChildInvokeOne &rhs) {
//         *((BaseWithNoDefaults *)this) = (BaseWithDefaults &)rhs;
//         *((BaseWithDefaults *)this) = (BaseWithNoDefaults &)rhs;
//         if (this == &rhs) {
//             return *this;
//         } else {
//         }
//         return *this;
//     }

};

class ChildInvokeBoth : public BaseWithNoDefaults,
                        public BaseWithDefaults
{
 public:
  // Should not add anything here.
  ChildInvokeBoth() : BaseWithNoDefaults(), BaseWithDefaults() { }

  // Should not add anything here.
  ChildInvokeBoth &operator=(ChildInvokeBoth &rhs) {
    (*((BaseWithDefaults *)this))= (BaseWithDefaults&)rhs;
    (*((BaseWithNoDefaults *)this))= (BaseWithNoDefaults&)rhs;
  }

// Should create the following:
//     ChildInvokeBoth(ChildInvokeBoth &rhs) 
//         : BaseWithNoDefaults(rhs), BaseWithDefaults(rhs) { }
//     ~ChildInvokeBoth() { 
//         ((BaseWithNoDefaults *)this)->~BaseWithNoDefaults();
//         ((BaseWithDefaults *)this)->~BaseWithDefaults();
//     }
};

int main()
{
  return 0;
}
