    
     #include <stdio.h>
     #include <math.h> 

     void foo(double a, double b, double c, double d);
 

     void head(double x[4], double y[4]) {
           double c,d;

#pragma $adic_indep x

           foo(x[1],x[2],c,d);
           y[3]=c*d;
           y[4]=c+d;

#pragma $adic_dep y

      }

      void foo(double a, double b, double c, double d)  {
            c=sin(a*b);
            d=cos(a+b); 
      }

      int main() {


          double a[4], b[4];
          head(a,b); 
          return 0;
      } 
