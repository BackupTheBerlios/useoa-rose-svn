/*
  MemRefExprTestInput.C

  Test command: 
    OATest --oa-MemRefExpr TestCases/MemRefExprTests/MemRefExprTestInput.C

  Goal: Testing all the memory reference expression examples that are 
        provided on the wiki as of November 2005.

  Output: see TestResults/MemRefExpr
*/

#include <stdlib.h> // for malloc
 
struct anotherStruct {
  int d;
};

struct myStruct {
  int b;
  int *bPtr;
  struct anotherStruct *a;
};

class hiClass {
 public:
  int *hello();
  int bar();
};

struct myStruct *gp; 
struct myStruct gy; 

int (*fp)(); 
int bar(); 
int *hello(); 
int foo(struct myStruct **); 
int foobar(struct myStruct *); 

int bar() 
{ 

  int x,y; 
  struct myStruct *p; 
  int **q; 
  int *r; 
  struct myStruct sta[40][10][30]; 
  int a, b, c;
  hiClass *hi;

  struct myStruct *ignoreStructPtr;
  struct anotherStruct *ignoreAnotherStructPtr;
  struct anotherStruct **ignoreAnotherStructPtrPtr;
  struct myStruct  ignoreStruct;
  int              ignoreInt;
  int             *ignoreIntPtr;
  struct myStruct (*ignoreArrayPtr)[30];

  // < MemRefType, Named?, SymHandle, NumDerefs, AddrOf?, Accuracy >

  p = ignoreStructPtr;     
  // < def, T, SymHandle(p), O, F, full >

  p;
  // < use, T, SymHandle(p), O, F, full >

  ignoreStructPtr = p;
  // < use, T, SymHandle(p), O, F, full >

  *(sta[3][4][2].bPtr) = ignoreInt;
  // < use, T, SymHandle(ignoreInt), 0, F, full >, 
  // < use, T, SymHandle(sta), 0, F, partial >, 
  // < def, T, SymHandle(sta), 1, F, partial >  

  ignoreIntPtr = &x;
  // < use, T, SymHandle(x), 0, T, full >
  // < def, T, SymHandle(ingoreIntPtr), 0, F, full >
 
  ignoreStructPtr = (struct myStruct *)malloc(sizeof(struct myStruct));
  // < use, F, StmtHandle(thisStmt), O, T, partial >

  foobar(p);
  // < use, T, SymHandle(p), 0, F, full >

  foo(&p);
  // < use, T, SymHandle(p), 0, T, full >

  *(hello()) = ignoreInt;
  // < use, F, UnknownRef, 0, T, partial >
  // < def, F, UnknownRef, 1, F, partial >

  *(hi->hello()) = ignoreInt;
  // < use, T, SymHandle(hi), 0, F, full >      // hi
  // < use, T, SymHandle(hi), 1, F, partial >   // hi->hello
  // < use, F, UnknownRef, 0, T, partial >      // hi->hello()
  // < def, F, UnknownRef, 1, F, partial >      // *(hi->hello())

  **q = ignoreInt;
  // < use, T, SymHandle(q), 0, F, full >
  // < use, T, SymHandle(q), 1, F, full >
  // < def, T, SymHandle(q), 2, F, full >

  gp->a->d = ignoreInt;
  // < use, T, SymHandle(gp), 0, F, full >
  // < use, T, SymHandle(gp), 1, F, partial >
  // < def, T, SymHandle(gp), 2, F, partial >

  gy.b = ignoreInt;
  // < use, T, SymHandle(m), 0, F, partial >

  ignoreAnotherStructPtr = (*p).a;
  // < use, T, SymHandle(p), 0, F, full >
  // < use, T, SymHandle(p), 1, F, partial >

  p->a = ignoreAnotherStructPtr;
  // < use, T, SymHandle(p), 0, F, full >
  // < def, T, SymHandle(p), 1, F, partial >

  ignoreAnotherStructPtrPtr = &(p->a);
  // < use, T, SymHandle(p), 0, F, full >
  // < use, T, SymHandle(p), 1, T, partial >

  ignoreArrayPtr = &(sta[3][4]);
  // < use, T, SymHandle(sta), 0, T, partial >

  *( r < *q ? r : *q ) = ignoreInt;
  // < use, T, SymHandle(r), 0, F, full >
  // < use, T, SymHandle(q), 0, F, full >
  // < use, T, SymHandle(q), 1, F, full >
  // < def, T, SymHandle(r), 1, F, full >
  // < def, T, SymHandle(q), 2, F, full >

  *( r + (int)*q ) = ignoreInt;
  // < use, T, SymHandle(r), 0, F, full >
  // < use, T, SymHandle(q), 0, F, full >
  // < use, T, SymHandle(q), 1, F, full >
  // < use, F, UnknownRef, 0, T, part > 
  // < def, F, UnknownRef, 1, F, part > 

  fp = bar;
  // < use, T, SymHandle(bar), 0, T, full >
  // < def, T, SymHandle(fp), 0, F, full >

  fp();
  // < use, T, SymHandle(fp), 1, F, full >
  // < use, T, SymHandle(fp), 0, F, full > // ??

  a = b = c;
  // < use, T, SymHandle(c), 0, F, full >
  // < defuse, T, SymHandle(b), 0, F, full >
  // < def, T, SymHandle(a), 0, F, full >

  int i = 3, j = 4;
  // < def, T, SymHandle(i), 0, F, full >
  // < def, T, SymHandle(j), 0, F, full >

  int d, e, f, g;

  for(int d = 0; e < 3; ++f) {
    g = g + 1;
  }

  while(d < 3) {
    g = g + 1;
  }

  if (d < 3) {
    g = g + 1;
  } else {
    f++;
  }

  // Reference examples.  Note that we model references as pointers.
  int A;
  int *ptr = &A;
  // < use, T, SymHandle(A), 0, T, full >
  // < def, T, SymHandle(ptr), O, F, full >
  // NamedRef(USE, SymHandle(A), T, full)
  // NamedRef(DEF, SymHandle(ptr))

  int &B = A;
  // -> int *B = &A;
  // < use, T, SymHandle(A), 0, T, full >
  // < def, T, SymHandle(B), O, F, full >
  // NamedRef(USE, SymHandle(A), T, full)
  // NamedRef(DEF, SymHandle(B))

  int &C = B;
  // -> int *C = &*B;
  // -> int *C = B;
  // < use, T, SymHandle(B), 0, F, full >
  // < def, T, SymHandle(C), O, F, full >
  // NamedRef(USE, SymHandle(B))
  // NamedRef(DEF, SymHandle(C))

  //  int *aPtr = &A;

  int *&F = ptr;
  // -> int **F = &ptr;
  // < use, T, SymHandle(ptr), 0, T, full >
  // < def, T, SymHandle(F), O, F, full >
  // NamedRef(USE, SymHandle(ptr), T, full)
  // NamedRef(DEF, SymHandle(F))

  int *&G = F;
  // -> int **G = &*F;
  // -> int **G = F;
  // < use, T, SymHandle(F), 0, F, full >
  // < def, T, SymHandle(G), O, F, full >
  // NamedRef(USE, SymHandle(F))
  // NamedRef(DEF, SymHandle(G))

  int *const &D = &A;
  // -> int **D;  *D = &A;
  // < use, T, SymHandle(A), 0, T, full >
  // < def, T, SymHandle(D), 1, F, full >
  // NamedRef(USE, SymHandle(A), T, full)
  // Deref(DEF, NamedRef(USE, SymHandle(D)), 1, F, full)

  int *const &E = &B;
  // -> int **E; *E = (*(&B));
  // -> int **E; *E = B;
  // < use, T, SymHandle(B), 0, F, full >
  // < def, T, SymHandle(E), 1, F, full >
  //  NamedRef(USE, SymHandle(B))
  //  Deref(DEF, NamedRef(USE, SymHandle(E)), 1, F, full)

  int *const *ptr3 = &D;
  // -> int **ptr3 = *(&D);
  // -> int **ptr3 = D;
  // < use, T, SymHandle(D), 0, F, full >
  // < def, T, SymHandle(ptr3), 0, F, full >
  //  NamedRef(USE, SymHandle(D))
  //  NamedRef(DEF, SymHandle(ptr3))

  return x;
  // < use, T, SymHandle(x), O, F, full >

} 
 

