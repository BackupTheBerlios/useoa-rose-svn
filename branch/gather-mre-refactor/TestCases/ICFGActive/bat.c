    #include <stdio.h>
    #include <math.h>

    void head(double x[4], double y[4]) { 
	  double t1,t2,t3;


#pragma $adic_indep x
	  t1=x[1]/x[2];
	  t2=x[3]*x[4];
      t3=t1*t2;
	  y[1]=t1*t3;
	  y[2]=1./t3;
	  y[3]=sin(t3);
	  y[4]=t3*t2;
#pragma $adic_dep y

    }

    int main() {
        double a[4], b[4];
        head(a,b);
        return 0;
    }
