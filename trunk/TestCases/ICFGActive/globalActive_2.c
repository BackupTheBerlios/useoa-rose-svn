
      #include <stdio.h>
      #include <math.h>

	  double aGlobal;
	  double aPassiveGlobal;

	 void head(double x[2], double y[1])
     {

#pragma $adic_indep x

	  aPassiveGlobal=2.0;
	  aGlobal=x[1]*x[2];
      y[1]=aGlobal*aPassiveGlobal;

#pragma $adic_dep y 

     }

     int main()
     {
         double a[2], b[1];
         head(a,b);
         return 0;
     }

