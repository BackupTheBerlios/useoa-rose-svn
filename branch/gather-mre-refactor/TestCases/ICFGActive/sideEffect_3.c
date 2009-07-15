
    #include <stdio.h>

	void bar(double c, double d) { 
	  d=c*c;
    }  

	void foo(double a, double b) { 
	  bar(a,b);
    }

	void head(double x[1], double y[1]) { 

#pragma $adic_indep x

	  foo(x[1],y[1]);


#pragma $adic_dep y

     }

     int main() {
         double a[1], b[1];
         head(a,b); 
         return 0;
     }
