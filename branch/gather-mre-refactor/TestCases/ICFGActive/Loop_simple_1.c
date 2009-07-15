

     #include <stdio.h>

	 void head(double x[1], double y[1]) {
          int i;


#pragma $adic_indep x

          y[1]=x[1];

          for (i=1; i<3; i++) {
            y[1]=y[1]*x[1];
          }
          y[1]=y[1] ;


#pragma $adic_dep y

    }


    int main() {
 
         double a[1], b[1];

         head(a,b); 

         return 0;
     }
