/* wage.h                                                           -*- C++ -*-
**    Include file for Wage Worker base class
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
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/wage.h $
** $Revision: 0.1 $
** $Date: 1994/12/24 01:45:27 $
**
** $Log: wage.h $
** Revision 0.1  1994/12/24  01:45:27  bkuhn
**   # initial version
**
**
*/

#ifndef _WAGE_H
#define _WAGE_H

#include "employee.h"

#define WAGE_WORKER_ID 3

#include <iostream.h>
#include <stdlib.h>

/* A wage worker  gets paid for every (item, hour, etc) worked/produced */

class WageWorker : public Employee {
  private:
    float        wage;           // wage per thing

  protected:
    float Wage();

  public:
    WageWorker(const char *, const char * , float = 0.0);

    void          SetWage(float);
    virtual void  Raise(int);

    // pure virtual functions

    virtual float Earnings() = 0;
    virtual void  Print()    = 0;
    virtual void  NewWeek()  = 0;
};
/*****************************************************************************/
WageWorker::WageWorker(const char *first, const char *last,
                                     float startWage)
 : Employee(first, last)        // this will call Employee's constructor
{
    SetWage(startWage);
}
/*****************************************************************************/
void
WageWorker::SetWage(float newWage) 
{
    wage = (newWage > 0.0) ? newWage : 0.0;
}
/*****************************************************************************/
float
WageWorker::Wage() 
{
    return wage;
}
/*****************************************************************************/
void
WageWorker::Raise(int units)
{
    if (units > 0)
      wage += units * dollarsToRaise;
}

#endif
