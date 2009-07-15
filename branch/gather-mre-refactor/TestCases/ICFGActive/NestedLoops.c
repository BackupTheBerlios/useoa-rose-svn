

    #include <stdio.h>

	void head(double x[2], double y[2]) {
          int i,j;

#pragma $adic_indep x

        for(i=1; i<2; i++) {
	       for(j=1; j<2; j++) {
              y[i]=x[i]*x[j];
            }
         }

#pragma $adic_dep y

    }

    int main() {

        double a[2], b[2];

        head(a,b); 

        return 0;
     }
