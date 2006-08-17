/*
 *  PLM 07/28/06
 *  Reference : Do not Remember 
 *
 */

#include<stdio.h>
       

   char *my_strcpy(char *destination, char *source)
   {
       char *p = destination;
       while (*source != '\0')
       {
           *p++ = *source++;
       }
       *p = '\0';
       return destination;
   }   



    int main(void)
    {
        char strA[] = "Laurel";
        char strB[] = "Hardy";
        char* cptr = my_strcpy(strB, strA);
        puts(strB);
    }    
