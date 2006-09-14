# ifndef _TOP_H
# define _TOP_H

class ListPtrConstraint;
class ListPtrVariable;

//  typedef enum {false, true};
  typedef int Boolean;
  typedef	void (*Proc)(...);
  
  void Error(char* string);

# define COUNT
# ifdef COUNT
  extern long nVarCreate, nConstrCreate,
              nSatisfy, nAddPropagate, nIncrementalAdd, nIncrementalRemove,
  	      nDownStream;
#   define INCR(var) var++
# else
#   define INCR(var)
# endif


//# define ALL_VIRTUALS

# ifdef ALL_VIRTUALS
#   define VIRTUAL  virtual
# else
#   define VIRTUAL
# endif

#endif  // _TOP_H
