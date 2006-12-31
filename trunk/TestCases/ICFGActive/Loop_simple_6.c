
    #include <stdio.h> 

	void head(double x[3], double y[3]) {
          int i,j,k;


#pragma $adic_indep x

	  if (x[1] < 4.0) {
	     for(i=1; i<3; i++) {
		     y[i]=x[i] ;
	     }
      }
	  else {
	     for(i=1; i<3; i++) {
    		y[i]=2.0*x[i];
	     }
	  }


#pragma $adic_dep y

     }


     int main() {

         double a[3], b[3];

         head(a,b);

         return 0;
     }
