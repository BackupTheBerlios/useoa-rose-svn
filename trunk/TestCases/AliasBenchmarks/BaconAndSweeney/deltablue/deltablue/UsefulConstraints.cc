/***************************************************************************
 UsefulConstraints.c

    Some useful constraints.

****************************************************************************/
 
#include <stdio.h>
#include "UsefulConstraints.h"
#include "DeltaBlue.h"

/* macro to reference a constraint variable value */
#define var(i)  (variables(i)->value())
#define setvar(i,v) (variables(i)->set_value(v))

/******* Stay Constraint *******/

StayConstraint::StayConstraint(Variable* v, int s)
: Constraint(1, s) {
  _variables[0] = v;
  _methodCount = 1;
  _methodOuts[0] = _variables[0];
  deltablue->AddConstraint(this);
};

void StayConstraint::execute() {
  if (whichMethod() == NO_METHOD)
    Error("Implementation error: Constraint is not enforced");
}

/******* Edit Constraint *******/

EditConstraint::EditConstraint(Variable* v, int s)
: Constraint(1, s) {
  _inputFlag = true;
  _variables[0] = v;
  _methodCount = 1;
  _methodOuts[0] = _variables[0];
  deltablue->AddConstraint(this);
};

void EditConstraint::execute() {
  if (whichMethod() == NO_METHOD)
    Error("Implementation error: Constraint is not enforced");
}

/****** Equals Constraint ******/

void EqualsConstraint::execute() {
  /* a = b */
  switch (whichMethod()) {
   case 0:
    setvar(0, var(1));
    break;
   case 1:
    setvar(1, var(0));
    break;
  }
}

EqualsConstraint::EqualsConstraint(Variable* a, Variable* b, int s)
: Constraint(2, s) {
  _variables[0] = a;
  _variables[1] = b;
  _methodCount = 2;
  _methodOuts[0] = _variables[0];
  _methodOuts[1] = _variables[1];
  deltablue->AddConstraint(this);
};

/******** Plus Constraint *******/

void PlusConstraint::execute() {
  /* a + b = sum */
  switch (whichMethod()) {
   case 0:
    setvar(2, var(0) + var(1));
    break;
   case 1:
    setvar(1, var(2) - var(0));
    break;
   case 2:
    setvar(0, var(2) - var(1));
    break;
  }
}

PlusConstraint::PlusConstraint(Variable* a, Variable* b, Variable* sum,
			     int s) : Constraint(3, s) {
  _variables[0] = a;
  _variables[1] = b;
  _variables[2] = sum;
  _methodCount = 3;
  _methodOuts[0] = _variables[0];
  _methodOuts[1] = _variables[1];
  _methodOuts[2] = _variables[2];
  deltablue->AddConstraint(this);
};

/******** Multiply Constraint *******/

void MultiplyConstraint::execute() {
  /* a * b = prod */
  switch (whichMethod()) {
   case 0:
    setvar(2, var(0) * var(1));
    break;
   case 1:
    setvar(1, var(2) / var(0));
    break;
   case 2:
    setvar(0, var(2) / var(1));
    break;
  }
}

MultiplyConstraint::MultiplyConstraint(Variable* a, Variable* b,
				       Variable* prod, int s)
: Constraint(3, s) {
  _variables[0] = a;
  _variables[1] = b;
  _variables[2] = prod;
  _methodCount = 3;
  _methodOuts[0] = _variables[0];
  _methodOuts[1] = _variables[1];
  _methodOuts[2] = _variables[2];
  deltablue->AddConstraint(this);
};
