#include <stdio.h>

 int main()
{
   double x[2];
   double y[1];

#pragma $adic_indep x
         y[1]=x[1]*x[2];
#pragma $adic_dep y

  return 0;
}


