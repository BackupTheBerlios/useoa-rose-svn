/* Swap Function.
   a[] is pass by reference
   i, i+1 are pass by value
*/


#include<stdio.h>

void swap(int v[], int i, int j)
{
  int temp;
  
  temp = v[i];
  v[i]=v[j];
  v[j]=temp;

}

int main()
{
  int a[5];
  int i,j;

  for(i=0; i<4; i++)
    {
      swap(a,i,i+1);
    }

  return 0;
}
