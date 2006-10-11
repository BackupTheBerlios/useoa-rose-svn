/* this program is taken from Section 6.4
   in Bjarne Stroustrup's book "The C++ Programming Language" 2nd Edition. */
#include <iostream>
#include "bjarne.h"
using namespace std;
enum color {black='*',white=' '};
char screen[40][24];

void screen_init()
{
  for (int y = 0; y < YMAX; y++) {
    for (int x = 0; x < XMAX; x++) {
      (screen[x])[y] = (white);
    }
  }
}


void screen_destroy()
{
}

// clipping

inline int on_screen(int a,int b)
{
  if ((0 <= a)) {
    if ((a < XMAX)) {
      if ((0 <= b)) {
        return (b < YMAX);
      }
      else {
        return false;
      }
    }
    else {{
        return false;
      }
    }
  }
  else {{{
        return false;
      }
    }
  }
}


void put_point(int a,int b)
{
  if ((on_screen(a,b))) {
    (screen[a])[b] = (black);
  }
  else {
  }
}


void put_line(int x0,int y0,int x1,int y1)
{
  int dx = 1;
  int a = (x1 - x0);
  if (a < 0) {
    (dx = (-1)) , a = (-a);
  }
  else {
  }
  int dy = 1;
  int b = (y1 - y0);
  if (b < 0) {
    (dy = (-1)) , b = (-b);
  }
  else {
  }
  int two_a = (2 * a);
  int two_b = (2 * b);
  int xcrit = ((-b) + two_a);
  int eps = 0;
  for (; ; ) {
    put_point(x0,y0);
    bool rose_sc_bool_2 = false;
    if ((x0 == x1)) {
      if ((y0 == y1)) {
        rose_sc_bool_2 = true;
      }
      else {
      }
    }
    else {{
      }
    }
    if (rose_sc_bool_2) {
      break; 
    }
    else {
    }
    if (eps <= xcrit) {
      (x0 += dx) , eps += two_b;
    }
    else {
    }
    bool rose_sc_bool_3 = false;
    if ((eps >= a)) {
      if ((a <= b)) {
        rose_sc_bool_3 = true;
      }
      else {
      }
    }
    else {{
      }
    }
    if (rose_sc_bool_3) {
      (y0 += dy) , eps -= two_a;
    }
    else {
    }
  }
}


void screen_clear()
{
  screen_init();
}


void screen_refresh()
{
// top to bottom
  for (int y = (YMAX - 1); 0 <= y; y--) {
// left to right
    for (int x = 0; x < XMAX; x++) {
       *((&std::cout))<<((screen[x])[y]);
    }
     *((&std::cout))<<'\n';
  }
}


struct shape 
{
  static struct shape *list;
  struct shape *next;
  

  inline shape()
{
    (this) -> next = shape::list;
    shape::list = (this);
  }

  virtual point *north() const = 0;
  virtual point *south() const = 0;
  virtual point *east() const = 0;
  virtual point *west() const = 0;
  virtual point *neast() const = 0;
  virtual point *seast() const = 0;
  virtual point *nwest() const = 0;
  virtual point *swest() const = 0;
  virtual void draw() = 0;
  virtual void move(int ,int ) = 0;
  

  ~shape()
{
  }

  

  shape(struct shape &rhs)
{
  }

  

  shape &operator=(struct shape &rhs)
{
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    return  *(this);
  }

}

;

class line : public shape
{
  private: struct point *w;
  struct point *e;
  

  public: virtual inline point *north() const
{
    struct point *ret;
    if (((( *((this) -> e)).y) < (( *((this) -> w)).y))) {
      ret = (new point ((((( *((this) -> w)).x) + (( *((this) -> e)).x)) / 2),(( *((this) -> w)).y)));
    }
    else {
      ret = (new point ((((( *((this) -> w)).x) + (( *((this) -> e)).x)) / 2),(( *((this) -> e)).y)));
    }
    return ret;
  }

  

