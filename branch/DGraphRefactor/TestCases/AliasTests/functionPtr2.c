/* functionPtr2.c */
/* This is a pretty complicated example, see functionPtr2.dot for a complete
   points-to graph for this example.
*/

  int read(); 

  int foo(int **x, int **y) {
    *x = *y;
  }

  int bar(int **xx, int **yy) {
    *yy = *xx;
  }

  void main() { 
    int input = read(); 
    int a, b;
    int *p = &a;
    int *q = &b;

    int (*fp)(int**, int**);

    if (input) { 
      fp = foo; 
    } else { 
      fp = bar; 
    } 
    fp(&p, &q); 
  } 

//  int foo() {}
//  int bar() {}
