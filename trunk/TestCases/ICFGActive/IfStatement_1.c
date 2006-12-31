
    #include <stdio.h>
    #include <math.h>

	void head(double x[1], double y[1]) {

#pragma $adic_indep x

          y[1]=2.0; 
          if (x[1]>0) {
            y[1]=x[1];
          }
          y[1]=y[1]*y[1];


#pragma $adic_dep y

     }


     int main() {

        double a[1], b[1];
        head(a,b);  
        return 0;

     }
