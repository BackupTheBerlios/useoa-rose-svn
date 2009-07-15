/*
 * FIAlias results should be: (*r,y,*p) (*q,x) 
 */
#include <stdio.h>

int main()
{
    int x=1, y=2, z=3;
    int *p;  
    int *q;  
    int *r = &y;  

    // comma operator: evaluated left-to-right and rightmost is result
    // q = &x, p=r
    p = (q = &x, r);

    printf ("*p = %d\n", *p);
    printf ("*q = %d\n", *q);
    return 0;
}
