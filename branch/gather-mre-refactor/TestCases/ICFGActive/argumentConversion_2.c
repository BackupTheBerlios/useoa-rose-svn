
    void foo(double x, double y)  {
	  y=x;
      }

	void head(double x, double y) { 
	  double py;

#pragma $adic_indep x
	  foo(x,y);
	  foo(2.0,py);

#pragma $adic_dep y

    }

   
    int main() {
        double a,b;
        head(a,b);
        return 0;
    }

