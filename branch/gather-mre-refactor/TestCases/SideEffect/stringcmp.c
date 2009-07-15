#include<stdio.h>

int stringcmp(char *s, char *t)
{
   int i;
   for(i=0; s[i] == t[i]; i++)
   {
      if(s[i] != '\0')
      {
	      return 0;
      } 
   }
   return (s[i] - t[i]);
}

int main()
{
   char a[] = "source";
   char b[] = "dest";   
   int retVal = 0;
   
   retVal = stringcmp(a,b);
  
   return 0;
}
