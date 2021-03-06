/* this is file class-defs.h provided to students */

//extern "C" {
//char *strcpy(char *nam, char *str);
//}
#include <string.h>
typedef char *strng;

/* class expr is the top of the class heirarchy for expressions. Any
   expression object will be an instance of some derived class which has
   class expr as its base class (or as the base class of its base class...)
*/
class expr {
 public:
  expr(){}
  ~expr(){}
  virtual void print_me()     = 0;

     /* This is the function that an expression object uses to print itself.
      The version given here should never be run - the print function should
      be redefined in the derived class */

  virtual expr *deriv(char *var)  = 0;
     /* This is the function that an expression object uses to compute its
      derivative with respect to "var". The version given here should never
      be run - deriv() should be redefined in the derived class */

  virtual double eval(double at)  = 0;
   /* This is the function that an expression object uses to compute its
      value when all variables in it are replaced by the value of "at"
      */

};
     
     
/* an expression which is a numerical constant */
class const_expr : public expr {
  double value;
 public:
  const_expr(double v) {value = v;}
  ~const_expr(){}
  void print_me() {} // {printf(" %d ", value);}
  double eval(double at);
  expr *deriv(strng var);
 };

/* an expression which is a single variable */
class var_expr : public expr {
private:
  char *name;
 public:
  var_expr(strng str) {strcpy(name,str);}
			/* Copy the initialization string
                       into the name buffer. To read the documentation for
                       strcpy() and other C string manipulation functions,
                       type the Unix shell command "man string" */
  ~var_expr(){}
  void print_me()  {} // {printf(" %s ", name);}
  double eval(double at);
  expr *deriv(strng var);
 };

/* expression resulting from applying a binary operator to two expressions */
class bin_op_expr : public expr {
 public:
  bin_op_expr(expr *e1, expr *e2) {first = e1; second = e2;}
  ~bin_op_expr(){}
  void print_me();
 protected:
  expr *first;
  expr *second;
  char op_name;
 };

/* an expression which is the sum of two expressions */
class sum_expr : public bin_op_expr {
 public:
  sum_expr(expr *e1, expr *e2) : bin_op_expr(e1,e2) {op_name = '+';}
  ~sum_expr(){}
  double eval(double at);
  expr *deriv(strng var);
 };

/* an expression which is the product of two expressions */
class prod_expr : public bin_op_expr {
 public:
  prod_expr(expr *e1, expr *e2) : bin_op_expr(e1,e2) {op_name = '*';}
  ~prod_expr(){}
  double eval(double at);
  expr *deriv(strng var);
 };

/* an expression which is the quotient of two expressions */
class quotient_expr : public bin_op_expr {
 public:
  quotient_expr(expr *e1, expr *e2) : bin_op_expr(e1,e2) {op_name = '/';}
  ~quotient_expr(){}
  double eval(double at);
  expr *deriv(strng var);
 };
// Written for undergraduate C++ course project at Dept of
// Computer Science, Rutgers University.

#include <iostream>
#include <string.h>

using namespace std;

void bin_op_expr::print_me()
{
  cout << "(";
  first->print_me();
  cout << op_name;
  second->print_me();
  cout << ")";
}

double const_expr::eval(double at)
{
  return value;
}

expr *const_expr::deriv(strng var)
{
  return (new const_expr(0));
}

double var_expr::eval(double at)
{
  return at;
}

expr *var_expr::deriv(strng var)
{
  if (1) return (new const_expr(1)); // was strcmp between var and name
  else return (new var_expr(name));
  
}

double prod_expr::eval(double at)
{
  return (first->eval(at) * second->eval(at));
}

expr *prod_expr::deriv(strng var)
{
  return new sum_expr(new prod_expr(first, second->deriv(var)),new prod_expr(second, first->deriv(var)));
}

double sum_expr::eval(double at)
{
  return (first->eval(at) + second->eval(at));
}

expr *sum_expr::deriv(strng var)
{
expr *f;
  return new sum_expr(f=first->deriv(var), second->deriv(var));
}

double quotient_expr::eval(double at)
{
  return (first->eval(at) / second->eval(at));
}

expr *quotient_expr::deriv(strng var)
{
  return new quotient_expr(new sum_expr(new prod_expr(second, first->deriv(var)), new prod_expr(new const_expr(-1), new prod_expr(first, second->deriv(var)))), new prod_expr(second, second));
}

