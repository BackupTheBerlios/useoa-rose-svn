
    #include <stdio.h>
    #include <math.h>

	void foo() {
        int a;
        a = 3;
    }

	void head(double x[2], double y)
    {
	  double t, t1, t2;

#pragma $adic_indep x

	  t=x[1] + x[2];
	  t2=t*2;
	  foo();
	  y=2*t+3*t2;

#pragma $adic_indep y

    }

    int main()
    {
        double a[2], b;
        head(a,b);
        return 0;
    }

