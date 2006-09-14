/* this program is taken from Section 6.4
   in Bjarne Stroustrup's book "The C++ Programming Language" 2nd Edition. */
#include <iostream>
#include "bjarne.h"

using namespace std;

enum color {black='*', white=' '};

char screen[XMAX][YMAX];

void screen_init() {
    for (int y=0; y<YMAX; y++)
	for (int x=0; x<XMAX; x++)
	    screen[x][y]=white;
}

void screen_destroy() {}

inline int on_screen(int a, int b) { // clipping
    return 0<=a && a<XMAX && 0<=b && b<YMAX;
}

void put_point(int a, int b) {
    if (on_screen(a,b)) screen[a][b] = black;
}

void put_line(int x0, int y0, int x1, int y1) {
    int dx=1;
    int a = x1-x0;
    if (a < 0) dx = -1, a = -a;

    int dy=1;
    int b = y1-y0;
    if (b < 0) dy = -1, b = -b;

    int two_a = 2*a;
    int two_b = 2*b;
    int xcrit = -b + two_a;
    int eps = 0;
    for (;;) {
	put_point(x0,y0);
	if (x0==x1 && y0==y1) break;
	if (eps <= xcrit) x0 += dx, eps += two_b;
	if (eps>=a && a<=b) y0 += dy, eps -= two_a;
    }
}

void screen_clear() { screen_init(); }

void screen_refresh() {
    for (int y=YMAX-1; 0<=y; y--) {  // top to bottom
	for (int x=0; x<XMAX; x++)   // left to right
	    cout << screen[x][y];
	cout << '\n';
    }
}

struct  shape {
    static shape *list;
    shape *next;

    shape() { next = list; list = this; }

    virtual point *north() const = 0;
    virtual point *south() const = 0;
    virtual point *east() const = 0;
    virtual point *west() const = 0;
    virtual point *neast() const = 0;
    virtual point *seast() const = 0;
    virtual point *nwest() const = 0;
    virtual point *swest() const = 0;

    virtual void draw() = 0;
    virtual void move(int, int) = 0;
};

class line : public shape {
    point *w,*e;
public:
    point *north() const
	{  point *ret = new point((w->x+e->x)/2,e->y<w->y?w->y:e->y);
	   return ret;
	}
    point *south() const
        {  point *ret = new point((w->x+e->x)/2,e->y<w->y?e->y:w->y);
           return ret;
        }
    point *east() const {}
    point *west() const {}
    point *neast() const {}
    point *seast() const {}
    point *nwest() const {}
    point *swest() const {}

    void move (int a, int b) {
	w->x += a; w->y += b; e->x += a; e->y += b; }
    void draw () { put_line(w,e); }

    line(point *a, point *b) { w = a; e = b; }
    line(point *a, int l) { w = new point(a->x+l-1,a->y);e = a; }
};

class rectangle : public shape {
public:
    point *sw,*ne;
    point *north() const
        {  point *ret = new point((sw->x+ne->x)/2,ne->y);
           return ret;
        }
    point *south() const
        {  point *ret = new point((sw->x+ne->x)/2,sw->y);
           return ret;
        }
    point *east() const {}
    point *west() const {}
    point *neast() const {return ne;}
    point *seast() const {}
    point *nwest() const {}
    point *swest() const {return sw;}
   
    void move (int a, int b) {
        sw->x += a; sw->y += b; ne->x += a; ne->y += b; }
    void draw ();

    rectangle(point *, point *);
};

rectangle::rectangle(point *a, point *b) {
    if (a->x <= b->x) {
	if (a->y <= b->y) {
	    sw = a;
	    ne = b;
	}
	else {
	    sw = new point(a->x,b->y);
	    ne = new point(b->x,a->y);
	}
    }
    else {
	if (a->y <= b->y) {
	    sw = new point(b->x,a->y);
	    ne = new point(a->x,b->y);
	}
	else {
	    sw = b;
	    ne = a;
	}
    }
}

void rectangle::draw() {
    point nw(sw->x,ne->y);
    point se(ne->x,sw->y);
    put_line(&nw,ne);
    put_line(ne,&se);
    put_line(&se,sw);
    put_line(sw,&nw);
}

shape* shape::list = 0;

void shape_refresh() {
    screen_clear();
    for (shape *p = shape::list; p; p=p->next) p->draw();
    screen_refresh();
}

void stack(shape *p, const shape *q) { // put p on top of q
    point *n = q->north();
    point *s = p->south();
    p->move(n->x-s->x,n->y-s->y+1);
}

