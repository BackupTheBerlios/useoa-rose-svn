/***************************************************************************
 DeltaBlue.h

    DeltaBlue, an incremental dataflow constraint solver.

****************************************************************************/


struct DeltaBlue {
  
  DeltaBlue();
  VIRTUAL void	AddVariable(Variable*);
  VIRTUAL void	AddConstraint(Constraint*);
  VIRTUAL void	DestroyConstraint(Constraint*);
  ListPtrConstraint *	ExtractPlan(void);
  ListPtrConstraint *	ExtractPlanFromConstraint(Constraint*);
  ListPtrConstraint *	ExtractPlanFromConstraints(ListPtrConstraint*);
  VIRTUAL long mark() { return currentMark; }
  
  VIRTUAL Boolean	AddPropagate(Constraint*);
 protected:
  ListPtrVariable * allVariables;
  static ListPtrConstraint * hot;	/* used to collect "hot" constraints */
  static long currentMark;

  static ListPtrConstraint * unsatisfied;
  	/* used to collect unsatisfied downstream constraints */
  static int strength;	    /* used to add unsatisfied constraints in strength order */
  
  static void  AddIfSatisfiedInput(Constraint*);
  static void  CollectSatisfiedInputs(Variable*);
  VIRTUAL ListPtrConstraint * MakePlan(void);
  VIRTUAL void		IncrementalAdd(Constraint*);
  static void AddAtStrength(Constraint*);
  VIRTUAL void		IncrementalRemove(Constraint*);
  static void	CollectUnsatisfied(Constraint*);
  VIRTUAL void		RemovePropagateFrom(Variable*);
  VIRTUAL void		Recalculate(Constraint*);
  VIRTUAL void		NewMark(void);

  static Constraint* NDC_determiningC;
  static ListPtrConstraint * NDC_todo;
  static void   NDC_helper(Constraint* c);
  VIRTUAL Constraint*	NextDownstreamConstraint(ListPtrConstraint *, Variable*);
};

extern DeltaBlue * deltablue;
