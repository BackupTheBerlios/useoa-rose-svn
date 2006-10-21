/***************************************************************************
 List.h

    Supports variable sized, ordered lists of elements.

****************************************************************************/

# ifndef _LIST_H
# define _LIST_H

//# pragma interface "List.h"

# include "top.h"

#include "Constraints.h"

class ListPtrConstraint {
    Constraint*	*slots;		/* variable-sized array of element slots */
    int		slotCount;	/* number of slots currently allocated */
    int		first;		/* index of first element */
    int		last;		/* index of last element (first-1, if empty) */

   public:
    /* Creation and Destruction */
    ListPtrConstraint(int);
    VIRTUAL ~ListPtrConstraint();
    
    /* Enumeration and Queries */
    VIRTUAL void Do(Proc);
    VIRTUAL int	 size();
    
    /* Adding */
    VIRTUAL void add(Constraint*);
    VIRTUAL void append(ListPtrConstraint*);
    
    /* Removing */
    VIRTUAL void remove(Constraint*);
    VIRTUAL void removeAll();
    VIRTUAL Constraint* removeFirst();

   protected:
    VIRTUAL void grow();
    VIRTUAL void makeRoom();
};


class ListPtrVariable {
    Variable*	*slots;		/* variable-sized array of element slots */
    int		slotCount;	/* number of slots currently allocated */
    int		first;		/* index of first element */
    int		last;		/* index of last element (first-1, if empty) */

   public:
    /* Creation and Destruction */
    ListPtrVariable(int);
    VIRTUAL ~ListPtrVariable();
    
    /* Enumeration and Queries */
    VIRTUAL void Do(Proc);
    VIRTUAL int	 size();
    
    /* Adding */
    VIRTUAL void add(Variable*);
    VIRTUAL void append(ListPtrVariable*);
    
    /* Removing */
    VIRTUAL void remove(Variable*);
    VIRTUAL void removeAll();
    VIRTUAL Variable* removeFirst();

   protected:
    VIRTUAL void grow();
    VIRTUAL void makeRoom();
};


# endif
