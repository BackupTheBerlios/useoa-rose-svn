#include<stdio.h>

int main()
{
   int a,b,c;
#pragma $adic_indep b
#pragma $adic_indep c

     a = 10;
 
     a = ( (a<=0) ? b : c );

#pragma $adic_dep a

}
