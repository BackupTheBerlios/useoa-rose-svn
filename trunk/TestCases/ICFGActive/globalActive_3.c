
     
     #include <stdio.h>
     #include <math.h>

	 double aGlobal;

	 void foo(double x[2], double y) 
     {
	  y=x[1]*x[2];
      }

	 void head(double x[2], double y[1]) 
     {

#pragma $adic_indep x

	  foo(x,aGlobal);
      y[1]=aGlobal;

#pragma $adic_indep y 

     }

     int main()
     {
     
        double a[2], b[1];
        head(a,b);
        return 0;  
     }
