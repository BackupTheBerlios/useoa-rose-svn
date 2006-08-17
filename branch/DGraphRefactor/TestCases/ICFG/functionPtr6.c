/* functionPtr6.c */

int read(); 

int (*fp1)();      // declaration of a function pointer 
int (*fp2)();      // declaration of a function pointer 
int foo();        // prototype for function foo 
int bar();        // prototype for function bar 
int zoo();        // prototype for function zoo 
int car1(int(*)()); // prototype for function car
int car2(int(*)()); // prototype for function car

void main() { 
  int x = read();
  fp1 = zoo;
  if (x) { 
    fp1 = foo;
    fp2 = bar;
  } else { 
    fp1 = foo;
    fp2 = bar;
  } 
  car1(fp1);
  car2(fp2);
} 

int foo() {}
int bar() {}
int zoo() {}

int car1(int(*funcPtr)()) {
  int res;
  res = (*funcPtr)();
  return res;
}
int car2(int(*funcPtr)()) {
  int res;
  res = 2*(*funcPtr)();
  return res;
}
