// Chapter 11 - Program 4
#include <iostream>
#include "supervsr.h"
using namespace std;
// In all cases, init_data assigns values to the class variables and
//  display outputs the values to the monitor for inspection.

supervisor::supervisor(char *in_name,int in_salary,char *in_title)
{
  (this) -> name = in_name;
  (this) -> salary = in_salary;
  (this) -> title = in_title;
}


void supervisor::display()
{
  ( *((&std::cout))<<"Supervisor --> "<<(((this) -> name))<<"\'s salary is ")<<((this) -> salary)<<" and is the "<<(((this) -> title))<<".\n\n";
}


programmer::programmer(char *in_name,int in_salary,char *in_title,char *in_language)
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


secretary::secretary(char *in_name,int in_salary,int in_shorthand,int in_typing_speed)
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


consultant::consultant(char *in_name,int in_salary,char *in_specialty,int in_contract_length)
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
