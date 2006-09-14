/***************************************************************************
 Constraints.h

    Constraint, variable, and strength data definitions for DeltaBlue.

****************************************************************************/
# ifndef _CONSTRAINTS_H
# define _CONSTRAINTS_H


# include "top.h"

/* Strength Constants */
#define S_required		6
#define S_strongPreferred	5
#define S_preferred		4
#define S_strongDefault		3
#define S_default		2
#define S_weakDefault		1
#define S_weakest		0

/* Other Constants and Macros */
#define NO_METHOD	(-1)
#define Weaker(a,b)	(a < b)

struct Variable;

class Constraint {
 protected:
  Boolean	_inputFlag;
  char		_strength;
  char		_whichMethod;
  char		_methodCount;
  char		_varCount;
  Variable**	_methodOuts;
  Variable**    _variables;
 public:
  Constraint(int, int);
  virtual ~Constraint();
  VIRTUAL void destroy();
  VIRTUAL void print();
  VIRTUAL Boolean inputsKnown();
  virtual void execute() = 0;
  VIRTUAL Constraint* satisfy(int mark);
  VIRTUAL void addConstraint();
  VIRTUAL int		chooseMethod(int mark);
  VIRTUAL int		outputWalkStrength();
  VIRTUAL Boolean	isConstantOutput();

  VIRTUAL Boolean	isInput() { return _inputFlag; }
  VIRTUAL char		strength() { return _strength; }
  VIRTUAL char		whichMethod() { return _whichMethod; }
  VIRTUAL void		set_whichMethod(char w) { _whichMethod = w; }
  VIRTUAL char		methodCount() { return _methodCount; }
  VIRTUAL char		varCount() { return _varCount; }
  VIRTUAL Variable*	methodOuts(int n) { return _methodOuts[n]; }
  VIRTUAL Variable*	variables(int n) { return _variables[n]; }
  VIRTUAL Boolean isSatisfied() { return whichMethod() != NO_METHOD; }
  VIRTUAL Variable* outVar();
  
};

class Variable {
 protected:
  long		_value;
  ListPtrConstraint* _constraints;
  Constraint*	_determinedBy;
  long		_mark;
  char		_walkStrength;
  Boolean	_stay;
  char		_name[10];
 public:
  Variable(char *, long, Boolean constant);
  VIRTUAL ~Variable();
  VIRTUAL void attachConstraint(Constraint* c);
  VIRTUAL void detachConstraint(Constraint* c);
  VIRTUAL void	print();
  
  VIRTUAL long		value() { return _value; }
  VIRTUAL void		set_value(long v) { _value = v; }
  VIRTUAL ListPtrConstraint* constraints() { return _constraints; }
  VIRTUAL Constraint*	determinedBy() { return _determinedBy; }
  VIRTUAL void		set_determinedBy(Constraint* c) { _determinedBy = c; }
  VIRTUAL long		mark() { return _mark; }
  VIRTUAL void		set_mark(long m) { _mark = m; }
  VIRTUAL char		walkStrength() { return _walkStrength; }
  VIRTUAL void		set_walkStrength(char s) { _walkStrength = s; }
  VIRTUAL Boolean	stay() { return _stay; }
  VIRTUAL void		set_stay(Boolean s) { _stay = s; }
  VIRTUAL char*		name() { return _name; }
};

/* Miscellaneous */
  void		ExecutePlan(ListPtrConstraint*);
  char* 	StrengthString(int);

# endif
