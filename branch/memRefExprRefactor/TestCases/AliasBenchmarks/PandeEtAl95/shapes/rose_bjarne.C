/* this program is taken from Section 6.4
   in Bjarne Stroustrup's book "The C++ Programming Language" 2nd Edition. */
#include <iostream>
#include "bjarne.h"
using namespace std;
enum color {black=42,white=32};
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
  return ((((0 <= a) && (a < XMAX)) && (0 <= b)) && (b < YMAX));
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
    if ((x0 == x1) && (y0 == y1)) {
      break; 
    }
    else {
    }
    if (eps <= xcrit) {
      (x0 += dx) , eps += two_b;
    }
    else {
    }
    if ((eps >= a) && (a <= b)) {
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
}

;

class line : public shape
{
  private: struct point *w;
  struct point *e;
  

  public: virtual inline point *north() const
{
    struct point *ret = new point ((((( *((this) -> w)).x) + (( *((this) -> e)).x)) / 2),((( *((this) -> e)).y) < (( *((this) -> w)).y))?(( *((this) -> w)).y):(( *((this) -> e)).y));
    return ret;
  }

  

  virtual inline point *south() const
{
    struct point *ret = new point ((((( *((this) -> w)).x) + (( *((this) -> e)).x)) / 2),((( *((this) -> e)).y) < (( *((this) -> w)).y))?(( *((this) -> e)).y):(( *((this) -> w)).y));
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

  

  inline line(struct point *a,struct point *b)
{
    (this) -> w = a;
    (this) -> e = b;
  }

  

  inline line(struct point *a,int l)
{
    (this) -> w = (new point ((((a -> x) + l) - 1),(a -> y)));
    (this) -> e = a;
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
}

;

rectangle::rectangle(struct point *a,struct point *b)
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

