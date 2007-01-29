//! LinearityAnalysis for Automatic Differentiation
//!
//!Result Should be:
//!<<a,a>,linear>, <<a,y>,nonlinear>, <<a,x>,nonlinear>
//!<<y,y>,nonlinear>, <<y,x>,nonlinear>
//!      

int main()
{
   double a, b, c, x, y, z;
   a = 0.0;
   b = 0.0;
   c = 0.0;
   x = 0.0;
   y = 0.0;
   z = 0.0;
   int i;

   a = 0;
   y = x;
   for(i=0; i<3; i++) {
     a = a + y;
     y = y * x;
   }
   return 0;
}
