/*
  func_call.c

  Test command: 
    OATest --oa-MemRefExpr TestCases/MemRefExprTests/func_call.c

  Goal: Testing function calls that return addresses and calls through
        function pointers.

  Output: see TestResults/MemRefExpr, not yet
*/

#include <stdlib.h> // for malloc
 
int (*fp)(); 
int* (*fp2)(); 
int bar(); 
int *hello(); 

int bar() 
{ 
  int x = 0;
  int *p;

  *(hello()) = x;
  // < use, F, UnknownRef, 0, T, partial >
  // < def, F, UnknownRef, 1, F, partial >

  fp = bar;
  // < use, T, SymHandle(bar), 0, T, full >
  // < def, T, SymHandle(fp), 0, F, full >

  fp();
  // < use, T, SymHandle(fp), 1, F, full >
  // < use, T, SymHandle(fp), 0, F, full > // ??
 
  bar();

  fp2 = hello;

  p = fp2();

  p = (int *)malloc(sizeof(int));

  return x;
  // < use, T, SymHandle(x), O, F, full >

} 
 

