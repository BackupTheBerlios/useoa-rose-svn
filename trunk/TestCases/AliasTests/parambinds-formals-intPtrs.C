/*
  parambinds-formals-intPtrs.C

  Testing the AliasIRInterface implementations for C/C++ compilers.
  Specifically looking at possible actual parameters that could
  be involved in pointer parameter bindings and formal parameters.

  Notes:
  http://www.newty.de/fpt/fpt.html, notes on function pointers

  This is a subset of parambinds-formals.C used to debug
  the handling of varargs (a la Bug #7964).
*/

#include <stdarg.h>
//#include <stdio.h>
//#include <iostream>
using namespace std;

//void ellipsis_intptrs(int x, ...)
//{
//}

// going to pass in ptrs to ints
// This gets me SgVarArgOp and SgVarArgEndOp, why not SgVarArgStartOp?
void ellipsis_intptrs(int i, ...)
{
    int* j;
    va_list ap;
    //    va_start(ap,(const void *)i);
    va_start(ap,i);

    // g++ only accepts basic types or ptr types for the second param
    j = va_arg(ap,int*);
    //    printf("j=%d\n",j);

    va_end(ap);
}

int main()
{
    int x, y, z;

    // variable numbers of parameters
    ellipsis_intptrs(3, &x, &y, &z);

    return 0;
}


