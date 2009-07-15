
/* Purpose:  Test ROSE AST normalization.  In particular, test
 *           the insertion of implicit invocations of base 
 *           class special methods within a derived class.
 */

// All special methods are defined.
class Base {
public:
  Base() { }
  Base(Base &rhs) { }
  Base &operator=(Base &rhs) {
    if ( this == &rhs ) {
      return *this;
    } else {
    }
    return *this;
  }
  ~Base() { }
};


// All special methods are defined.
class DefinedChild : public Base {
public:
  // Should add an invocation to Base() within DefinedChild()'s 
  // constructor initializer list.
  DefinedChild() { }

  // Should add an invocation to Base(rhs) within constructor's
  // initializer list.
  DefinedChild(DefinedChild &rhs) { }

  // Should add _nothing_.
  DefinedChild &operator=(DefinedChild &rhs) {
    if ( this == &rhs ) {
      return *this;
    } else {
    }
    return *this;
  }

  // Should add an invocation to ~Base().
  ~DefinedChild() { }
};


// No special methods are defined.
class UndefinedChild : public Base {
public:
  // Should create the following:
  //     UndefinedChild() : Base() { }
  //     UndefinedChild(UndefinedChild &rhs) : Base(rhs) { }
  //     UndefinedChild &operator=(UndefinedChild &rhs) {
  //         *((Base *)(this)) = (Base &)(rhs);
  //         if ( this == &rhs ) {
  //             return *this;
  //         } else {
  //         }
  //         return *this;
  //     }
  //     ~UndefinedChild() { ((Base*)this)->~Base(); }
};
