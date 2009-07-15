
	void foo(double x, double y) {
	  y=x*2;
      }  

	void head(double x[2], double y) { 
	  double p[2],q[2];
	  int k,l;

#pragma $adic_indep x
	  k=1;
	  foo(x[k],y);
	  q[1]=y;
 	  y=q[1];
	  p[1]=1.0;
	  l=1;
	  foo(p[k],q[l]);

#pragma $adic_dep y

     }

     int main() {
         double a[2], b;
         head(a,b);
         return 0;
     }
