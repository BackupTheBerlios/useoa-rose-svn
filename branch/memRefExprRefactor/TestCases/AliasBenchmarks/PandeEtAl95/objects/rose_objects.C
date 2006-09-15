// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//  Written by Ashok Sreenivasan, TRDDC, Pune, India.  1993.  May be
//  distributed freely, provided this comment is displayed at the top.
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
#include "onlyobj.h"

extern "C" { int strlen(char *str1)
{
  return 0;
}

 }

extern "C" { int strcmp(char *str1,char *str2)
{
  return 0;
}

 }

extern "C" { char *strcpy(char *str1,char *str2)
{
  return "";
}

 }

extern "C" { char *strncpy(char *str1,char *str2,int i)
{
  return "";
}

 }

extern "C" { char *strcat(char *str1,char *str2)
{
  return "";
}

 }
#include "artest.h"
#define abs(x)		((x) > 0 ? (x) : (-x))
// The search function for a table searches for the element linearly and
// returns the index of the element in the list.  If not found, a -1 is
// returned.  The comparison of objects is done through the '==' operator.

int Table::Search(class Object &obj)
{
  for (int i = 0; i < ((this) -> nelem); i++) {
    if ((obj== *(this) -> Get(i))) {
      return i;
    }
    else {
    }
  }
  return (-1);
}

// The table == operator is used to check for equality of tables themselves!
// This operator checks first if the types of the two tables are the same,
// then if the no. of elements are the same, and finally if each and every
// element is the same.  Only then are the tables deemed equal.  Note that
// if the tables are large, this could be an expensive operation.

int Table::operator==(class Object &tblobj)
{
  char *a;
// Type check
  if ((strcmp((a = tblobj.Type()),(this) -> Type()))) {
    return 0;
  }
  else {
  }
  class Table *tblptr = (class Table *)(&tblobj);
// Count check
  if (tblptr -> Nelem() == ((this) -> nelem)) {
    class Object *b;
    for (int i = 0; i < ((this) -> nelem); i++) {
// Elementwise check
      if ((( *(b = tblptr -> Get(i)))!= *(this) -> Get(i))) {
        return 0;
      }
      else {
      }
    }
  }
  else {
    return 0;
  }
  return 1;
}

// The array constructor allocates the reqd space and initialises the elems
// explicitly to NULL.

Array::Array(int sz,enum ::TblType t) : Table(t)
{
  (this) -> array = (new Objp [sz]);
  (this) -> size = sz;
// Explicitly NULLify
  for (int i = 0; i < sz; i++) {
    ((this) -> array)[i] = ((0));
  }
}

// The array destructor - deletes the elements from the rightmost to leftmost
// first.  Once the elements are rid, the array pointer itself is deleted.

Array::~Array()
{
  ((class Table *)(this)) -> ~Table();
  if ((((this) -> type)) == (VOLAT)) {
    for (int i = (((this) -> nelem) - 1); i >= 0; i--) {
      (this) -> Remove(i);
    }
  }
  else {
  }
  if (((this) -> array)) {
    delete ((this) -> array);
    (this) -> array = ((0));
  }
  else {
  }
  (this) -> Init();
}

// The array append function increments nelem and sets curind and adds the
// passed element at the end.  Appending to an empty array is an error.

int Array::Append(class Object *o)
{
  if (((this) -> nelem) == ((this) -> size)) {
    return 0;
  }
  else {
  }
  ((this) -> array)[(this) -> curind = ((this) -> nelem)] = o;
  (this) -> nelem++;
  return 1;
}

// The Array insert function inserts the object at the given position and
// moves the objects to the right by one position as required.  Cannot insert
// far to the right of rightmost element.  Returns a boolean success / fail
// value.  Insertion also fails when the array is full.

