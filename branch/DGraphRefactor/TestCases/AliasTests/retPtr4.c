#include<stdio.h>
#define ALLOCSIZE 100 /* Size fo available space */


static char allocbuf[ALLOCSIZE];  /* storage for alloc */
static char *allocp = allocbuf;   /* next free position */

char *alloc(int n)    /* return pointer to n characters */
{
    if(allocbuf + ALLOCSIZE -allocp >= n)
    {
        
        allocp += n;
        return allocp - n;    
    }
    else
    {
        /* Not enough room */
        return 0;
    }
}   


int main()
{
    char * retval = alloc(10);
    return 0;
}
    
