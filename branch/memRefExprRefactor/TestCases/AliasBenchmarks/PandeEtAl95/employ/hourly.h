/* hourly.h                                                         -*- C++ -*-
**    Include file for Hourly class
** 
** COPYRIGHT (C) 1994 Bradley M. Kuhn
**
** Written   :   Bradley M. Kuhn                                 Loyola College
**   By
**
** Written   :   David W. Binkley                                Loyola College
**   For         
**
** Acknowledgements:
**    This code is based on code that appears in:
**       C++ How to Program by H. M. Deitel and P. J. Deitel
**         Prentice Hall, New Jersey, p. 536
**
** RCS       :
**
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/hourly.h $
** $Revision: 0.2 $
** $Date: 1994/12/31 01:22:16 $
**
** $Log: hourly.h $
** Revision 0.2  1994/12/31  01:22:16  bkuhn
**   -- version were getting data from
**
** Revision 0.1  1994/12/24  01:43:50  bkuhn
**   # initial version
**
**
*/

#ifndef _HOURLY_H
#define _HOURLY_H

#include "wage.h"

#define HOURLY_ID 5
#include <iostream.h>
#include <stdlib.h>

/* An hourly worker gets paid for every hour worked */

class HourlyWorker : public WageWorker {
  private:
    float  thisWeekHours;             // hours worked this week

  protected:
    float ThisWeekHours()                 { return thisWeekHours; };
    void  SetThisWeekHours(float);

  public:
    HourlyWorker(const char *, const char * , float = 0.0);

    virtual void  Print();
    virtual void  NewWeek();
    virtual void  Raise(int);

    // pure virtual function
    virtual float Earnings() = 0;
};
/*****************************************************************************/
HourlyWorker::HourlyWorker(const char *first, const char *last,
                           float startWage) 
 : WageWorker(first, last, startWage)    // this will call Wage's constructor
{
    dollarsToRaise = 0.5;
    thisWeekHours = 0.0;
}
/*****************************************************************************/
void
HourlyWorker::SetThisWeekHours(float hours) 
{
    thisWeekHours = hours;
}
/*****************************************************************************/
void
HourlyWorker::Print() {
    cout << "      Hourly Worker: " << FirstName() << ' ' << LastName();
}
/*****************************************************************************/
void
HourlyWorker::Raise(int units)
{
    if (units > 0)
      SetWage(Wage() + (units * dollarsToRaise));
}
/*****************************************************************************/
void
HourlyWorker::NewWeek()
{
    float hours;

    hours = 44; // ( float(rand()) / float(RAND_MAX) ) * 80.0;

    SetThisWeekHours(hours);
}

#endif
