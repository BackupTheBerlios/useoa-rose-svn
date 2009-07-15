
    #include <stdio.h>

	void foo(double a, double b) { 
	  b=a*a;
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
