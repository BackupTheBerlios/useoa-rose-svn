
    void foo (int g) {
         g=10;
    }

    int main()
    {
        void (*fp)(int);
        fp = foo;
        return 0;
    }
