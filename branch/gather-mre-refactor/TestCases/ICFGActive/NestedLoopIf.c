
    #include <stdio.h>

	void head(double x[2], double y[2]) {
          int i;

#pragma $adic_indep x

          for(i=1; i<2; i++) {
            if (i==1) {
              y[i]=x[i]*x[i] ;
            }
            else {
              y[i]=x[i]+x[i];
            }
            x[i]=0.0;
          }

#pragma $adic_dep y

     }

     int main() {

         double a[2], b[2];

         head(a,b);

         return 0;

     }
