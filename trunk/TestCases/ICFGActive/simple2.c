

   #include <stdio.h>
   #include <math.h>

   void head(double x1, double x2, double y1, double y2) {
	  double t1,t2;

#pragma $adic_indep x1
#pragma $adic_indep x2
	  t1=x1*x2;
	  t2=x1*sin(t1);
      y1=cos(t2);
      y2=t2*x2;

#pragma $adic_dep y1
#pragma $adic_dep y2
     }


     int main() {
         double a,b,c,d;
         head(a,b,c,d);
         return 0;
     }
