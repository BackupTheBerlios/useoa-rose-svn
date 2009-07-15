	void foo(double x, double y) { 
	  y=x*x;
    }

	void head(double x[2], double y) { 
	  double px[2];
	  double py;

#pragma $adic_indep x

	   foo(x[1],y);
	   foo(px[1],py);


#pragma $adic_dep y

     }


     int main() {
         double a[2], b;
         head(a,b);
         return 0;
     }
