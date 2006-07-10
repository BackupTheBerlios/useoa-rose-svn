/* functionPtr3.c */

  int read(); 

  int (*fp)();      // declaration of a function pointer 
  int foo();        // prototype for function foo 
  int bar();        // prototype for function bar 
  int zoo();        // prototype for function zoo 

  void main() { 
    int x = read(); 
    if (x) { 
      fp = foo; 
    } else { 
      fp = bar; 
    } 
    fp(); 
  } 

  int foo() {}
  int bar() {}
  int zoo() {}
