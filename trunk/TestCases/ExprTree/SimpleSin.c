//! LinearityAnalysis for Automatic Differentiation Example
        #include <math.h>

        void foo(double, double, double, double);

        int main()
        {
          double a, f, t;
          double x;
        
          foo (a,f,x,t);
          return 0;
        }
        
        void foo(double a, double f,double x,double t)
        {
          int i;
          a = ((a*x)+sin(t));
        }

