
    #include <stdio.h>

	void head(double x[3], double y[3]) {
        int i,j,k;


#pragma $adic_indep x

          for (i=1; i<3; i++)  {
            y[i]=x[i]; 
          } 
          for (i=1; i<3; i++) {
            y[i]=x[i];
            for( j=1; j<3; j++) {
              y[i]=x[i];
              for( k=1; k<3; k++) {
                y[i]=y[i]*x[j];
              }
            }
          }

#pragma $adic_dep y

    }

    int main() {

        double a[3], b[3];
        head(a,b);
        return 0;
    }
