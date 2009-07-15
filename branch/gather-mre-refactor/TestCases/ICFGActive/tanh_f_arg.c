
    #include <stdio.h>
    #include <math.h>

	void head(double x[1], double y[1]) { 


#pragma $adic_indep x

          y[1]=tanh(x[1]);


#pragma $adic_dep y

     }

     int main() {

         double a[1], b[1];
         head(a,b);
         return 0;
     }
