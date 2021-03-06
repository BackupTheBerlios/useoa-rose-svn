/* commission.h                                                     -*- C++ -*-
**    Include file for Commission Worker class
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
**         Prentice Hall, New Jersey, pp. 534-5
**
** RCS       :
**
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/commission.h $
** $Revision: 0.2 $
** $Date: 1994/12/31 01:21:59 $
**
** $Log: commission.h $
** Revision 0.2  1994/12/31  01:21:59  bkuhn
**   -- version were getting data from
**
** Revision 0.1  1994/12/28  01:36:49  bkuhn
**   # initial version
**
**
*/

#ifndef _COMMISSION_H
#define _COMMISSION_H

#include "employee.h"

#include <iostream.h>
#include <stdlib.h>

#define COMMISSIONED_WORKER_ID 2
/* A commissioned worker gets a weekly salary + commission per quantity sold */

class CommissionedWorker : public Employee {
  private:
    float        weeklySalary;         // base salary
    float        commissionRate;       // % commission made on weekly sales
    float        thisWeekSales;        // total sales this week

  protected:
    void          SalesThisWeek(float);

  public:
    CommissionedWorker(const char *, const char * , float = 0.0, float = 0.0);

    void          SetWeeklySalary(float);
    void          SetCommissionRate(float);
    virtual float Earnings();
    virtual void  Print();
    virtual void  Raise(int);
    virtual void  NewWeek();
};
/*****************************************************************************/
CommissionedWorker::CommissionedWorker(const char *first, const char *last,
                                     float startSalary, float startCommission) 
 : Employee(first, last)        // this will call Employee's constructor
{
    SetWeeklySalary(startSalary);
    SetCommissionRate(startCommission);
    dollarsToRaise = 10.0;
}
/*****************************************************************************/
void
CommissionedWorker::SetWeeklySalary(float newSalary) 
{
    weeklySalary = (newSalary > 0.0) ? newSalary : 0.0;
}
/*****************************************************************************/
void
CommissionedWorker::SetCommissionRate(float newRate) 
{
    if (newRate < 0.0)
      commissionRate = 0.0;
    else if (newRate > 60.0)
      commissionRate = 60.0;
    else
      commissionRate = newRate;
}
/*****************************************************************************/
void
CommissionedWorker::SalesThisWeek(float sales) 
{
    thisWeekSales = sales;
}
/*****************************************************************************/
float
CommissionedWorker::Earnings() {
    return weeklySalary + (commissionRate / 100.0) * thisWeekSales;
}
/*****************************************************************************/
void
CommissionedWorker::Print() {
    cout << "Commissioned Worker: " << FirstName() << ' ' << LastName();
}
/*****************************************************************************/
void
CommissionedWorker::Raise(int units)
{
    if (units > 0) {
        SetCommissionRate(commissionRate + units);
        weeklySalary += units * dollarsToRaise;
    }
}
/*****************************************************************************/
void
CommissionedWorker::NewWeek()
{
    int quantity = 5; // rand() % 5;

    SalesThisWeek(quantity * 5000.0);
}

#endif
