      
       void head(double x[2], double y[1])
       {

#pragma $adic_indep x
         y[1] = x[1] * x[2];
#pragma $adic_dep y 
       }
 
       int main() {
           double a[2], b[1];

           head(a,b);
           return 0;
       }
