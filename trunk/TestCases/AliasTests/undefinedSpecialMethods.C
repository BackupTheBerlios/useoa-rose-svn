
/** Purpose:  Test ROSE normalization. In particular, test the
 *            creation of special method declarations and definitions.
 */

// Should insert following special methods:
//   DefaultCtorOnly(DefaultCtorOnly &rhs) { }
//   ~DefaultCtorOnly() { }
//   DefaultCtorOnly &operator=(DefaultCtorOnly &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     return *(this);
//   }
class DefaultCtorOnly {
 public:
  DefaultCtorOnly() { }
};

// Should insert following special methods:
//   AllButDefaultCtor() { }
class AllButDefaultCtor {
 public:
  AllButDefaultCtor(AllButDefaultCtor &other) { }
  ~AllButDefaultCtor() { }
  AllButDefaultCtor &operator=(AllButDefaultCtor &rhs) { 
      if ( (this) == &rhs ) {
          return *(this);
      } else {
      }
      return *(this);
  }
};

// Now, repeat the above, but include a member variable that
// will need to be copied on assignment and initialization
// Should insert following special methods:
//   DefaultCtorOnlyWithMemberVar(DefaultCtorOnlyWithMemberVar &rhs) : mX(rhs.mX) { }
//   ~DefaultCtorOnlyWithMemberVar() { }
//   DefaultCtorOnlyWithMemberVar &operator=(DefaultCtorOnlyWithMemberVar &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     (this) -> mX = rhs.mX;
//     return *(this);
//   }
class DefaultCtorOnlyWithMemberVar {
 public:
  DefaultCtorOnlyWithMemberVar() { }
 private:
  int mX;
};

// Should insert following special methods:
//   AllButDefaultCtorWithMemberVar() { }
class AllButDefaultCtorWithMemberVar {
 public:
  AllButDefaultCtorWithMemberVar(AllButDefaultCtorWithMemberVar &rhs) : mX(rhs.mX) { }
  ~AllButDefaultCtorWithMemberVar() { }
  AllButDefaultCtorWithMemberVar &operator=(AllButDefaultCtorWithMemberVar &rhs) {
      if ( (this) == &rhs ) {
          return *(this);
      } else {
      }
      (this) -> mX = rhs.mX;
      return *(this);
  }
 private:
  int mX;
};

// Repeat the above for a copy constructor

// Should insert following special methods:
//   CopyCtorOnly() { }
//   ~CopyCtorOnly() { }
//   CopyCtorOnly &operator=(CopyCtorOnly &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     return *(this);
//   }
class CopyCtorOnly {
 public:
  CopyCtorOnly() { }
};

// Should insert following special methods:
//   AllButCopyCtor(AllButCopyCtor &rhs) { }
class AllButCopyCtor {
 public:
  AllButCopyCtor() { }
  ~AllButCopyCtor() { }
  AllButCopyCtor &operator=(AllButCopyCtor &rhs) { 
      if ( (this) == &rhs ) {
          return *(this);
      } else {
      }
      return *(this);
  }
};

// Now, repeat the above, but include a member variable that
// will need to be copied on assignment and initialization
// Should insert following special methods:
//   CopyCtorOnlyWithMemberVar() { }
//   ~CopyCtorOnlyWithMemberVar() { }
//   CopyCtorOnlyWithMemberVar &operator=(CopyCtorOnlyWithMemberVar &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     (this) -> mX = rhs.mX;
//     return *(this);
//   }
class CopyCtorOnlyWithMemberVar {
 public:
  CopyCtorOnlyWithMemberVar(CopyCtorOnlyWithMemberVar &rhs) : mX(rhs.mX) { }
 private:
  int mX;
};

// Should insert following special methods:
//   AllButCopyCtorWithMemberVar(AllButCopyCtorWithMemberVar &rhs) : mX(rhs.mX) { }
class AllButCopyCtorWithMemberVar {
 public:
  AllButCopyCtorWithMemberVar() { }
  ~AllButCopyCtorWithMemberVar() { }
  AllButCopyCtorWithMemberVar &operator=(AllButCopyCtorWithMemberVar &rhs) {
      if ( (this) == &rhs ) {
          return *(this);
      } else {
      }
      (this) -> mX = rhs.mX;
      return *(this);
  }
 private:
  int mX;
};

// Repeat the above for a destructor

// Should insert following special methods:
//   DestructorOnly() { }
//   DestructorOnly(DestructorOnly &rhs) { }
//   DestructorOnly &operator=(DestructorOnly &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     return *(this);
//   }
class DestructorOnly {
 public:
  ~DestructorOnly() { }
};

