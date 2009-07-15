                                       // Chapter 11 - Program 4
#include <iostream>
#include "supervsr.h"

using namespace std;

// In all cases, init_data assigns values to the class variables and
//  display outputs the values to the monitor for inspection.

supervisor::supervisor(char *in_name, int in_salary, char *in_title)
{
   name = in_name;
   salary = in_salary;
   title= in_title;
}




void
supervisor::display(void)
{
   cout << "Supervisor --> " << name << "'s salary is " << salary <<
                                 " and is the " << title << ".\n\n";
}




programmer::programmer(char *in_name, int in_salary, char *in_title,
                  char *in_language)
{
   name = in_name;
   salary = in_salary;
   title = in_title;
   language = in_language;
}




void
programmer::display(void)
{
   cout << "Programmer --> " << name << "'s salary is " << salary <<
                                        " and is " << title << ".\n";
   cout << "               " << name << "'s specialty is " <<
                                                 language << ".\n\n";
}




secretary::secretary(char *in_name, int in_salary,
                             int in_shorthand, int in_typing_speed)
{
   name = in_name;
   salary = in_salary;
   shorthand = in_shorthand;
   typing_speed = in_typing_speed;
}




void
secretary::display(void)
{
   cout << "Secretary ---> " << name << "'s salary is " << salary <<
                                                                 ".\n";
   cout << "               " << name << " types " << typing_speed <<
              " per minute and can ";
   if (!shorthand) cout << "not ";
   cout << "take shorthand.\n\n";
}

consultant::consultant(char *in_name, 
		      int in_salary,
		      char *in_specialty, 
		      int in_contract_length)
{
   name = in_name;
   salary = in_salary;
   specialty = in_specialty;
   contract_length = in_contract_length;
}




void
consultant::display(void)
{
   cout << "Consultant --> " << name << "'s salary is " << salary <<
                                      " and consults in " <<
					  specialty << ".\n"; 
   cout << "               " << name << "'s contract lasts " <<
                                                 contract_length << 
						   " weeks.\n\n"; 
}

                                     // Chapter 11 - Program 5

person *staff1,*staff2,*staff3,*staff4;

main()
{
supervisor *suppt;
programmer *progpt;
secretary *secpt;

   suppt = new supervisor("Big John", 5100, "President");
   staff1 = suppt;

   progpt = new programmer("Joe Hacker", 3500, "debugger", "Pascal");
   staff2 = progpt;

   progpt = new programmer("OOP Wizard", 7700, "senior analyst", "C++");
   staff3 = progpt;

   secpt = new secretary("Tillie Typer", 2200, 1, 85);
   staff4 = secpt;

   staff1->display();
   staff2->display();
   staff3->display();
   staff4->display();
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

pthread_once_t once_control = PTHREAD_ONCE_INIT;
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
	return NULL;		// more semantic
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


