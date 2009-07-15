
    #include <stdio.h>
    

	void head(double x[2], double y[2]) {

#pragma $adic_indep x

          if (x[1]<=x[2]) {
            y[1]=x[2]-x[1];
          }
          else {
            y[1]=x[1]-x[2];
          }

          if (y[1]==0) {
            y[2]=x[1];
          } 
          else {
            y[2]=y[1];
          }

#pragma $adic_dep y

    }

    int main() {

        double a[2], b[2];

        head(a,b);

        return 0;
 
    }
