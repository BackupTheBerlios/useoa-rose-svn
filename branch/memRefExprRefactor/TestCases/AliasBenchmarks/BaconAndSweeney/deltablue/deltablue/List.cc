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

#include <stdio.h>
#include "List.h"

#ifdef NOTHING

/****** Create and Destruction ******/

List<Element>::List(int initialCount) {
  slots = new Element[initialCount];
  slotCount = initialCount;
  first = 0;
  last = -1;
}

  List<Element>::~List() {
  if (slots == NULL) Error("bad ListStruct; already freed?");
  delete [] slots;
  slots = NULL;
  slotCount = 0;
  first = 0;
  last = -1;
}

/****** Enumeration and Queries ******/

  void List<Element>::Do(Proc proc) {
  for (int i = first; i <= last; i++) {
    (*proc)(slots[i]);
  }
}

  int List<Element>::size() {
  return (last - first) + 1;
}

/****** Adding ******/

  void List<Element>::add(Element element) {
  if (last >= (slotCount - 1)) makeRoom();
  slots[++last] = element;
}

  void List<Element>::append(List<Element>* list2) {
  for (int i = list2->first; i <= list2->last; i++) {
    add(slots[i]);
  }
}

/****** Removing ******/

  void List<Element>::remove(Element element) {
  register Element *srcPtr = &slots[first];
  register Element *destPtr = &slots[0];
  register Element *lastPtr = &slots[last];
  
  last = last - first;
  first = 0;
  while (srcPtr <= lastPtr) {
    if (*srcPtr == element) {
      last--;
    } else {
      *destPtr++ = *srcPtr;
    }
    srcPtr++;
  }
}

  Element List<Element>::removeFirst() {
  register Element element;
  
  if (last < first) return NULL;
  element = slots[first++];
  return element;
}

  void List<Element>::removeAll() {
  first = 0;
  last = -1;
}

/****** Private ******/

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

  void List<Element>::grow() {
  int n = slotCount;
  slotCount += min(max(slotCount, 2), 512);
  Element* newSlots = new Element[slotCount];
  for (int i = 0; i < n; i++) newSlots[i] = slots[i];
  delete [] slots;
  slots = newSlots;
}

  void List<Element>::makeRoom() {
  register Element *srcPtr = &slots[first];
  register Element *destPtr = &slots[0];
  register Element *lastPtr = &slots[last];
  
  if (((last - first) + 1) >= slotCount) grow();
  if (first == 0) return;
  while (srcPtr <= lastPtr) {
    *destPtr++ = *srcPtr++;
  }
  last = last - first;
  first = 0;
}

#endif

// ============



/****** Create and Destruction ******/

typedef Constraint *PtrConstraint;

ListPtrConstraint::ListPtrConstraint(int initialCount) {
  slots = new PtrConstraint[initialCount];
  slotCount = initialCount;
  first = 0;
  last = -1;
}

ListPtrConstraint::~ListPtrConstraint() {
  if (slots == NULL) Error("bad ListStruct; already freed?");
  delete [] slots;
  slots = NULL;
  slotCount = 0;
  first = 0;
  last = -1;
}

/****** Enumeration and Queries ******/

void ListPtrConstraint::Do(Proc proc) {
  for (int i = first; i <= last; i++) {
    (*proc)(slots[i]);
  }
}

int ListPtrConstraint::size() {
  return (last - first) + 1;
}

/****** Adding ******/

void ListPtrConstraint::add(PtrConstraint element) {
  if (last >= (slotCount - 1)) makeRoom();
  slots[++last] = element;
}

void ListPtrConstraint::append(ListPtrConstraint *list2) {
  for (int i = list2->first; i <= list2->last; i++) {
    add(slots[i]);
  }
}

/****** Removing ******/

void ListPtrConstraint::remove(PtrConstraint element) {
  register PtrConstraint *srcPtr = &slots[first];
  register PtrConstraint *destPtr = &slots[0];
  register PtrConstraint *lastPtr = &slots[last];
  
  last = last - first;
  first = 0;
  while (srcPtr <= lastPtr) {
    if (*srcPtr == element) {
      last--;
    } else {
      *destPtr++ = *srcPtr;
    }
    srcPtr++;
  }
}

