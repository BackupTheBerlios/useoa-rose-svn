
    #include <stdio.h>

	void head(double x[10], double y[1]) { 
          int i;


#pragma $adic_indep x

          for(i=1; i<10; i++) {
            if (i==1) {
              y[1]=x[1];
            }  
            else {
              y[1]=y[1]*x[i];
            }
          }


#pragma $adic_dep y

     }

     int main() {
         double a[10], b[1];
 
         head(a,b);
          
         return 0;
     }
