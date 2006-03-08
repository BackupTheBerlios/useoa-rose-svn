#include <stdio.h>

int main()
{
    int x=1, y=2, z=3;
    int *p;  
    int *q;  
    int *r = &y;  

    p = (q = &x, r);

    printf ("*p = %d\n", *p);
    printf ("*q = %d\n", *q);
    return 0;
}
