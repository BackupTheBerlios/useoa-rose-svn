#include<stdio.h>
#include<stdlib.h>

char *month_name(int n)
{
    static char *name[] = 
    {
        "Illegal Month",
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };

    return ( n < 1 || n > 12 ) ? name[0] : name[n];
}

int main(void)
{
    int i;
    char* retval;

    retval = (char *)malloc(100);
    
    for(i=0; i <= 12; i++)
    {
        retval = month_name(i);
    }
    
    return 0;
    
}
