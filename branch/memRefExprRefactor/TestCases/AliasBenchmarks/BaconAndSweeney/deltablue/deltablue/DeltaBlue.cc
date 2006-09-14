/******************************************************************************
 DeltaBlue.c

    DeltaBlue incremental constraint solver.

*******************************************************************************/

#include <stdio.h>
#include "List.h"
#include "Constraints.h"
#include "DeltaBlue.h"

/******** Public: Initialization *******/

DeltaBlue* deltablue = NULL;
ListPtrConstraint *DeltaBlue::hot;
long DeltaBlue::currentMark;
ListPtrConstraint *DeltaBlue::unsatisfied;
int DeltaBlue::strength;	
Constraint* DeltaBlue::NDC_determiningC;
ListPtrConstraint *DeltaBlue::NDC_todo;

DeltaBlue::DeltaBlue() {
  allVariables = new ListPtrVariable(128);
  currentMark = 0;
}

/******** Public: Variables and Constraints *******/

void DeltaBlue::AddVariable(Variable* v) { allVariables->add(v); }

void DeltaBlue::AddConstraint(Constraint* c) {
  register int i;
  
  c->addConstraint();
  IncrementalAdd(c);
}

void DeltaBlue::DestroyConstraint(Constraint* c) {
  register int i;

  if (c->isSatisfied()) IncrementalRemove(c);
  c->destroy();
}

/******** Public: Plan Extraction *******/

void DeltaBlue::AddIfSatisfiedInput(Constraint* c) {
  if (c->isInput() && c->isSatisfied()) {
    hot->add(c);
  }
}

void DeltaBlue::CollectSatisfiedInputs(Variable* v) {
  v->constraints()->Do((Proc)AddIfSatisfiedInput);
}

ListPtrConstraint * DeltaBlue::ExtractPlan() {
  if (hot == NULL) hot = new ListPtrConstraint(128);
  hot->removeAll();
  allVariables->Do((Proc)CollectSatisfiedInputs);
  return MakePlan();
}

ListPtrConstraint * DeltaBlue::ExtractPlanFromConstraint(Constraint* c) {
  if (hot == NULL) hot = new ListPtrConstraint (128);
  hot->removeAll();
  AddIfSatisfiedInput(c);
  return MakePlan();
}

ListPtrConstraint * DeltaBlue::ExtractPlanFromConstraints(ListPtrConstraint * constraints) {
  if (hot == NULL) hot = new ListPtrConstraint(128);
  hot->removeAll();
  constraints->Do((Proc)AddIfSatisfiedInput);
  return MakePlan();
}

/******* Private: Plan Extraction *******/

ListPtrConstraint * DeltaBlue::MakePlan() {
  ListPtrConstraint *	plan;
  Constraint*	nextC;
  Variable*	out;

  NewMark();
  plan = new ListPtrConstraint(128);
  nextC = hot->removeFirst();
  while (nextC != NULL) {
    out = nextC->outVar();
    if ((out->mark() != currentMark) && nextC->inputsKnown()) {
      plan->add(nextC);
      out->set_mark(currentMark);
      nextC = NextDownstreamConstraint(hot, out);
    } else {
      nextC = hot->removeFirst();
    }
  }
  return plan;
}

/******* Private: Adding *******/

void DeltaBlue::IncrementalAdd(Constraint* c) {
  register Constraint* overridden;
  INCR(nIncrementalAdd);

  NewMark();
  overridden = c->satisfy(mark());
  while (overridden != NULL) {
    overridden = overridden->satisfy(mark());
  }
}

/******* Private: Removing *******/

void DeltaBlue::AddAtStrength(Constraint* c) {
  if (c->strength() == strength) deltablue->IncrementalAdd(c);
}

void DeltaBlue::CollectUnsatisfied(Constraint* c) {
  if (!c->isSatisfied()) unsatisfied->add(c);
}

void DeltaBlue::IncrementalRemove(Constraint* c) {
  Variable* out;
  register int i;
  
  INCR(nIncrementalRemove);
  out = c->outVar();
  c->set_whichMethod(NO_METHOD);
  for (i = c->varCount() - 1; i >= 0; i--) {
    c->variables(i)->detachConstraint(c);
  }
  unsatisfied = new ListPtrConstraint(8);
  RemovePropagateFrom(out);
  for (strength = S_required; strength <= S_weakest; strength++) {
    unsatisfied->Do((Proc)AddAtStrength);
  }
  delete unsatisfied;
}

void DeltaBlue::RemovePropagateFrom(Variable* v) {
  Constraint*	nextC;
  ListPtrConstraint todo(8);

  v->set_determinedBy(NULL);
  v->set_walkStrength(S_weakest);
  v->set_stay(true);
  while (true) {
    v->constraints()->Do((Proc)CollectUnsatisfied);
    nextC = NextDownstreamConstraint(&todo, v);
    if (nextC == NULL) {
      break;
    } else {
      Recalculate(nextC);
      v = nextC->outVar();
    }
  }
}

/******* Private: Recalculation *******/

void DeltaBlue::Recalculate(Constraint* c) {
  Variable* out;
  
  out = c->outVar();
  out->set_walkStrength(c->outputWalkStrength());
  out->set_stay(c->isConstantOutput());
  if (out->stay()) c->execute();
}

Boolean DeltaBlue::AddPropagate(Constraint* c) {
  ListPtrConstraint todo(8);	/* unprocessed constraints */
  Constraint*	nextC;
  Variable*	out;

#ifdef COUNT
  INCR(nAddPropagate);
#endif
  nextC = c;
  while (nextC != NULL) {
    out = nextC->outVar();
    if (out->mark() == currentMark) {
      /* remove the cycle-causing constraint */
      IncrementalRemove(c);
      return false;
    }
    Recalculate(nextC);
    nextC = NextDownstreamConstraint(&todo, out);
  }
  return true;
}


/******* Private: Miscellaneous *******/

void DeltaBlue::NewMark(void) { currentMark++; }

void DeltaBlue::NDC_helper(Constraint* c) {
  nDownStream++;
  if (c != NDC_determiningC && c->isSatisfied()) {
      NDC_todo->add(c);
  }
}

Constraint* DeltaBlue::NextDownstreamConstraint(ListPtrConstraint * todo,
						Variable* variable){
  NDC_todo = todo;
  NDC_determiningC = variable->determinedBy();
  variable->constraints()->Do((Proc)NDC_helper);
  return NDC_todo->removeFirst();
}

extern "C" void exit(int);

void Error(char* errorString) {
  printf("List.c error: %s.\n", errorString);
  exit(-1);
}


