	void head(double x[1], double y[1]) {
	  double t1,t2,y1,y2,y3,y4;


#pragma $adic_indep x
	  t1=x[1]+x[1];
	  t2=2*t1;
      y1=t2;
      y[1]=y1;
#pragma $adic_dep y

    }

    int main() {
        double a[1], b[2];
        head(a,b);
        return 0;
    }
