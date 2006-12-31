
    #include <stdio.h>
    #include <math.h>

	void head(double x[2], double y[2])
    {
          int i;

#pragma $adic_indep x

          i=1;
          while (i<3) {
            if (i<2) {
              y[2]=sin(x[1]);
            }
            else
            {
              y[1]=cos(x[2]);
            }
            i=i+1;
          }
          y[2]=y[1]*y[2];

#pragma $adic_indep y

    }

    int main() {
        double a[2], b[2];
        head(a,b);
        return 0;
    }