  virtual inline point *south() const
{
    struct point *ret;
    if (((( *((this) -> e)).y) < (( *((this) -> w)).y))) {
      ret = (new point ((((( *((this) -> w)).x) + (( *((this) -> e)).x)) / 2),(( *((this) -> e)).y)));
    }
    else {
      ret = (new point ((((( *((this) -> w)).x) + (( *((this) -> e)).x)) / 2),(( *((this) -> w)).y)));
    }
    return ret;
  }

  

  virtual inline point *east() const
{
  }

  

  virtual inline point *west() const
{
  }

  

  virtual inline point *neast() const
{
  }

  

  virtual inline point *seast() const
{
  }

  

  virtual inline point *nwest() const
{
  }

  

  virtual inline point *swest() const
{
  }

  

  virtual inline void move(int a,int b)
{
    ( *((this) -> w)).x += a;
    ( *((this) -> w)).y += b;
    ( *((this) -> e)).x += a;
    ( *((this) -> e)).y += b;
  }

  

  virtual inline void draw()
{
    put_line(((this) -> w),((this) -> e));
  }

  

  inline line(struct point *a,struct point *b) : shape()
{
    (this) -> w = a;
    (this) -> e = b;
  }

  

  inline line(struct point *a,int l) : shape()
{
    (this) -> w = (new point ((((a -> x) + l) - 1),(a -> y)));
    (this) -> e = a;
  }

  

  line() : shape()
{
  }

  

  ~line()
{
    ((struct shape *)(this)) -> ~shape();
  }

  

  line(class line &rhs) : w(rhs.w), e(rhs.e), shape(rhs)
{
  }

  

