

    #include <stdio.h> 
    #include <math.h>

    #define PI  3.14159265358979323844d0  
    #define deg2rad  (2.d0*PI/360.d0)      

    void head(double x[1], double y[1]) 
    {
#pragma $adic_indep x

          y[1]=cos(deg2rad*x[1]);

#pragma $adic_dep y 

    }

    int main() 
    {
       double a[1], b[1];
       head(a,b); 
       return 0;
    }
