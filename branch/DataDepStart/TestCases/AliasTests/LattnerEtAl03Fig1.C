/*
 * from Data Structure Analysis: A Fast and Scalable Context-Sensitive 
 * Heap Analysis, Chris Lattner & Vikram Adve, 
 * Technical Report #UIUCDCS-R-2003-2340, Computer Science Dept., 
 * Univ. of Illinois, Apr. 2003.
 * Figure 1.
 */  

#include <stdlib.h>

typedef struct list { 
  struct list *Next;
  int Data;
} list;

int Global = 10;

void do_all(list *L, void (*FP)(int *)) 
{
  do {
    FP(&(L->Data));
    L = L->Next;
  } while(L);
}

void addG(int *X) { (*X) |= Global; }
void addGToList(list *L) { do_all(L, addG); }

list *makeList(int Num)
{
  list *New = (list *)malloc(sizeof(list));
  New->Next = Num ? makeList(Num - 1) : 0;
  New->Data = Num;
  return New;
}

int main()
{
  list *X = makeList(10);
  list *Y = makeList(100);
  addGToList(X);
  Global = 20;
  addGToList(Y);
  *X;
  *Y;
  return 0;
}
