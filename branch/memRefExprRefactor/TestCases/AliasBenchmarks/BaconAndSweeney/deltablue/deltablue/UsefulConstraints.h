/***************************************************************************
 UsefulConstraints.h

    Some useful constraints. 

****************************************************************************/

#include "top.h"
#include "List.h"
#include "Constraints.h"

class StayConstraint: public Constraint {
 public:
  StayConstraint(Variable* v, int);			/* keeps v constant */
  void execute();
};

class EditConstraint: public Constraint {
 public:
  EditConstraint(Variable* v, int);				/* change v */
  void execute();
};

class EqualsConstraint: public Constraint {
 public:
  EqualsConstraint(Variable* a, Variable* b, int);		/* a = b */
  void execute();
};

class PlusConstraint: public Constraint {
 public:
  PlusConstraint(Variable* a, Variable* b, Variable* sum, int);
  		/* a + b = sum */
  void execute();
};

class MultiplyConstraint: public Constraint {
 public:
  MultiplyConstraint(Variable* a, Variable* b, Variable* prod, int);
  		/* a * b = prod */
  void execute();
};

