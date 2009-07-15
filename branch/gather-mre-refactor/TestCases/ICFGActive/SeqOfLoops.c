


    #include <stdio.h> 

	void head(double x[2], double y[2]) {
          int i;


#pragma $adic_indep x

          for(i=1; i<2; i++ ) {
            y[i]=x[i]  ;
          }
          for( i=1; i<2; i++ ) {  
            y[i]=y[i]*x[i];
          }

#pragma $adic_dep y

    }

    int main() {

        double a[2], b[2];

        head(a,b); 

        return 0;

    }
