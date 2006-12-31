
      #include <stdio.h>
      #include <math.h>

 void foo(double a, double b, double c);
 void bar(double a);

      void head(double x[4], double y[4]) {

#pragma $adic_indep x

         foo(x[1],x[2],y[1]);
         y[2]=sin(x[1]*x[2]);
         y[3]=sin(x[3]);
         y[4]=cos(x[4]);

#pragma $adic_dep y

       }

      void foo(double a, double b, double c) { 
      
         bar(b);
         c=a*a+b;

      } 

      void bar(double a) {

         a=cos(a*a);
      
      }


      int main() {

          double a[4], b[4];
          head(a,b);
          return 0;
      }
