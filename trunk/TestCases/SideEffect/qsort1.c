/*
    Nested function call, array of pointer as parameter
*/

#include<stdio.h>
#include<string.h>
#include<stdlib.h>


//int getline(char *, int);

void qsort(char *v[], int left, int right)
{
  int i, last;
  void swap(char *v[], int i, int j);

  if(left >= right)
    {
      return;
    }

  swap(v, left, (left+right)/2);
  last = left;

  for( i = left+1; i <= right; i++)
    {
      if(strcmp(v[i], v[left]) < 0)
	{
           swap(v, ++last, i);
	}
    }
  swap(v, left, last);
  qsort(v, left, last-1);
  qsort(v, last+1, right);
}

void swap(char *v[], int i, int j)
{
  char *temp;
 
  temp = v[i];
  v[i] = v[j];
  v[j]=temp;
}


int main()
{
  char* lineptr[10];
  char *line;
  int nlines=5, i=0, len=0;
  size_t maxlength=10;
  FILE* fp;

  while(i < 5)
    {
      if( (len = getline(&line,&maxlength,fp)) != -1)
	{
          line[len-1] = '\0';
	  lineptr[i++] = line;
	}
    }
    
  qsort(lineptr, 0, nlines-1);
  return 0;
}
