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

  

  item()
{
  }

  

  ~item()
{
  }

  

  item(class item &rhs) : source(rhs.source)
{
  }

  

  item &operator=(class item &rhs)
{
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> source = rhs.source;
    return  *(this);
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

  

  counter() : item()
{
  }

  

  ~counter()
{
    ((class item *)(this)) -> ~item();
  }

  

  counter(class counter &rhs) : value(rhs.value), item(rhs)
{
  }

  

  counter &operator=(class counter &rhs)
{
    (*((class item *)(this)))=((class item &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> value = rhs.value;
    return  *(this);
  }

}

;

class sieve : public item
{
  public: virtual int out();
  

  inline sieve(class item *src) : item(src)
{
  }

  

  sieve() : item()
{
  }

  

  ~sieve()
{
    ((class item *)(this)) -> ~item();
  }

  

  sieve(class sieve &rhs) : item(rhs)
{
  }

  

  sieve &operator=(class sieve &rhs)
{
    (*((class item *)(this)))=((class item &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    return  *(this);
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

  

  filter() : item()
{
  }

  

  ~filter()
{
    ((class item *)(this)) -> ~item();
  }

  

  filter(class filter &rhs) : factor(rhs.factor), item(rhs)
{
  }

  

  filter &operator=(class filter &rhs)
{
    (*((class item *)(this)))=((class item &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> factor = rhs.factor;
    return  *(this);
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

// stubs

extern "C" { int printf(const char *format,... )
{
  return 0;
}

 }