main() {
 const_expr c(8);
 var_expr x("x");
 prod_expr simple(new const_expr(123.45), new var_expr("y"));

 cout << "c is ";
 c.print_me();
 cout << "\n      and its value at 3 is: " <<
 c.eval(3);
 cout << "\n      and its derivative with respect to x is: ";
 c.deriv("x")->print_me();
 cout << "\nx is ";
 x.print_me();
 cout << "\n      and its value at 3 is: " <<
 x.eval(3);
 cout << "\n      and its derivative with respect to x is: ";
 x.deriv("x")->print_me();
 cout << "\nsimple is ";
 simple.print_me();
 cout << "\n     and its value at 3 is: " <<
 simple.eval(3);
 cout << "\n     and its derivative with respect to y is: ";
 simple.deriv("y")->print_me();
 cout << "\n     and its derivative with respect to x is: ";
 simple.deriv("x")->print_me();
 }


//Stubs
int    pthread_once(pthread_once_t *once_control, void (*init_routine) (void)) __THROW
{
  return 0;
}

int     pthread_key_create(pthread_key_t *key, void (*destr_function) (void *)) __THROW
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
// Return void * key? can also return null but only returns on error,
// normally returns the value associated with key on success? what to
// put here?
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
  // Returning value before it is set, giving warning, problem?
}

wchar_t *wcspbrk(const wchar_t *wcs, const wchar_t *accept) __THROW
{
  wchar_t *wcspbrktemp;
  return wcspbrktemp;
  // Returning value before it is set, giving warning, problem?
}

wchar_t *wcsrchr(const wchar_t *wcs, wchar_t wc) __THROW
{
  wchar_t *wcsrchrtemp;
  return wcsrchrtemp;
  // Returning value before it is set, giving warning, problem?
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
  // Returning value before it is set, giving warning, problem?
}

void *memchr(const void *s, int c, size_t n) __THROW
{
// Return void *? can also return null but only returns on error,
// normally supposed to return pointer to the matching byte
}

char *__builtin_strchr(char const *str, int ch)
{
  char *__builtin_strchrtemp;
  return __builtin_strchrtemp;
  // Returning value before it is set, giving warning, problem?
  // Not in man library, got information at:
  // http://www.cs.berkeley.edu/~smcpeak/elkhound/sources/elsa/gnu.cc
}

char *__builtin_strpbrk(char const *str, char const *accept)
{
  char *__builtin_strpbrktemp;
  return __builtin_strpbrktemp;
  // Returning value before it is set, giving warning, problem?
  // Not in man library, got information at:
  // http://www.cs.berkeley.edu/~smcpeak/elkhound/sources/elsa/gnu.cc
}

char *__builtin_strrchr(char const *str, int ch)
{
  char *__builtin_strrchrtemp;
  return __builtin_strrchrtemp;
  // Returning value before it is set, giving warning, problem?
  // Not in man library, got information at:
  // http://www.cs.berkeley.edu/~smcpeak/elkhound/sources/elsa/gnu.cc
}

char *__builtin_strstr(char const *haystack, char const *needle)
{
  char *__builtin_strstrtemp;
  return __builtin_strstrtemp;
  // Returning value before it is set, giving warning, problem?
  // Not in man library, got information at:
  // http://www.cs.berkeley.edu/~smcpeak/elkhound/sources/elsa/gnu.cc
}

int memcmp(const void *s1, const void *s2, size_t n) __THROW
{
  return 0;
}

size_t strlen(const char *s) __THROW
{
  size_t strlentemp;
  return strlentemp;
  // Returning value before it is set, giving warning, problem?
}

 void *memmove(void *dest, const void *src, size_t n) __THROW
{
// Return void *memmove, returns pointer to dest?
}

void *memcpy(void *dest, const void *src, size_t n) __THROW
{
  // Return void *memcpy, returns pointer to dest?
}

void *memset(void *s, int c, size_t n) __THROW
{
  // Return void *memset, returns pointer to s?
}

int wmemcmp(const wchar_t *s1, const wchar_t *s2, size_t n) __THROW
{
  return 0;
}

size_t wcslen(const wchar_t *s) __THROW
{
  size_t wcslentemp;
  return wcslentemp;
  // Returning value before it is set, giving warning, problem?
}

wchar_t *wmemmove(wchar_t *dest, const wchar_t *src, size_t n) __THROW
{
  wchar_t *wmemmovetemp;
  return wmemmovetemp;
  // Returning value before it is set, giving warning, problem?
}

wchar_t *wmemcpy(wchar_t *dest, const wchar_t *src, size_t n) __THROW
{
  wchar_t *wmemcpytemp;
  return wmemcpytemp;
  // Returning value before it is set, giving warning, problem?
}

