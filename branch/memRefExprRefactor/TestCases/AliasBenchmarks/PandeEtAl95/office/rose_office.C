// Chapter 11 - Program 4
#include <iostream>
#include "supervsr.h"
using namespace std;
// In all cases, init_data assigns values to the class variables and
//  display outputs the values to the monitor for inspection.

supervisor::supervisor(char *in_name,int in_salary,char *in_title) : person()
{
  (this) -> name = in_name;
  (this) -> salary = in_salary;
  (this) -> title = in_title;
}


void supervisor::display()
{
  ( *((&std::cout))<<"Supervisor --> "<<(((this) -> name))<<"\'s salary is ")<<((this) -> salary)<<" and is the "<<(((this) -> title))<<".\n\n";
}


programmer::programmer(char *in_name,int in_salary,char *in_title,char *in_language) : person()
{
  (this) -> name = in_name;
  (this) -> salary = in_salary;
  (this) -> title = in_title;
  (this) -> language = in_language;
}


void programmer::display()
{
  ( *((&std::cout))<<"Programmer --> "<<(((this) -> name))<<"\'s salary is ")<<((this) -> salary)<<" and is "<<(((this) -> title))<<".\n";
   *((&std::cout))<<"               "<<(((this) -> name))<<"\'s specialty is "<<(((this) -> language))<<".\n\n";
}


secretary::secretary(char *in_name,int in_salary,int in_shorthand,int in_typing_speed) : person()
{
  (this) -> name = in_name;
  (this) -> salary = in_salary;
  (this) -> shorthand = (in_shorthand);
  (this) -> typing_speed = in_typing_speed;
}


void secretary::display()
{
  ( *((&std::cout))<<"Secretary ---> "<<(((this) -> name))<<"\'s salary is ")<<((this) -> salary)<<".\n";
  ( *((&std::cout))<<"               "<<(((this) -> name))<<" types ")<<((this) -> typing_speed)<<" per minute and can ";
  if (!(((this) -> shorthand))) {
     *((&std::cout))<<"not ";
  }
  else {
  }
   *((&std::cout))<<"take shorthand.\n\n";
}


consultant::consultant(char *in_name,int in_salary,char *in_specialty,int in_contract_length) : person()
{
  (this) -> name = in_name;
  (this) -> salary = in_salary;
  (this) -> specialty = in_specialty;
  (this) -> contract_length = in_contract_length;
}


void consultant::display()
{
  ( *((&std::cout))<<"Consultant --> "<<(((this) -> name))<<"\'s salary is ")<<((this) -> salary)<<" and consults in "<<(((this) -> specialty))<<".\n";
  ( *((&std::cout))<<"               "<<(((this) -> name))<<"\'s contract lasts ")<<((this) -> contract_length)<<" weeks.\n\n";
}

// Chapter 11 - Program 5
class person *staff1;
class person *staff2;
class person *staff3;
class person *staff4;

int main()
{
  class supervisor *suppt;
  class programmer *progpt;
  class secretary *secpt;
  suppt = (new supervisor (("Big John"),5100,("President")));
  staff1 = (suppt);
  progpt = (new programmer (("Joe Hacker"),3500,("debugger"),("Pascal")));
  staff2 = (progpt);
  progpt = (new programmer (("OOP Wizard"),7700,("senior analyst"),("C++")));
  staff3 = (progpt);
  secpt = (new secretary (("Tillie Typer"),2200,1,85));
  staff4 = (secpt);
  staff1 -> display();
  staff2 -> display();
  staff3 -> display();
  staff4 -> display();
}

// Result of execution
// XYZ Staff -- note salary is monthly.
//
// Supervisor --> Big John's salary is 5100 and is the President.
//
// Programmer --> Joe Hacker's salary is 3500 and is debugger.
//                Joe Hacker's specialty is Pascal.
//
// Programmer --> OOP Wizard's salary is 7700 and is senior analyst.
//                OOP Wizard's specialty is C++.
//
// Secretary ---> Tillie Typer's salary is 2200.
//                Tillie typer types 85 per minute and can take shorthand.
//
// Supervisor --> Tom Talker's salary is 5430 and is the sales manager.
//
// Programmer --> Dave Debugger's salary is 5725 and is code maintainer.
//                Dave Debugger's specialty is assembly language.
//
// End of employee list.
// stubs
pthread_once_t once_control = (0);

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
// more semantic
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
