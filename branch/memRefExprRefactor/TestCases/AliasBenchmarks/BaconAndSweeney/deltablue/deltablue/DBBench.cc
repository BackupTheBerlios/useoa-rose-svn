#include <stdio.h>
#include <time.h>
#include "UsefulConstraints.h"
#include "DeltaBlue.h"

#ifdef COUNT
  long nVarCreate,nConstrCreate,nSatisfy,nAddPropagate,
       nIncrementalAdd,nIncrementalRemove,nDownStream;
#endif



/***************************************************************************

    Private Prototypes

****************************************************************************/

long Milliseconds(void);
void Start(void);
void Finish(long*);
void Assign(Variable*, long);
void Change(Variable*, long);
void Benchmark(int);
void ProjectionTest(int);
void TempertureConverter(void);
void TreeTest(int);
Variable* MakeTree(int);

/***************************************************************************

    Timing Functions

****************************************************************************/

static long startTime;

# define CLOCKS_PER_SEC  1000000  /* microseconds */

long Milliseconds()
{
    int millisecondsPerClock = CLOCKS_PER_SEC / 1000;
    return (clock() / millisecondsPerClock);
}

void Start()
{
    startTime = Milliseconds();
}

void Finish(long *milliseconds)
{
    *milliseconds = Milliseconds() - startTime;
}

/***************************************************************************
*
* This is the standard DeltaBlue benchmark. A long chain of equality
* constraints is constructed with a stay constraint on one end. An edit
* constraint is then added to the opposite end and the time is measured for
* adding and removing this constraint, and extracting and executing a
* constraint satisfaction plan. There are two cases. In case 1, the added
* constraint is stronger than the stay constraint and values must propagate
* down the entire length of the chain. In case 2, the added constraint is
* weaker than the stay constraint so it cannot be accomodated. The cost in
* this case is, of course, very low. Typical situations lie somewhere between
* these two extremes.
*
****************************************************************************/

void Benchmark(int n)
{
    long 	msecs, i;
    char	name[20];
    Variable*	prev, *v, *first, *last;
    Constraint*	editC;
    ListPtrConstraint *plan;

    deltablue = new DeltaBlue;
    prev = first = last = NULL;

  Start();
    for (i = 0; i < n; i++) {
	sprintf(name, "v%ld", i);
	v = new Variable(name, 0, false);
	if (prev != NULL) {
	    new EqualsConstraint(prev, v, S_required);
	}
	if (i == 0) first = v;
	if (i == (n-1)) last = v;
	prev = v;
    }
  Finish(&msecs);
    printf("\n%ld msecs to add %d constraints.\n", msecs, n);
    new StayConstraint(last, S_default);

    printf("Case 1:\n");
  Start();
    editC = new EditConstraint(first, S_strongDefault);
  Finish(&msecs);
    printf("  Add Constraint: %ld msecs.\n", msecs);

  Start();
    plan = deltablue->ExtractPlanFromConstraint(editC);
  Finish(&msecs);
    printf(
    	"  Make Plan: %ld msecs (plan is length %d).\n",
    	msecs, plan->size());

  Start();
    for (i = 0; i < 100; i++) {
	ExecutePlan(plan);
    }
  Finish(&msecs);
    printf("  Execute Plan: %.3f msecs.\n", msecs / 100.0);
    delete plan;

  Start();
    deltablue->DestroyConstraint(editC);
  Finish(&msecs);
    printf("  Remove Constraint: %ld msecs\n", msecs);

    printf("Case 2:\n");
  Start();
    editC = new EditConstraint(last, S_strongDefault);
  Finish(&msecs);
    printf("  Add Constraint: %ld msecs.\n", msecs);

  Start();
    plan = deltablue->ExtractPlanFromConstraint(editC);
  Finish(&msecs);
    printf(
	"  Make Plan: %ld msecs (plan is length %d).\n",
	msecs, plan->size());

  Start();
    for (i = 0; i < 100; i++) {
	ExecutePlan(plan);
    }
  Finish(&msecs);
    printf("  Execute Plan: %.3f msecs.\n", msecs / 100.0);
    delete plan;

  Start();
    deltablue->DestroyConstraint(editC);
  Finish(&msecs);
    printf("  Remove Constraint: %ld msecs\n", msecs);

}

/***************************************************************************
*
* This test constructs a two sets of variables related to each other by a
* simple linear transformation (scale and offset). The time is measured to
* change a variable on either side of the mapping and to change the scale or
* offset factors. It has been tested for up to 2000 variable pairs.
*
****************************************************************************/

void ProjectionTest(int n) {
  Variable*	src, *scale, *offset, *dest;
  long 	msecs, i;
  char	name[20];

  Start();
  scale = new Variable("scale", 10, false);
  offset = new Variable("offset", 1000, false);

  for (i = 1; i <= n; i++) {
    /* make src and dest variables */
    sprintf(name, "src%ld", i);
    src = new Variable(name, i, false);
    sprintf(name, "dest%ld", i);
    dest = new Variable(name, i, false);
    
    /* add stay on src */
    new StayConstraint(src, S_default);
    
    /* add scale/offset constraint */
    // new ScaleOffsetConstraint(src, scale, offset, dest, S_required);
  }
  Finish(&msecs);
  printf("\nSetup time for %d points: %ld msecs.\n", n, msecs);

  Change(scale, 2);
}

void Change(Variable* v, long newValue) {
  Constraint*	editC;
  long 	i, msecs;
  ListPtrConstraint *plan;

  printf("Changing %s...\n", v->name());
  Start();
  editC = new EditConstraint(v, S_strongDefault);
  Finish(&msecs);
  printf("  Adding Constraint: %ld msecs.\n", msecs);

  Start();
  plan = deltablue->ExtractPlanFromConstraint(editC);
  Finish(&msecs);
  printf("  Making Plan (length: %d): %ld msecs.\n", plan->size(), msecs);

  Start();
  v->set_value(newValue);
  for (i = 0; i < 100; i++) {
    ExecutePlan(plan);
  }
  Finish(&msecs);
  printf("  Executing Plan: %.3f msecs.\n", msecs / 100.0);
  delete plan;

  Start();
  deltablue->DestroyConstraint(editC);
  Finish(&msecs);
  printf("  Removing Constraint: %ld msecs\n", msecs);
}

#ifdef MACINTOSH
#include <console.h>
#endif

extern "C" void exit(int);

# ifdef DEBUG
  long nVarCreate = 0, nConstrCreate = 0, nSatisfy = 0;
# endif

main(int argc, char **argv) {
    int n;

#ifdef MACINTOSH
    argc = ccommand(&argv);
    printf("Macintosh Delta Blue Tests\n");
#else
    printf("Delta Blue Tests\n");
#endif
    if (argc < 2) {
	printf("usage: %s <count>\n", argv[0]);
	exit(-1);
    }
    sscanf(&*argv[1], "%d", &n);
    deltablue = new DeltaBlue;
    Benchmark(n);
    ProjectionTest(n);
#   ifdef DEBUG
      printf("%ld variables, %ld constraints, %ld satisfy calls\n",
	     nVarCreate, nConstrCreate, nSatisfy);
#   endif
    // vprof_output_statistics("out");
    return 0;
}
