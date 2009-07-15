#include<stdio.h>

int main()
{
  double x, y, z;

  x = 2;
  if ( x >= 0 ) 
  {
    y = 5;
    x = y;
    x = y + y;
    y = x * 5;
  }else
  {
    y = 3;
  }
  
  z = y;

  return 0; 
}
