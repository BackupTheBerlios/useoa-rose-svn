	void head(double x[1], double y[1])
    {
c$openad INDEPENDENT(x)
          y[1]=x[1]*float(2*3);
c$openad DEPENDENT(y)
    }

    int main()
    {
        double a[1], b[1];
        head(a,b);
        return 0;
    } 
