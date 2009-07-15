 

    #include <stdio.h>
    #include <math.h>    

	void head(double x[2], double y[4])  {
	  double t1,t2,y1,y2,y3,y4;


#pragma $adic_indep x

	  t1=x[1]/x[2];
	  t2=cos(t1);
      y1=exp(t2);
      y2=3.14*t2;
	  y3=sin(t2);
      y4=t2*t1;
      y[1]=y1;
      y[2]=y2;
	  y[3]=y3;
      y[4]=y4;


#pragma $adic_dep y

     }

     int main() {
        double a[2], b[4];
        head(a,b); 
        return 0;
     }