wchar_t *wmemset(wchar_t *wcs, wchar_t wc, size_t n) __THROW
{
  wchar_t *wmemsettemp;
  return wmemsettemp;
  // Returning value before it is set, giving warning, problem?
}

long int labs(long int j) __THROW
{
  return 0;
}

ldiv_t ldiv(long int numer, long int denom) __THROW
{
  ldiv_t ldivtemp;
  return ldivtemp;
  // Returning value before it iset , giving warning, problem?
}

void *operator new(size_t size) throw (std::bad_alloc)
{ 
  // Returning void *operator new, what does this return?
  // Not in man library, got information at:
  // http://www.utexas.edu/its/unix/reference/oracledocs/v92/B10501_01/appdev.920/a96583/cci08r15.htm
}

void operator delete(void *obj, size_t size)
{
  // Not in man library, got information at:
  // http://www.utexas.edu/its/unix/reference/oracledocs/v92/B10501_01/appdev.920/a96583/cci08r15.htm
}

// Missing         method: qualified name: std::locale::operator==
// type signature: public: bool operator==(const class std::locale
// &__other) const throw(); [referenced]

// Missing         method: qualified name: std::locale::classic type
// signature: public: static const std::locale &classic();
// [referenced]
#if 0
void __atomic_add(volatile _Atomic_word* __mem, int __val)
{
  // Not in man libray, got information at:
  //http://www.nabble.com/notes-on-current-atomics-config-and-usage-t1902803.html
}

 _Atomic_word __exchange_and_add(volatile _Atomic_word* __mem, int __val)
{
  _Atomic_word __exchange_and_addtemp;
  return __exchange_and_addtemp;
  // Returning value before it is set, giving warning, problem?
  // Not in man library, got information at:
  //http://www.nabble.com/notes-on-current-atomics-config-and-usage-t1902803.html
}
#endif

int strcmp(const char *s1, const char *s2) __THROW
{
  return 0;
}

// Missing         method: qualified name: std::locale::locale type
// signature: public: locale(const class std::locale &__other)
// throw(); [referenced]

// Missing         method: qualified name:
// std::ios_base::_M_grow_words type signature: protected:
// std::ios_base::_Words &_M_grow_words(int __index); [referenced]

char *strncpy(char *dest, const char *src, size_t n) __THROW
{
  char *strncpytemp;
  return strncpytemp;
  // Returning value before it is set, giving warning, problem?
}

int iconv_close(iconv_t cd)
{
  return 0;
}


iconv_t iconv_open(const char *tocode, const char *fromcode)
{
  iconv_t iconv_opentemp;
  return iconv_opentemp;
  // Returning value before it is set, giving warning, problem?
}

void __throw_runtime_error(const char*)
{
  // Found many instances of this online, wasn't in man, may not be right
}

double __builtin_fabs(double x)
{
  return 0;
}

float __builtin_fabsf(float x)
{
  return 0;
}

long double __builtin_fabsl(long double x)
{
  return 0;
}

// ASSUMING (function)f implies float (function)l implies long double
// after seeing several refernces online about it
// Main site used for math functions:
// http://www.opengroup.org/onlinepubs/000095399/functions/atan2.html


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
  // Not in man library, got information at:
  //http://docs.biostat.wustl.edu/cgi-bin/info2html?(gcc.info.gz)Other%2520Builtins
}

double __builtin_nans (const char *str)
{
 return 0;
  // Not in man library, got information at:
  //http://docs.biostat.wustl.edu/cgi-bin/info2html?(gcc.info.gz)Other%2520Builtins
}

long double __builtin_nansl (const char *str)
{
  return 0;
  // Not in man library, got information at:
  //http://docs.biostat.wustl.edu/cgi-bin/info2html?(gcc.info.gz)Other%2520Builtins
}

char *strcpy(char *dest, const char *src) __THROW
{
  char *strcpytemp;
  return strcpytemp;
  // Returning value before it is set, giving warning, problem?
}

// Missing SgTemplateInstantiationFunctionDecl [referenced]

// Missing method: qualified name: expr::print_me type signature:
// public: virtual void print_me() = 0; [referenced]

// Missing SgTemplateInstantiationFunctionDecl [referenced]

// Missing method: qualified name: expr::eval type signature: public:
// virtual double eval(double at) = 0; [referenced]

// Missing method: qualified name: expr::deriv type signature: public:
// virtual expr *deriv(char *var) = 0; [referenced]

// Missing SgTemplateInstantiationMemberFunctionDecl [referenced]