Constraint *ListPtrConstraint::removeFirst() {
  register PtrConstraint element;
  
  if (last < first) return NULL;
  element = slots[first++];
  return element;
}

void ListPtrConstraint::removeAll() {
  first = 0;
  last = -1;
}

/****** Private ******/

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

void ListPtrConstraint::grow() {
  int n = slotCount;
  slotCount += min(max(slotCount, 2), 512);
  PtrConstraint* newSlots = new PtrConstraint[slotCount];
  for (int i = 0; i < n; i++) newSlots[i] = slots[i];
  delete [] slots;
  slots = newSlots;
}

void ListPtrConstraint::makeRoom() {
  register PtrConstraint *srcPtr = &slots[first];
  register PtrConstraint *destPtr = &slots[0];
  register PtrConstraint *lastPtr = &slots[last];
  
  if (((last - first) + 1) >= slotCount) grow();
  if (first == 0) return;
  while (srcPtr <= lastPtr) {
    *destPtr++ = *srcPtr++;
  }
  last = last - first;
  first = 0;
}

// ======================================================

/****** Create and Destruction ******/

typedef Variable *PtrVariable;

ListPtrVariable::ListPtrVariable(int initialCount) {
  slots = new PtrVariable[initialCount];
  slotCount = initialCount;
  first = 0;
  last = -1;
}

ListPtrVariable::~ListPtrVariable() {
  if (slots == NULL) Error("bad ListStruct; already freed?");
  delete [] slots;
  slots = NULL;
  slotCount = 0;
  first = 0;
  last = -1;
}

/****** Enumeration and Queries ******/

void ListPtrVariable::Do(Proc proc) {
  for (int i = first; i <= last; i++) {
    (*proc)(slots[i]);
  }
}

int ListPtrVariable::size() {
  return (last - first) + 1;
}

/****** Adding ******/

void ListPtrVariable::add(PtrVariable element) {
  if (last >= (slotCount - 1)) makeRoom();
  slots[++last] = element;
}

void ListPtrVariable::append(ListPtrVariable *list2) {
  for (int i = list2->first; i <= list2->last; i++) {
    add(slots[i]);
  }
}

/****** Removing ******/

void ListPtrVariable::remove(PtrVariable element) {
  register PtrVariable *srcPtr = &slots[first];
  register PtrVariable *destPtr = &slots[0];
  register PtrVariable *lastPtr = &slots[last];
  
  last = last - first;
  first = 0;
  while (srcPtr <= lastPtr) {
    if (*srcPtr == element) {
      last--;
    } else {
      *destPtr++ = *srcPtr;
    }
    srcPtr++;
  }
}

Variable *ListPtrVariable::removeFirst() {
  register PtrVariable element;
  
  if (last < first) return NULL;
  element = slots[first++];
  return element;
}

void ListPtrVariable::removeAll() {
  first = 0;
  last = -1;
}

/****** Private ******/

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

void ListPtrVariable::grow() {
  int n = slotCount;
  slotCount += min(max(slotCount, 2), 512);
  PtrVariable* newSlots = new PtrVariable[slotCount];
  for (int i = 0; i < n; i++) newSlots[i] = slots[i];
  delete [] slots;
  slots = newSlots;
}

void ListPtrVariable::makeRoom() {
  register PtrVariable *srcPtr = &slots[first];
  register PtrVariable *destPtr = &slots[0];
  register PtrVariable *lastPtr = &slots[last];
  
  if (((last - first) + 1) >= slotCount) grow();
  if (first == 0) return;
  while (srcPtr <= lastPtr) {
    *destPtr++ = *srcPtr++;
  }
  last = last - first;
  first = 0;
}


/*
struct Constraint;
struct Variable;

typedef List<Constraint*> CList;
typedef List<Variable*> VList;
*/
