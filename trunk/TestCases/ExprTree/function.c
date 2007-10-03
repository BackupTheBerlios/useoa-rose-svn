
    int foo(int g, int f) {
         g=f;
         return g;
    }

    int main()
    {
        int a,b,c;
        c = foo(a=b, c=a=b);
        return 0;
    }
