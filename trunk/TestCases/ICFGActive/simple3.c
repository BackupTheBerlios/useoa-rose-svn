    #include <stdio.h>
    #include <math.h>

    void head(double x1, double x2,double y1,double y2)  {
	  double  v3,v4;


#pragma $adic_indep x1
#pragma $adic_indep x2
 
	  v3=x1*x2;
	  v4=x1*v3;
      y1=v3*x2*v4;
      y2=sin(v4);


#pragma $adic_dep y1 
#pragma $adic_dep y2

    }

    int main() {

        double a,b,c,d;
        head(a,b,c,d);
        return 0;
    }