class myshape : public rectangle {
    line *l_eye;
    line *r_eye;
    line *mouth;
public:
    myshape(point *a, point *b) : rectangle(a,b) {
    int ll = neast()->x-swest()->x+1;
    int hh = neast()->y-swest()->y+1;
    point *l_eye_point = new point(swest()->x+2,swest()->y+hh*3/4);
    point *r_eye_point = new point(swest()->x+ll-4,swest()->y+hh*3/4);
    point *mouth_point = new point(swest()->x+2,swest()->y+hh/4);
    l_eye = new line(l_eye_point,2);
    r_eye = new line(r_eye_point,2);
    mouth = new line(mouth_point,ll-4);
    }

    void draw();
    void move(int, int);
};

void myshape::draw() {
    rectangle::draw();
    int a = (swest()->x+neast()->x)/2;
    int b = (swest()->y+neast()->y)/2;
    put_point(new point(a,b));
}

void myshape::move(int a, int b) {
    rectangle::move(a,b);
    l_eye->move(a,b);
    r_eye->move(a,b);
    mouth->move(a,b);
}

int main() {
    screen_init();
    point *point00 = new point(0,0);
    point *point1010 = new point(10,10);
    point *point015 = new point(0,15);
    point *point1510 = new point(15,10);
    point *point2718 = new point(27,18);
    shape *p1 = new rectangle(point00,point1010);
    shape *p2 = new line(point015,17);
    shape *p3 = new myshape(point1510,point2718);
    shape_refresh();
    p3->move(-10,-10);
    stack(p2,p3);
    stack(p1,p2);
    shape_refresh();
    screen_destroy();
    return 0;
}



// stubs

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) __THROW 
{
	return 0;
}

int pthread_key_create(pthread_key_t *key, void (*destr_function) (void *)) __THROW
{
	return 0;
}

int  pthread_setspecific(pthread_key_t key, const void *pointer) __THROW
{
	return 0;
}

int pthread_key_delete(pthread_key_t key) __THROW
{
	return 0;
}

void * pthread_getspecific(pthread_key_t key) __THROW
{
	return NULL;	
}


int pthread_mutex_lock(pthread_mutex_t *mutex) __THROW
{
  return 0;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex) __THROW
{
  return 0;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex) __THROW
{
  return 0;
}

wchar_t *wcschr(const wchar_t *wcs, wchar_t wc) __THROW
{
  wchar_t *wcshrtemp;
  return wcshrtemp;
}


wchar_t *wcspbrk(const wchar_t *wcs, const wchar_t *accept) __THROW
{
  wchar_t *wcspbrktemp;
  return wcspbrktemp;
}

wchar_t *wcsrchr(const wchar_t *wcs, wchar_t wc) __THROW
{
  wchar_t *wcsrchrtemp;
  return wcsrchrtemp;
}

wchar_t *wcsstr(const wchar_t *haystack, const wchar_t *needle) __THROW
{
  wchar_t *wcsstrtemp;
  return wcsstrtemp;
  // Returning value before it is set, giving warning, problem?
}

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n) __THROW
{
  wchar_t *wmemchrtemp;
  return wmemchrtemp;
}

void *memchr(const void *s, int c, size_t n) __THROW
{
	return NULL;
}

char *__builtin_strchr(char const *str, int ch)
{
  char *__builtin_strchrtemp;
  return __builtin_strchrtemp;
  // http://www.cs.berkeley.edu/~smcpeak/elkhound/sources/elsa/gnu.cc
}

char *__builtin_strpbrk(char const *str, char const *accept)
{
  char *__builtin_strpbrktemp;
  return __builtin_strpbrktemp;
}

char *__builtin_strrchr(char const *str, int ch)
{
  char *__builtin_strrchrtemp;
  return __builtin_strrchrtemp;
}

char *__builtin_strstr(char const *haystack, char const *needle)
{
  char *__builtin_strstrtemp;
  return __builtin_strstrtemp;
}

int memcmp(const void *s1, const void *s2, size_t n) __THROW
{
  return 0;
}

size_t strlen(const char *s) __THROW
{
  size_t strlentemp;
  return strlentemp;
}

void *memmove(void *dest, const void *src, size_t n) __THROW
{
	return NULL;
}

void *memcpy(void *dest, const void *src, size_t n) __THROW
{
	return NULL;
}

void *memset(void *s, int c, size_t n) __THROW
{
	return NULL;
}

int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n) __THROW
{
	return 0;
}

size_t wcslen(const wchar_t *s) __THROW
{
	size_t wcslentemp;
	return wcslentemp;
}


wchar_t *wmemmove(wchar_t *dest, const wchar_t *src, size_t n) __THROW
{
	wchar_t *wmemmovetemp;
	return wmemmovetemp;
}



