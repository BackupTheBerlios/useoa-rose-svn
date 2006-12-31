     #include <stdlib.h>
     #include <stdio.h>
     #include <math.h>

     void head(double x[2], double y[2]) {
          double t;
#pragma $adic_indep x

          t = x[1] * x[2];
          y[1] = sin(t);
          y[2] = cos(t);   


#pragma $adic_dep y 

     }

    
     int main() {
         double a[2], b[2];
         head(a,b);
         return 0;
     }
