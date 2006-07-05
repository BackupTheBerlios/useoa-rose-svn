/*
   Quick Sort
   Function Pointer, recursive function call 
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

void swap(int v[], int i, int j)
{
  int temp;

  temp = v[i];
  v[i]=v[j];
  v[j]=temp;

}

void qsort(int v[], int left, int right, int (*comp)(int, int))
{
  int i, last;

  if( left >= right )
    {
      return;
    }

  swap(v, left, (left+right)/2);
  last = left;
  for(i=left; i<=right; i++)
    {
      if((*comp)(v[i], v[left]) < 0){
        swap(v, ++last, i);
      }
    }
 
  swap(v, left, last);
  qsort(v, left, last-1, comp);
  qsort(v, last+1, right, comp);
}

int numcmp(int n1, int n2)
{ 
  if( n1 < n2 )
    {
      return -1;
    } 
  else if( n1 > n2 )
    {
      return 1;
    }
  else
    {
      return 0;  
    }
}

int main()
{
  int nlines=0;
  int a[10]; 

  if(nlines < 5)
    {
      qsort(a, 0, nlines-1, (int(*)(int, int))numcmp);  
      nlines++;   
    }

  return 0;
}
