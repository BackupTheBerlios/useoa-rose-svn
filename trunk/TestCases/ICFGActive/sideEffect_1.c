
    #include <stdio.h>

    double aGlobal;


	void foo(double x[1], double y) 
    {
	  y=x[1]*x[1];
    }       

	void bar(double x) { 
	  x=x*aGlobal;
    }

	void head(double x[1], double y[1]) {


#pragma $adic_indep x

	  aGlobal=2.0;
	  bar(x[1]);
	  foo(x,aGlobal);
      y[1]=aGlobal;



#pragma $adic_dep y

    }



    int main() {

        double a[1], b[1];
        head(a,b);
        return 0;
    }
