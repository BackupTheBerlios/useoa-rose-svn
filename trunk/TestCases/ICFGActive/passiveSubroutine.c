      
      #include <stdio.h>
      #include <math.h>

      void foo(double a, double b);
      void bar(double a, double b);

      void head(double x[1], double y[1]) {
           double p,q;

#pragma $adic_indep x

           foo(x[1],y[1]);
           p=1.0;
           bar(p,q);

#pragma $adic_dep y

      }

      void foo(double a, double b) {
           b=a*a;
      }

      void bar(double a, double b) {
           b=cos(a);
      }

      
      int main() {
          double a[1], b[1];
          head(a,b); 
          return 0;
      }
