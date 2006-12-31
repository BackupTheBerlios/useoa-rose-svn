
    #include <stdio.h>

    void head(double x[1], double y[1])
    { 
          int i,j,k;

#pragma $adic_indep x

          for (i=1; i<2; i++) { 
            y[1]=x[1]; 
            if (y[1]>0) {
              y[1]=x[1] ;
              for( j=1; j < 3; j++ ) {
                y[1]=x[1]; 
                for (k=1; k < 3; k++) {
                  y[1]=x[1];
                  if (x[1]>0) {
                    y[1]=y[1]*x[1];
                  }
                }
              }
            }
            else
            {
              y[1]=y[1]/x[1];
            }
          }

#pragma $adic_dep y

      }

      
      int main() {
          double a[1], b[1];
          head(a,b);  
          return 0;
      }
