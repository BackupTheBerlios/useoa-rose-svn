/* functionPtr1.c */

  int read(); 

  int (*fp)();      // declaration of a function pointer 
  int foo();        // prototype for function foo 
  int bar();        // prototype for function bar 
  int zoo();        // prototype for function bar 

  void main() { 
    int x = read(); 
    fp = zoo;
    if (x) { 
      fp = foo; 
    } 
//    else { 
//      fp = bar; 
//    } 
    fp(); 
  } 

  int foo() {}
  int bar() {}
  int zoo() {}
