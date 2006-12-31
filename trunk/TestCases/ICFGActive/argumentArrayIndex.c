	void foo(double x, double y)  {
	  y=x*2;
    }

	void head(double x[2], double y) {
	  int k;

#pragma $adic_indep x

	  k=1;
	  foo(x[k],x[k+1]);
	  foo(x[k],y);


#pragma $adic_dep y 
    }

    int main() {

        double a[2], b;
        head(a,b);
        return 0;
    }
