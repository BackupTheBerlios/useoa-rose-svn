/* functionPtr5.c */

int read(); 

int (*fp)();      // declaration of a function pointer 
int foo();        // prototype for function foo 
int bar();        // prototype for function bar 
int zoo();        // prototype for function zoo 
int car(int(*)()); // prototype for function car

void main() { 
  int x = read(); 
  fp = zoo;
  if (x) { 
    fp = foo; 
  } else { 
    fp = bar; 
  } 
  car(fp); 
} 

int foo() {}
int bar() {}
int zoo() {}

int car(int(*funcPtr)()) {
  int res;
  res = (*funcPtr)();
  return res;
}