int Array::Insert(class Object *obj,int pos)
{
  bool rose_sc_bool_0 = false;
  if ((pos > ((this) -> nelem))) {{{
        rose_sc_bool_0 = true;
      }
    }
  }
  else {
    if ((pos < 0)) {{
        rose_sc_bool_0 = true;
      }
    }
    else {
      if ((((this) -> nelem) >= ((this) -> size))) {
        rose_sc_bool_0 = true;
      }
      else {
      }
    }
  }
  if (rose_sc_bool_0) {
    return 0;
  }
  else {
  }
  for (int i = ((this) -> nelem); i > pos; i--) {
// Move objects right.
    ((this) -> array)[i] = (((this) -> array)[i - 1]);
  }
  ((this) -> array)[pos] = obj;
  (this) -> nelem++;
  (this) -> curind = pos;
  return 1;
}

// The Array assign function overwrites the object at the given position with
// the passed object if the passed position is valid.

int Array::Assign(class Object *obj,int pos)
{
  bool rose_sc_bool_1 = false;
  if ((pos > ((this) -> nelem))) {{{
        rose_sc_bool_1 = true;
      }
    }
  }
  else {
    if ((pos < 0)) {{
        rose_sc_bool_1 = true;
      }
    }
    else {
      if ((((this) -> nelem) >= ((this) -> size))) {
        rose_sc_bool_1 = true;
      }
      else {
      }
    }
  }
  if (rose_sc_bool_1) {
    return 0;
  }
  else {
  }
  if (pos == ((this) -> nelem)) {
// Adding new element
    (this) -> nelem++;
  }
  else {
  }
  ((this) -> array)[(this) -> curind = pos] = obj;
  return 1;
}

// The array get function just performs a simple array access on the array
// after validating bounds.