wchar_t *wmemcpy(wchar_t *dest, const wchar_t *src, size_t n) __THROW
{
	wchar_t *wmemcpy;
	return wmemcpy;
}


wchar_t *wmemset(wchar_t *wcs, wchar_t wc, size_t n) __THROW
{
	wchar_t *wmemset;
	return wmemset;
}

long int labs(long int j) __THROW
{
	return 0;
}

ldiv_t ldiv(long int numer, long int denom) __THROW
{
	ldiv_t ldivtemp;
	return ldivtemp;
}

void *operator new(size_t size) throw (std::bad_alloc)
{
}

void operator delete(void *obj, size_t size)
{
}

_Atomic_word __exchange_and_add(volatile _Atomic_word* __mem, int __val)
{
	_Atomic_word __exchange_and_addtemp;
	return __exchange_and_addtemp;
}

int strcmp(const char *s1, const char *s2) __THROW
{
	return 0;
}

char *strncpy(char *dest, const char *src, size_t n) __THROW
{
	char *strncpytemp;
	return strncpytemp;
}

int iconv_close(iconv_t cd)
{
	return 0;
}


iconv_t iconv_open(const char *tocode, const char *fromcode)
{
	iconv_t iconv_opentemp;
	return iconv_opentemp;
}

void __throw_runtime_error(const char*)
{
}

double __builtin_fabs(double x)
{
	return 0;
}

float __builtin_fabsf(float x)
{
	return 0;
}

long double __builtin_fabs1(long double x)
{
	return 0;
}

float acosf(float x) __THROW
{
  return 0;
}

long double acosl(long double x) __THROW
{
  return 0;
}

float asinf(float x) __THROW
{
  return 0;
}

long double asinl(long double x) __THROW
{
  return 0;
}

float atanf(float x) __THROW
{
  return 0;
}
long double atanl(long double x) __THROW
{
  return 0;
}

float atan2f(float y, float x) __THROW
{
  return 0;
}

long double atan2l(long double y, long double x) __THROW
{
  return 0;
}

float ceilf(float x) __THROW
{
	return 0;
}

long double ceill(long double x) __THROW
{
	return 0;
}

float __builtin_cosf(float x)
{
  return 0;
}

long double __builtin_cosl(long double x)
{
  return 0;
}

float coshf(float x) __THROW
{
  return 0;
}

long double coshl(long double x) __THROW
{
  return 0;
}

float expf(float x) __THROW
{
  return 0;
}

long double expl(long double x) __THROW
{
  return 0;
}

float floorf(float x) __THROW
{
	return 0;
}

long double floorl(long double x) __THROW
{
	return 0;
}

float fmodf(float x, float y) __THROW
{
  return 0;
}

long double fmodl(long double x, long double y) __THROW
{
  return 0;
}

float frexpf(float num, int *exp) __THROW
{
  return 0;
}

long double frexpl(long double num, int *exp) __THROW
{
  return 0;
}

float ldexpf(float x, int exp) __THROW
{
  return 0;
}

long double ldexpl(long double x, int exp) __THROW
{
  return 0;
}

float logf(float x) __THROW
{
  return 0;
}

long double logl(long double x) __THROW
{
  return 0;
}

float log10f(float x) __THROW
{
  return 0;
}

long double log10l(long double x) __THROW
{
  return 0;
}

float modff(float value, float *iptr) __THROW
{
  return 0;
}

long double modfl(long double value, long double *iptr) __THROW
{
  return 0;
}

float powf(float x, float y) __THROW
{
  return 0;
}

long double powl(long double x, long double y) __THROW
{
  return 0;
}

float __builtin_sinf(float x)
{
  return 0;
}

long double __builtin_sinl(long double x)
{
  return 0;
}

float sinhf(float x) __THROW
{
  return 0;
}

long double sinhl(long double x) __THROW
{
  return 0;
}

float __builtin_sqrtf(float x)
{
  return 0;
}

long double __builtin_sqrtl(long double x)
{
  return 0;
}

float tanf(float x) __THROW
{
  return 0;
}

long double tanl(long double x) __THROW
{
  return 0;
}

float tanhf(float x) __THROW
{
  return 0;
}

long double tanhl(long double x) __THROW
{
  return 0;
}

float __builtin_nansf (const char *str)
{
  return 0;
}

double __builtin_nans (const char *str)
{
 return 0;
}

long double __builtin_nansl (const char *str)
{
  return 0;
}

char *strcpy(char *dest, const char *src) __THROW
{
  char *strcpytemp;
  return strcpytemp;
}

// Missing SgTemplateInstantiationFunctionDecl [referenced]

