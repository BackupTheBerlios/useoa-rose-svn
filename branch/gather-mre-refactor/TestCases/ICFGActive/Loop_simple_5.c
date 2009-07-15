
    #include <stdio.h>

	void head(double x[3], double y[3]) {

      int i,j,k;

#pragma $adic_indep x
      for( i=1; i<3; i++) {
	    if (i == 2) { 
	       y[i]=x[i];
        } 
	    else {
	       y[i]=2*x[i] ;
	    }
      }

#pragma $adic_dep y

      }

     int main() {

        double a[3], b[3];

        head(a,b); 

        return 0;

     } 
