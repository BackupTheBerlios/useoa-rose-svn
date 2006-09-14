// From Ravi Sethi's book on Programming Languages.
#include <stdio.h>

class item 
{
  public: class item *source;
  

  inline item(class item *src)
{
    (this) -> source = src;
  }

  

  virtual inline int out()
{
    return 0;
  }

}

;

class counter : public item
{
  private: int value;
  

  public: virtual inline int out()
{
    return (this) -> value++;
  }

  

  inline counter(int v) : item(((0)))
{
    (this) -> value = v;
  }

}

;

class sieve : public item
{
  public: virtual int out();
  

  inline sieve(class item *src) : item(src)
{
  }

}

;

class filter : public item
{
  private: int factor;
  public: virtual int out();
  

  inline filter(class item *src,int f) : item(src)
{
    (this) -> factor = f;
  }

}

;

int main()
{
  class counter c(2);
  class sieve s(((&c)));
  int next;
  do {
    next = s.sieve::out();
    printf("%d ",next);
  }while (next < 61);
  printf("\n");
}


int sieve::out()
{
  int n = ( *((this) -> source)).out();
  (this) -> source = ((new filter (((this) -> source),n)));
  return n;
}


int filter::out()
{
  while((1)){
    int n = ( *((this) -> source)).out();
    if ((n % ((this) -> factor))) {
      return n;
    }
    else {
    }
  }
}