  line &operator=(class line &rhs)
{
    (*((struct shape *)(this)))=((struct shape &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> w = rhs.w;
    (this) -> e = rhs.e;
    return  *(this);
  }

}

;

class rectangle : public shape
{
  public: struct point *sw;
  struct point *ne;
  

  virtual inline point *north() const
{
    struct point *ret = new point ((((( *((this) -> sw)).x) + (( *((this) -> ne)).x)) / 2),(( *((this) -> ne)).y));
    return ret;
  }

  

  virtual inline point *south() const
{
    struct point *ret = new point ((((( *((this) -> sw)).x) + (( *((this) -> ne)).x)) / 2),(( *((this) -> sw)).y));
    return ret;
  }

  

  virtual inline point *east() const
{
  }

  

  virtual inline point *west() const
{
  }

  

  virtual inline point *neast() const
{
    return (this) -> ne;
  }

  

  virtual inline point *seast() const
{
  }

  

  virtual inline point *nwest() const
{
  }

  

  virtual inline point *swest() const
{
    return (this) -> sw;
  }

  

  virtual inline void move(int a,int b)
{
    ( *((this) -> sw)).x += a;
    ( *((this) -> sw)).y += b;
    ( *((this) -> ne)).x += a;
    ( *((this) -> ne)).y += b;
  }

  virtual void draw();
  rectangle(struct point *,struct point *);
  

  rectangle() : shape()
{
  }

  

  ~rectangle()
{
    ((struct shape *)(this)) -> ~shape();
  }

  

  rectangle(class rectangle &rhs) : sw(rhs.sw), ne(rhs.ne), shape(rhs)
{
  }

  

  rectangle &operator=(class rectangle &rhs)
{
    (*((struct shape *)(this)))=((struct shape &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> sw = rhs.sw;
    (this) -> ne = rhs.ne;
    return  *(this);
  }

}

;

rectangle::rectangle(struct point *a,struct point *b) : shape()
{
  if ((a -> x) <= (b -> x)) {
    if ((a -> y) <= (b -> y)) {
      (this) -> sw = a;
      (this) -> ne = b;
    }
    else {
      (this) -> sw = (new point ((a -> x),(b -> y)));
      (this) -> ne = (new point ((b -> x),(a -> y)));
    }
  }
  else {
    if ((a -> y) <= (b -> y)) {
      (this) -> sw = (new point ((b -> x),(a -> y)));
      (this) -> ne = (new point ((a -> x),(b -> y)));
    }
    else {
      (this) -> sw = b;
      (this) -> ne = a;
    }
  }
}


void rectangle::draw()
{
  struct point nw((( *((this) -> sw)).x),(( *((this) -> ne)).y));
  struct point se((( *((this) -> ne)).x),(( *((this) -> sw)).y));
  put_line(&nw,((this) -> ne));
  put_line(((this) -> ne),&se);
  put_line(&se,((this) -> sw));
  put_line(((this) -> sw),&nw);
}

struct shape *shape::list = (0);

void shape_refresh()
{
  screen_clear();
  for (struct shape *p = shape::list; p; p = (p -> next)) {
    p -> draw();
  }
  screen_refresh();
}

// put p on top of q

void stack(struct shape *p,const struct shape *q)
{
  struct point *n = q -> north();
  struct point *s = p -> south();
  p -> move(((n -> x) - (s -> x)),(((n -> y) - (s -> y)) + 1));
}


class myshape : public rectangle
{
  private: class line *l_eye;
  class line *r_eye;
  class line *mouth;
  

  public: inline myshape(struct point *a,struct point *b) : rectangle(a,b)
{
    int ll = (((( *(this) -> neast()).x) - (( *(this) -> swest()).x)) + 1);
    int hh = (((( *(this) -> neast()).y) - (( *(this) -> swest()).y)) + 1);
    struct point *l_eye_point = new point (((( *(this) -> swest()).x) + 2),((( *(this) -> swest()).y) + ((hh * 3) / 4)));
    struct point *r_eye_point = new point ((((( *(this) -> swest()).x) + ll) - 4),((( *(this) -> swest()).y) + ((hh * 3) / 4)));
    struct point *mouth_point = new point (((( *(this) -> swest()).x) + 2),((( *(this) -> swest()).y) + (hh / 4)));
    (this) -> l_eye = (new line (l_eye_point,2));
    (this) -> r_eye = (new line (r_eye_point,2));
    (this) -> mouth = (new line (mouth_point,(ll - 4)));
  }

  virtual void draw();
  virtual void move(int ,int );
  

  myshape() : rectangle()
{
  }

  

  ~myshape()
{
    ((class rectangle *)(this)) -> ~rectangle();
  }

  

  myshape(class myshape &rhs) : l_eye(rhs.l_eye), r_eye(rhs.r_eye), mouth(rhs.mouth), rectangle(rhs)
{
  }

  

  myshape &operator=(class myshape &rhs)
{
    (*((class rectangle *)(this)))=((class rectangle &)rhs);
    if ((this) == &rhs) {
      return  *(this);
    }
    else {
    }
    (this) -> l_eye = rhs.l_eye;
    (this) -> r_eye = rhs.r_eye;
    (this) -> mouth = rhs.mouth;
    return  *(this);
  }

}

;

void myshape::draw()
{
  (this) -> rectangle::draw();
  int a = (((( *(this) -> swest()).x) + (( *(this) -> neast()).x)) / 2);
  int b = (((( *(this) -> swest()).y) + (( *(this) -> neast()).y)) / 2);
  put_point((new point (a,b)));
}


void myshape::move(int a,int b)
{
  (this) -> rectangle::move(a,b);
  ( *((this) -> l_eye)).move(a,b);
  ( *((this) -> r_eye)).move(a,b);
  ( *((this) -> mouth)).move(a,b);
}


int main()
{
  screen_init();
  struct point *point00 = new point (0,0);
  struct point *point1010 = new point (10,10);
  struct point *point015 = new point (0,15);
  struct point *point1510 = new point (15,10);
  struct point *point2718 = new point (27,18);
  struct shape *p1 = (new rectangle (point00,point1010));
  struct shape *p2 = (new line (point015,17));
  struct shape *p3 = ((new myshape (point1510,point2718)));
  shape_refresh();
  p3 -> move((-10),(-10));
  stack(p2,(p3));
  stack(p1,(p2));
  shape_refresh();
  screen_destroy();
  return 0;
}

// stubs

extern "C" { int pthread_once(pthread_once_t *once_control,void (*init_routine)()) throw()
{
  return 0;
}

 }

extern "C" { int pthread_key_create(pthread_key_t *key,void (*destr_function)(void *)) throw()
{
  return 0;
}

 }

extern "C" { int pthread_setspecific(pthread_key_t key,const void *pointer) throw()
{
  return 0;
}

 }

extern "C" { int pthread_key_delete(pthread_key_t key) throw()
{
  return 0;
}

 }

extern "C" { void *pthread_getspecific(pthread_key_t key) throw()
{
  return (0);
}

 }

extern "C" { int pthread_mutex_lock(pthread_mutex_t *mutex) throw()
{
  return 0;
}

 }

extern "C" { int pthread_mutex_trylock(pthread_mutex_t *mutex) throw()
{
  return 0;
}

 }

extern "C" { int pthread_mutex_unlock(pthread_mutex_t *mutex) throw()
{
  return 0;
}

 }

extern "C" { wchar_t *wcschr(const wchar_t *wcs,wchar_t wc) throw()
{
  wchar_t *wcshrtemp;
  return wcshrtemp;
}

 }

extern "C" { wchar_t *wcspbrk(const wchar_t *wcs,const wchar_t *accept) throw()
{
  wchar_t *wcspbrktemp;
  return wcspbrktemp;
}

 }

extern "C" { wchar_t *wcsrchr(const wchar_t *wcs,wchar_t wc) throw()
{
  wchar_t *wcsrchrtemp;
  return wcsrchrtemp;
}

 }

extern "C" { wchar_t *wcsstr(const wchar_t *haystack,const wchar_t *needle) throw()
{
  wchar_t *wcsstrtemp;
  return wcsstrtemp;
// Returning value before it is set, giving warning, problem?
}

 }

extern "C" { wchar_t *wmemchr(const wchar_t *s,wchar_t c,size_t n) throw()
{
  wchar_t *wmemchrtemp;
  return wmemchrtemp;
}

 }

extern "C" { void *memchr(const void *s,int c,size_t n) throw()
{
  return (0);
}

 }

char *__builtin_strchr(const char *str,int ch)
{
  char *__builtin_strchrtemp;
  return __builtin_strchrtemp;
// http://www.cs.berkeley.edu/~smcpeak/elkhound/sources/elsa/gnu.cc
}


char *__builtin_strpbrk(const char *str,const char *accept)
{
  char *__builtin_strpbrktemp;
  return __builtin_strpbrktemp;
}


char *__builtin_strrchr(const char *str,int ch)
{
  char *__builtin_strrchrtemp;
  return __builtin_strrchrtemp;
}


char *__builtin_strstr(const char *haystack,const char *needle)
{
  char *__builtin_strstrtemp;
  return __builtin_strstrtemp;
}


extern "C" { int memcmp(const void *s1,const void *s2,size_t n) throw()
{
  return 0;
}

 }

extern "C" { size_t strlen(const char *s) throw()
{
  size_t strlentemp;
  return strlentemp;
}

 }

extern "C" { void *memmove(void *dest,const void *src,size_t n) throw()
{
  return (0);
}

 }

extern "C" { void *memcpy(void *dest,const void *src,size_t n) throw()
{
  return (0);
}

 }

extern "C" { void *memset(void *s,int c,size_t n) throw()
{
  return (0);
}

 }

extern "C" { int wmemcmp(const wchar_t *s1,const wchar_t *s2,size_t n) throw()
{
  return 0;
}

 }

extern "C" { size_t wcslen(const wchar_t *s) throw()
{
  size_t wcslentemp;
  return wcslentemp;
}

 }

extern "C" { wchar_t *wmemmove(wchar_t *dest,const wchar_t *src,size_t n) throw()
{
  wchar_t *wmemmovetemp;
  return wmemmovetemp;
}

 }

extern "C" { wchar_t *wmemcpy(wchar_t *dest,const wchar_t *src,size_t n) throw()
{
  wchar_t *wmemcpy;
  return wmemcpy;
}

 }

extern "C" { wchar_t *wmemset(wchar_t *wcs,wchar_t wc,size_t n) throw()
{
  wchar_t *wmemset;
  return wmemset;
}

 }

extern "C" { long labs(long j) throw()
{
  return (0);
}

 }

extern "C" { ldiv_t ldiv(long numer,long denom) throw()
{
  ldiv_t ldivtemp;
  return ldivtemp;
}

 }

void *operator new(size_t size) throw(std::bad_alloc )
{
}


void operator delete(void *obj,size_t size)
{
}

#if 0
#endif

extern "C" { int strcmp(const char *s1,const char *s2) throw()
{
  return 0;
}

 }

extern "C" { char *strncpy(char *dest,const char *src,size_t n) throw()
{
  char *strncpytemp;
  return strncpytemp;
}

 }

extern "C" { int iconv_close(iconv_t cd)
{
  return 0;
}

 }

extern "C" { iconv_t iconv_open(const char *tocode,const char *fromcode)
{
  iconv_t iconv_opentemp;
  return iconv_opentemp;
}

 }

void __throw_runtime_error(const char *)
{
}


double __builtin_fabs(double x)
{
  return (0);
}


float __builtin_fabsf(float x)
{
  return (0);
}


long double __builtin_fabs1(long double x)
{
  return (0);
}


extern "C" { float acosf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double acosl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float asinf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double asinl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float atanf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double atanl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float atan2f(float y,float x) throw()
{
  return (0);
}

 }

extern "C" { long double atan2l(long double y,long double x) throw()
{
  return (0);
}

 }

extern "C" { float ceilf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double ceill(long double x) throw()
{
  return (0);
}

 }

float __builtin_cosf(float x)
{
  return (0);
}


long double __builtin_cosl(long double x)
{
  return (0);
}


extern "C" { float coshf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double coshl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float expf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double expl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float floorf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double floorl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float fmodf(float x,float y) throw()
{
  return (0);
}

 }

extern "C" { long double fmodl(long double x,long double y) throw()
{
  return (0);
}

 }

extern "C" { float frexpf(float num,int *exp) throw()
{
  return (0);
}

 }

extern "C" { long double frexpl(long double num,int *exp) throw()
{
  return (0);
}

 }

extern "C" { float ldexpf(float x,int exp) throw()
{
  return (0);
}

 }

extern "C" { long double ldexpl(long double x,int exp) throw()
{
  return (0);
}

 }

extern "C" { float logf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double logl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float log10f(float x) throw()
{
  return (0);
}

 }

extern "C" { long double log10l(long double x) throw()
{
  return (0);
}

 }

extern "C" { float modff(float value,float *iptr) throw()
{
  return (0);
}

 }

extern "C" { long double modfl(long double value,long double *iptr) throw()
{
  return (0);
}

 }

extern "C" { float powf(float x,float y) throw()
{
  return (0);
}

 }

extern "C" { long double powl(long double x,long double y) throw()
{
  return (0);
}

 }

float __builtin_sinf(float x)
{
  return (0);
}


long double __builtin_sinl(long double x)
{
  return (0);
}


extern "C" { float sinhf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double sinhl(long double x) throw()
{
  return (0);
}

 }

float __builtin_sqrtf(float x)
{
  return (0);
}


long double __builtin_sqrtl(long double x)
{
  return (0);
}


extern "C" { float tanf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double tanl(long double x) throw()
{
  return (0);
}

 }

extern "C" { float tanhf(float x) throw()
{
  return (0);
}

 }

extern "C" { long double tanhl(long double x) throw()
{
  return (0);
}

 }

float __builtin_nansf(const char *str)
{
  return (0);
}


double __builtin_nans(const char *str)
{
  return (0);
}


long double __builtin_nansl(const char *str)
{
  return (0);
}


extern "C" { char *strcpy(char *dest,const char *src) throw()
{
  char *strcpytemp;
  return strcpytemp;
}

 }
// Missing SgTemplateInstantiationFunctionDecl [referenced]
