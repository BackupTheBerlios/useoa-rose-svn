	double gx;
	double gy;

	void bar(double barx, double bary) {
     	double t;
	    t=barx;
     	barx=bary;
     	bary=t;
    }

	void foo( ) {
	bar(gx,gy);
    }
	
	head(double x[2], double y[1]) { 

#pragma $adic_indep x
	gx=x[1];
	gy=gx;
	y[1]=gy;
#pragma $adic_dep y

    }

    int main() {

        double a[2], b[1];
        head(a,b);
        return 0;
    }
