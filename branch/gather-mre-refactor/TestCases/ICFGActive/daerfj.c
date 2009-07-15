

      #include <stdio.h>
      #include <math.h>

      void head(double x[4], double fvec[11])
      {
      int i;
      double temp1, temp2;
      double v[11], y[11];


#pragma $adic_indep x

      v(1)=4.0d0;
      v(2)=2.0d0;
      v(3)=1.0d0;
      v(4)=5.0d-1;
      v(5)=2.5d-1;
      v(6)=1.67d-1;
      v(7)=1.25d-1;
      v(8)=1.0d-1;
      v(9)=8.33d-2;
      v(10)=7.14d-2;
      v(11)=6.25d-2;
      y(1)=1.957d-1;
      y(2)=1.947d-1;
      y(3)=1.735d-1;
      y(4)=1.6d-1;
      y(5)=8.44d-2;
      y(6)=6.27d-2;
      y(7)=4.56d-2;
      y(8)=3.42d-2;
      y(9)=3.23d-2;
      y(10)=2.35d-2;
      y(11)=2.46d-2;

      for(i = 1; i < 11; i++) {
         temp1 = v[i]*(v[i]+x[2]);
         temp2 = v[i]*(v[i]+x[3]) + x[4];
         fvec[i] = y[i] - x[1]*temp1/temp2;
      }


#pragma $adic_dep fvec

      }


      int main()
      {
          double a[4], b[11];
          head(a,b);     
          return 0;
      }
