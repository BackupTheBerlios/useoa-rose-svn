/***************************************************************************
 Constraints.c

    Constraint, variable, and other operations for DeltaBlue.

****************************************************************************/


#include <stdio.h>
#include <string.h>
#include "List.h"
#include "Constraints.h"
#include "DeltaBlue.h"

/******* Variables *******/

Variable::Variable(char *n, long initialValue, Boolean constant) {
  INCR(nVarCreate);
  _value = initialValue;
  _constraints = new ListPtrConstraint(constant ? 0 : 2);
  _determinedBy = NULL;
  _mark = 0;
  _walkStrength = constant ? S_required : S_weakest;
  _stay = true;
  strncpy(_name, n, 10);
  _name[9] = 0;
  deltablue->AddVariable(this);
}


Variable::~Variable() {
  if (_constraints == NULL) {
    Error("bad VariableStruct; already freed?");
  }
  delete _constraints;
  _constraints = NULL;
}

void Variable::print() {
  printf("%s(%s,%ld)", name(), StrengthString(walkStrength()), value());
}

void Variable::attachConstraint(Constraint* c) {
  constraints()->add(c);
}

void Variable::detachConstraint(Constraint* c) {
  constraints()->remove(c);
  if (determinedBy() == c) set_determinedBy(NULL);
}

/******* Constraints *******/

Constraint::Constraint(int variableCount, int s) {
  int i;
  INCR(nConstrCreate);
  _inputFlag = false;
  _strength = s;
  _whichMethod = NO_METHOD;
  _methodCount = 0;
  _varCount = variableCount;
  _variables = new Variable*[variableCount];
  _methodOuts = new Variable*[variableCount];
  for (i = 0; i < _varCount; i++) {
    _variables[i] = NULL;
    _methodOuts[i] = NULL;
  }
}

Constraint::~Constraint() {
  if (_variables == NULL) {
    Error("bad ConstraintStruct; already freed?");
  }
  delete [] _variables;
  delete [] _methodOuts;
  _variables = NULL;
}

int Constraint::outputWalkStrength() {
  int minStrength;

  minStrength = strength();
  Variable* out = outVar();
  for (int m = methodCount() - 1; m >= 0; m--) {
    Variable* mOutput = variables(m);
    if ((mOutput != out) &&
	(Weaker(mOutput->walkStrength(), minStrength))) {
      minStrength = mOutput->walkStrength();
    }
  }
  return minStrength;
}

#define INPUTS_DO(stmt)							      \
  Variable* out = outVar();						      \
  for (int i = varCount() - 1; i >= 0; i--) {				      \
    if (variables(i) != out) { stmt; }					      \
  }									      \


Boolean Constraint::isConstantOutput() {
  if (isInput()) return false;
  INPUTS_DO(
    if (!variables(i)->stay()) return false;
  )
  return true;
}

void Constraint::destroy() {
  for (int i = varCount() - 1; i >= 0; i--) {
    variables(i)->detachConstraint(this);
  }
  delete this;
}

void Constraint::addConstraint() {
  for (int i = varCount() - 1; i >= 0; i--) {
    variables(i)->attachConstraint(this);
  }
  set_whichMethod(NO_METHOD);
}

void Constraint::print() {
  int i, outIndex;
  
  if (!isSatisfied()) {
    printf("Unsatisfied(");
    for (i = 0; i < _varCount; i++) {
      _variables[i]->print();
      printf(" ");
    }
    printf(")");
  } else {
    printf("Satisfied(");
    INPUTS_DO(
      _variables[i]->print();
      printf(" ");
    )
    printf("-> ");
    variables(outIndex)->print();
    printf(")");
  }
  printf("\n");
}

Boolean Constraint::inputsKnown() {
  INPUTS_DO(
    Variable* in = variables(i);
    if (in->mark() != deltablue->mark() &&
	!in->stay() &&
	in->determinedBy() != NULL) {
      return false;
    }
  )
  return true;
}

Variable* Constraint::outVar() {
  return methodOuts(whichMethod());
}

Constraint* Constraint::satisfy(int mark) {
  int	i;
  Constraint*	overridden;
  Variable*	out;

  INCR(nSatisfy);
  set_whichMethod(chooseMethod(mark));
  if (isSatisfied()) {
    /* mark inputs to allow cycle detection in AddPropagate */
    INPUTS_DO(
      variables(i)->set_mark(mark);
    )
    out = outVar();
    overridden = out->determinedBy();
    if (overridden != NULL) overridden->set_whichMethod(NO_METHOD);
    out->set_determinedBy(this);
    if (!deltablue->AddPropagate(this)) {
      Error("Cycle encountered");
      return NULL;
    }
    out->set_mark(mark);
    return overridden;
  } else {
    if (strength() == S_required) {
      Error("Could not satisfy a required constraint");
    }
    return NULL;
  }
}

int Constraint::chooseMethod(int mark) {
  int	best, bestOutStrength, m;

  best = NO_METHOD;
  bestOutStrength = strength();
  for (m = methodCount() - 1; m >= 0; m--) {
    Variable* mOut = methodOuts(m);
    if ((mOut->mark() != mark) &&
	(Weaker(mOut->walkStrength(), bestOutStrength))) {
      best = m;
      bestOutStrength = mOut->walkStrength();
    }
  }
  return best;
}



/******* Miscellaneous Functions *******/

char* StrengthString(int strength) {
    static char temp[20];

    switch (strength) {
    case S_required:		return "required";
    case S_strongPreferred:	return "strongPreferred";
    case S_preferred:		return "preferred";
    case S_strongDefault:	return "strongDefault";
    case S_default:		return "default";
    case S_weakDefault:		return "weakDefault";
    case S_weakest:		return "weakest";
    default:
	sprintf(temp, "strength[%d]", strength);
	return temp;
    }
}

static void Execute(Constraint* c) { c->execute(); }

void ExecutePlan(ListPtrConstraint * list) {
  list->Do((Proc)Execute);
}
