/***************************************************************************
 List.c

    Implementation of List.h.
    Invariants and relationships:
	slots != NULL
	slotCount > 0
	sizeof(*slots) == slotCount * sizeof(Element)
	0 <= first < slotCount
	-1 <= last < slotCount
	last >= first (if not empty)
	last == first - 1 (if empty)
	NumberOfItems == (last - first) + 1

****************************************************************************/
#ifdef NOTHING
#include <stdio.h>

# pragma implementation "List.h"

#include "List.cc"
struct Constraint;
struct Variable;

typedef List<Constraint*> CList;
typedef List<Variable*> VList;

#endif