Object *Array::Get(int idx)
{
  bool rose_sc_bool_2 = false;
  if ((idx >= ((this) -> nelem))) {{
      rose_sc_bool_2 = true;
    }
  }
  else {
    if ((idx < 0)) {
      rose_sc_bool_2 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_2) {
    return (0);
  }
  else {
  }
  (this) -> curind = idx;
  return ((this) -> array)[idx];
}

// The Fetch function gets an object from a list and returns it to the caller
//	without deleting it.  It also takes care of shifting elements 
// appropriately etc.

Object *Array::Fetch(int idx)
{
  bool rose_sc_bool_3 = false;
  if ((idx >= ((this) -> nelem))) {{
      rose_sc_bool_3 = true;
    }
  }
  else {
    if ((idx < 0)) {
      rose_sc_bool_3 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_3) {
    return (0);
  }
  else {
  }
  class Object *ret = (this) -> Get(idx);
  for (int i = idx; i < (((this) -> nelem) - 1); i++) {
    ((this) -> array)[i] = (((this) -> array)[i + 1]);
  }
  ((this) -> array)[((this) -> nelem) - 1] = ((0));
// Right most element
  if (idx == (((this) -> nelem) - 1)) {
    (this) -> curind = (idx - 1);
  }
  else {
    (this) -> curind = idx;
  }
  (this) -> nelem--;
  return ret;
}

// The assign function is the only way to add / overwrite elements in a 
// sparse array.  It just overwrites the element in the position with the new
// object, while deleting the old one if one existed.

int SpArray::Assign(class Object *o,int pos)
{
  bool rose_sc_bool_4 = false;
  if ((pos >= ((this) -> size))) {{
      rose_sc_bool_4 = true;
    }
  }
  else {
    if ((pos < 0)) {
      rose_sc_bool_4 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_4) {
    return 0;
  }
  else {
  }
  if ((((this) -> array)[pos]) == ((0))) {
// increase element count
    (this) -> nelem++;
  }
  else {
  }
  ((this) -> array)[(this) -> curind = pos] = o;
  return 1;
}

// The Get function gets the required array element and returns it.

Object *SpArray::Get(int idx)
{
  bool rose_sc_bool_5 = false;
  if ((idx >= ((this) -> size))) {{
      rose_sc_bool_5 = true;
    }
  }
  else {
    if ((idx < 0)) {
      rose_sc_bool_5 = true;
    }
    else {
    }
  }
  if (rose_sc_bool_5) {
    return (0);
  }
  else {
  }
  (this) -> curind = idx;
  return ((this) -> array)[idx];
}

// The Fetch function just gets an array element and returns it, and forgets
// it from the sparse array.

Object *SpArray::Fetch(int pos)
{
  class Object *ret = (this) -> Get(pos);
// Location was occupied
  if (ret) {
    ((this) -> array)[pos] = ((0));
    if (pos == (((this) -> nelem) - 1)) {
      (this) -> curind = (pos - 1);
    }
    else {
      (this) -> curind = pos;
    }
    (this) -> nelem--;
  }
  else {
  }
  return ret;
}

// Search all the elements in the array - as nelem is no limiting factor !

int SpArray::Search(class Object &o)
{
  for (int i = 0; i < ((this) -> size); i++) {
    class Object *p = (((this) -> array)[i]);
// Location is occupied
    if (p != ((0))) {
      if (((*p)==o)) {
        return i;
      }
      else {
      }
    }
    else {
    }
  }
  return (-1);
}

// The sparse array destructor - deletes the elements from the rightmost to
// leftmost first.  Once the elements are rid, the array pointer itself is
// deleted.

SpArray::~SpArray()
{
  ((class Array *)(this)) -> ~Array();
  if ((((this) -> type)) == (VOLAT)) {
    for (int i = (((this) -> size) - 1); i >= 0; i--) {
      if ((((this) -> array)[i]) != ((0))) {
        (this) -> Remove(i);
      }
      else {
      }
    }
  }
  else {
  }
  if (((this) -> array)) {
    delete ((this) -> array);
    (this) -> array = ((0));
  }
  else {
  }
  (this) -> Init();
}


class A : public Object
{
  

  public: virtual inline int operator==(class Object &o)
{
    char *a;
    if ((strcmp((a = o.Type()),(this) -> Type()))) {
      return 0;
    }
    else {
    }
    return (((this) -> i) == (((class A &)o).i));
  }

  

  virtual inline char *Type()
{
    return "A";
  }

  

  inline A(int j=0) : i(j), Object()
{
  }

  

  inline operator int()
{
    return (this) -> i;
  }

  

  virtual inline ~A()
{
    ((class Object *)(this)) -> ~Object();
  }

  protected: int i;
  

  public: A(class A &rhs) : i(rhs.i), Object(rhs)
{
  }

  

  A &operator=(class A &rhs)
{
    (*((class Object *)(this)))=((class Object &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> i = rhs.i;
    return  *(this);
  }

}

;

class B : public A
{
  

  public: virtual inline int operator==(class Object &o)
{
    char *a;
    if ((strcmp((a = o.Type()),(this) -> Type()))) {
      return 0;
    }
    else {
    }
    return (((this) -> j) == (((class B &)o).j));
  }

  

  virtual inline char *Type()
{
    return "B";
  }

  

  inline B(int j=0) : A()
{
  }

  

  inline operator int()
{
    return (this) -> j;
  }

  

  virtual inline ~B()
{
    ((class A *)(this)) -> ~A();
  }

  protected: int j;
  

  public: B(class B &rhs) : j(rhs.j), A(rhs)
{
  }

  

  B &operator=(class B &rhs)
{
    (*((class A *)(this)))=((class A &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> j = rhs.j;
    return  *(this);
  }

}

;

int main()
{
  class A *a1 = new A (1);
  class A *a2 = new A (2);
  class A *a3;
  class Array *ar = new Array (3,VOLAT);
  class SpArray *sar = new SpArray (3,VOLAT);
  class Array *a;
  if ((0)) {
    a = ar;
  }
  else {
    a = (sar);
  }
  a -> Assign((a1));
  a -> Assign((a2),2);
  a3 = ((class A *)(a -> Fetch(2)));
  a3 -> Type();
}