// Should insert following special methods:
//   ~AllButDestructor() { }
class AllButDestructor {
 public:
  AllButDestructor() { }
  AllButDestructor(AllButDestructor &rhs) { }
  AllButDestructor &operator=(AllButDestructor &rhs) { 
      if ( (this) == &rhs ) {
          return *(this);
      } else {
      }
      return *(this);
  }
};

// Now, repeat the above, but include a member variable that
// will need to be copied on assignment and initialization
// Should insert following special methods:
//   DestructorOnlyWithMemberVar() { }
//   DestructorOnlyWithMemberVar(DestructorOnlyWithMemberVar &rhs) : mX(rhs.mX) { }
//   DestructorOnlyWithMemberVar &operator=(DestructorOnlyWithMemberVar &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     (this) -> mX = rhs.mX;
//     return *(this);
//   }
class DestructorOnlyWithMemberVar {
 public:
  ~DestructorOnlyWithMemberVar() { }
 private:
  int mX;
};

// Should insert following special methods:
//   ~AllButDestructorWithMemberVar() { }
class AllButDestructorWithMemberVar {
 public:
  AllButDestructorWithMemberVar() { }
  AllButDestructorWithMemberVar(AllButDestructorWithMemberVar &rhs) : mX(rhs.mX) { }
  AllButDestructorWithMemberVar &operator=(AllButDestructorWithMemberVar &rhs) {
      if ( (this) == &rhs ) {
          return *(this);
      } else {
      }
      (this) -> mX = rhs.mX;
      return *(this);
  }
 private:
  int mX;
};

// Repeat the above for operator=

// Should insert following special methods:
//   AssignmentOperatorOnly() { }
//   AssignmentOperatorOnly(AssignmentOperatorOnly &rhs) { }
//   ~AssignmentOperatorOnly() { }
class AssignmentOperatorOnly {
 public:
   AssignmentOperatorOnly &operator=(AssignmentOperatorOnly &rhs) { 
     if ((this) == &rhs) {
       return *(this);
     } else {
     }
     return *(this);
   }
};

// Should insert following special methods:
//   AllButAssignmentOperator &operator=(AllButAssignmentOperator &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     return *(this);
//   }
class AllButAssignmentOperator {
 public:
  AllButAssignmentOperator() { }
  AllButAssignmentOperator(AllButAssignmentOperator &rhs) { }
  ~AllButAssignmentOperator() { }
};

// Now, repeat the above, but include a member variable that
// will need to be copied on assignment and initialization
// Should insert following special methods:
//   AssignmentOperatorOnlyWithMemberVar() { }
//   AssignmentOperatorOnlyWithMemberVar(AssignmentOperatorOnlyWithMemberVar &rhs) : mX(rhs.mX) { }
//   ~AssignmentOperatorOnlyWithMemberVar() { }
//   }
class AssignmentOperatorOnlyWithMemberVar {
 public:
   AssignmentOperatorOnlyWithMemberVar &operator=(AssignmentOperatorOnlyWithMemberVar &rhs) { 
     if ((this) == &rhs) {
       return *(this);
     } else {
     }
     (this) -> mX = rhs.mX;
     return *(this);
   }
 private:
  int mX;
};

// Should insert following special methods:
//   AssignmentOperatorOnlyWithMemberVar &operator=(AssignmentOperatorOnlyWithMemberVar &rhs) { 
//     if ((this) == &rhs) {
//       return *(this);
//     } else {
//     }
//     (this) -> mX = rhs.mX;
//     return *(this);
//   }
class AllButAssignmentOperatorWithMemberVar {
 public:
  AllButAssignmentOperatorWithMemberVar() { }
  AllButAssignmentOperatorWithMemberVar(AllButAssignmentOperatorWithMemberVar &rhs) : mX(rhs.mX) { }
  ~AllButAssignmentOperatorWithMemberVar() { }
 private:
  int mX;
};

// But do _not_ create a default constructor if the user has defined
// a constructor.
// Should insert following special methods:
//     UserDefinedConstructor(UserDefinedConstructor &rhs) { }
//     UserDefinedConstructor &operator=(UserDefinedConstructor &rhs) { 
//         if ((this) == &rhs) {
//             return *(this);
//         } else {
//         }
//         (this) -> mX = rhs.mX;
//         return *(this);
//     }
//     ~UserDefinedConstructor() { }
class UserDefinedConstructor {
public:
    UserDefinedConstructor(int someVar) { }
}


int main()
{
  return 0;
}
