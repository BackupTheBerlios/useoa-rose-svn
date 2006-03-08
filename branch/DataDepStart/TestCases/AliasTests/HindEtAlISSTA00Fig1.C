/*
 * from Which Pointer Analysis Should I Use?
 * Michael Hind and Anthony Pioli.
 * International Symposium on Software Testing and Analysis (ISSTA 2000).
 * Figure 1.
 */

class T { 
 public:
  T() { }
  T(const T &t) { }
  
};

T *p, *q, *r;

void g(T **fp)
{
  T local;
  int cond;
  if ( cond > 3 )
    p = &local;
}

void f()
{
  q = new T;
  g(&p);
  r = new T;
}

int main()
{
  p = new T;
  f();
  g(&p);
  p = new T;
  
  T t = *p;
  return 0;
}
