/* functionPtr2.c */
/* This is a pretty complicated example, see functionPtr2.dot for a complete
   points-to graph for this example.

 ptr assigns
   **x, **y (26)
   **yy **xx (28)
   *p, a (13)
   *q, b (16)
   *fp, foo, bar (21)
   x (1)
   *x (3)
   y (2)
   *y (4)
   xx (6)
   *xx (8)
   yy (5)
   *yy (7)
   p (11)
   q (14)
   fp (17)

 secondary effects
   none

 parameter bindings
    *x, p, *xx (11)
    *y, q, *yy  (14)
   **x, **y (26)
   **yy **xx (28)
   *p, a (13)
   *q, b (16)
   *fp, foo, bar (21)
   x (1)
   y (2)
   xx (6)
   yy (5)
   fp (17)

 secondary effects
   **x, **y, *p, a, *q, b, **yy, **xx (16) 
   *x, p, *xx (11)
   *y, q, *yy (14)
   *fp, foo, bar (21)
   x (1)
   y (2)
   xx (6)
   yy (5)
   fp (17)

  
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
