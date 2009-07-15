
    #include <stdio.h>
    #include <string.h>
    #include <math.h>

	void head(double x[1], double y[1]) {
          int i;
	      char aString[10] = "blah";

#pragma $adic_indep x

          y[1]=x[1];

          for(i=1; i<3; i++) {
            if (strcmp(aString,"blah") == 0) {
              y[1]=y[1]*x[1];
            }
            if (strcmp(aString ,"bloh") == 0) {
              y[1]=y[1]-x[1];
            }
          }
          y[1]=y[1];

#pragma $adic_dep y
    
     }

     int main() {

         double a[1], b[1];          

         head(a,b);           
 
         return 0;
     }
