
    #include <stdio.h>
    
	void foo(double x[2], double y[2], int k) {
          int i,j;
	      j=k;
          for( i=1; i<3; i++) {
            y[j]=y[j]+x[i]*x[i];
          }
     }


	void head(double x[2], double y[2]) {

#pragma $adic_indep x

	  y[1]=1.0;
	  y[2]=1.0;
      foo(x,y,1);
	  foo(x,y,2);


#pragma $adic_dep y

    }

    int main() {
        double a[2], b[2];
        head(a,b);
        return 0; 
    }
