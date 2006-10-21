/* employee.h                                                       -*- C++ -*-
**    Include file for abstract employee base class
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
**         Prentice Hall, New Jersey, pp. 531-2
**
** RCS       :
**
** $Source: /home/bkuhn/CURRENT/fbc/c++-opt/EMPLOYEE-EXAMPLE/NON-OPT/RCS/employee.h $
** $Revision: 0.1 $
** $Date: 1994/12/24 01:26:08 $
**
** $Log: employee.h $
** Revision 0.1  1994/12/24  01:26:08  bkuhn
**   # initial version
**
**
*/

#ifndef _EMPLOYEE_H
#define _EMPLOYEE_H

#include <iostream.h>
#include <iomanip.h>

#define EMPLOYEE_ID 0

class Employee {

  private:
    char * firstName;
    char * lastName;

  protected:
    float  dollarsToRaise;      // the dollar value added to salary when raised

  public:
    Employee(const char *, const char *);
    virtual ~Employee();

    const char * FirstName();
    const char * LastName();
          void   PrintWithEarnings(int);

    // pure virtual functions

    virtual float Earnings() = 0;
    virtual void  Print()    = 0;
    virtual void  Raise(int) = 0;
    virtual void  NewWeek()  = 0;
};
/*****************************************************************************/
Employee::Employee(const char * first, const char * last)
{
    firstName = new char [ strlen(first) + 1 ];
    strcpy(firstName, first);

    lastName = new char [ strlen(last) + 1 ];
    strcpy(lastName, last);
}
/*****************************************************************************/
Employee::~Employee()
{
    delete [] firstName;
    delete [] lastName;
}
/*****************************************************************************/
const char *
Employee::FirstName()
{

    return firstName; // caller must make a copy
}
/*****************************************************************************/
const char *
Employee::LastName()
{
    return lastName; // caller must make a copy
}
/*****************************************************************************/
void
Employee::PrintWithEarnings(int weekNumber)
{
    this->Print();

    cout << " earned $" << this->Earnings() << " in week "
         << weekNumber << '\n';
}

#endif
/* boss.h                                                         -*- C++ -*-
**    Include file for Boss class
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
**         Prentice Hall, New Jersey, pp. 533-4
**
** RCS       :
**
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/boss.h $
** $Revision: 0.1 $
** $Date: 1994/12/24 00:35:59 $
**
** $Log: boss.h $
** Revision 0.1  1994/12/24  00:35:59  bkuhn
**   # initial version
**
** Revision 0.1  1994/12/24  00:35:59  bkuhn
**   # initial version
**
**
*/

#ifndef _BOSS_H
#define _BOSS_H

#define BOSS_ID 1

#include <iostream.h>

/* A boss gets a weekly salary, regardless of how much (s)he works */

class Boss : public Employee {
  private:
    float weeklySalary;

  public:
    Boss(const char *, const char * , float = 0.0);

    void          SetWeeklySalary(float);
    virtual float Earnings();
    virtual void  Print();
    virtual void  Raise(int);
    virtual void  NewWeek();
};
/*****************************************************************************/
Boss::Boss(const char *first, const char *last, float startSalary) 
 : Employee(first, last)        // this will call Employee's constructor
{
    SetWeeklySalary(startSalary);
    dollarsToRaise = 100.0;
}
/*****************************************************************************/
void
Boss::SetWeeklySalary(float newSalary) 
{
    weeklySalary = (newSalary > 0.0) ? newSalary : 0.0;
}
/*****************************************************************************/
float
Boss::Earnings() {
    return weeklySalary;
}
/*****************************************************************************/
void
Boss::Print() {
    cout << "               Boss: " << FirstName() << ' ' << LastName();
}
/*****************************************************************************/
void
Boss::Raise(int units)
{
    if (units > 0)
      weeklySalary += units * dollarsToRaise;
}
/*****************************************************************************/
void
Boss::NewWeek()
{
    return;
}
#endif
/* company.h                                                        -*- C++ -*-
**    Include file for a company
** 
** COPYRIGHT (C) 1994 Bradley M. Kuhn
**
** Written   :   Bradley M. Kuhn                                 Loyola College
**   By
**
** Written   :   David W. Binkley                                Loyola College
**   For         
**
** RCS       :
**
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/company.h $
** $Revision: 0.1 $
** $Date: 1994/12/24 00:39:43 $
**
** $Log: company.h $
** Revision 0.1  1994/12/24  00:39:43  bkuhn
**   # initial version
**
**
*/

#ifndef _COMPANY_H
#define _COMPANY_H

#include <iostream.h>

class EmployeeNode {

  private:
    Employee *     employee;
    EmployeeNode * next;
    
  public:
    EmployeeNode(Employee *e, EmployeeNode *n) {
        employee = e;
        next = n;
    }

    Employee * Employee() { return employee; }
    EmployeeNode *  Next();
};
/*****************************************************************************/
EmployeeNode *
EmployeeNode::Next()
{
    return next;
}
/*****************************************************************************/

class Company {
  private:
    EmployeeNode * employeeList;
    int            employeeCount;
    int            currentWeek;

  public:
    Company();

    void          AddEmployee(Employee *);
     int          EmployeeCount();

    void          PrintWithEarnings();
    void          NewWeek();
    void          AcrossTheBoardRaise(int);
};
/*****************************************************************************/
Company::Company()
{
    employeeList  = NULL;
    employeeCount = 0;
    currentWeek   = 0;
}
/*****************************************************************************/
int
Company::EmployeeCount()
{
    return employeeCount;
}
/*****************************************************************************/
void
Company::AddEmployee(Employee * e)
{
    EmployeeNode * newNode;

    newNode = new EmployeeNode(e, employeeList);
    employeeList = newNode;
}
/*****************************************************************************/
void
Company::PrintWithEarnings()
{
    EmployeeNode * curE;

    for(curE = employeeList; curE != NULL; curE = curE->Next())
      curE->Employee()->PrintWithEarnings(currentWeek);
}
/*****************************************************************************/
void
Company::NewWeek()
{
    EmployeeNode * curE;

    currentWeek++;

    for(curE = employeeList; curE != NULL; curE = curE->Next())
      curE->Employee()->NewWeek();
}
/*****************************************************************************/
void
Company::AcrossTheBoardRaise(int units)
{
    EmployeeNode * curE;

    for(curE = employeeList; curE != NULL; curE = curE->Next())
      curE->Employee()->Raise(units);
}

#endif
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
/* piece.h                                                          -*- C++ -*-
**    Include file for Piece Worker class
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
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/piece.h $
** $Revision: 0.2 $
** $Date: 1994/12/31 01:22:21 $
**
** $Log: piece.h $
** Revision 0.2  1994/12/31  01:22:21  bkuhn
**   -- version were getting data from
**
** Revision 0.1  1994/12/24  00:48:37  bkuhn
**   # initial version
**
**
*/

#ifndef _PIECE_H
#define _PIECE_H

#define PIECE_WORKER_ID 4
#include <iostream.h>

/* A piece worker gets paid for every item produced */

class PieceWorker : public WageWorker {
  private:
    int thisWeekTotal;           // number of items produced

  protected:
    void  ProducedThisWeek(int);

  public:
    PieceWorker(const char *, const char * , float = 0.0);

    virtual float Earnings();
    virtual void  Print();
    virtual void  NewWeek();
};
/*****************************************************************************/
PieceWorker::PieceWorker(const char *first, const char *last, float startWage) 
 : WageWorker(first, last, startWage)
{
    dollarsToRaise = 15.0;
}
/*****************************************************************************/
void
PieceWorker::ProducedThisWeek(int total) 
{
    thisWeekTotal = total;
}
/*****************************************************************************/
float
PieceWorker::Earnings() {
    return Wage() * thisWeekTotal;
}
/*****************************************************************************/
void
PieceWorker::Print() {
    cout << "       Piece Worker: " << FirstName() << ' ' << LastName();
}
/*****************************************************************************/
void
PieceWorker::NewWeek()
{
    int quantity = 3; //rand() % 5;

    ProducedThisWeek(quantity);
}

#endif
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
/* hourly-no-over.h                                                 -*- C++ -*-
**    Include file for Hourly No Overtime class
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
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/hourly-no-over.h $
** $Revision: 0.1 $
** $Date: 1994/12/24 01:45:45 $
**
** $Log: hourly-no-over.h $
** Revision 0.1  1994/12/24  01:45:45  bkuhn
**   # initial version
**
**
*/

#ifndef _HOURLY_NO_OVER_H
#define _HOURLY__NO_OVER_H

#include <iostream.h>
#include <stdlib.h>

#define HOURLY_WORKER_NO_OVERTIME_ID 7

/* An hourly worker no overtime gets paid for every hour worked, but no
** extra pay is given for overtime hours */

class HourlyWorkerNoOvertime : public HourlyWorker {

  public:
    HourlyWorkerNoOvertime(const char *, const char * , float = 0.0);

    virtual float Earnings();
};
/*****************************************************************************/
HourlyWorkerNoOvertime::HourlyWorkerNoOvertime(const char *first, const char *last,
                                     float startWage) 
 : HourlyWorker(first, last, startWage)    // this will call Wage's constructor
{
    return;
}
/*****************************************************************************/
float
HourlyWorkerNoOvertime::Earnings() {
    return Wage() * ThisWeekHours();
}

#endif
/* hourly-over.h                                                   -*- C++ -*-
**    Include file for Hourly Overtime class
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
**         Prentice Hall, New Jersey, p. 537
**
** RCS       :
**
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/hourly-over.h $
** $Revision: 0.1 $
** $Date: 1994/12/24 00:50:58 $
**
** $Log: hourly-over.h $
** Revision 0.1  1994/12/24  00:50:58  bkuhn
**   # initial version
**
**
*/

#ifndef _HOURLY_OVER_H
#define _HOURLY_OVER_H

#include <iostream.h>
#include <stdlib.h>

#define HOURLY_WORKER_OVERTIME_ID 6
/* An hourly worker overtime gets paid for every hour worked, and time and 
** a half for overtime hours */

class HourlyWorkerOvertime : public HourlyWorker {

  public:
    HourlyWorkerOvertime(const char *, const char * , float = 0.0);

    virtual float Earnings();
};
/*****************************************************************************/
HourlyWorkerOvertime::HourlyWorkerOvertime(const char *first, const char *last,
                                     float startWage) 
 : HourlyWorker(first, last, startWage)    // this will call Wage's constructor
{
    return;
}
/*****************************************************************************/
float
HourlyWorkerOvertime::Earnings() {
    float totHours, overHours;
    
    totHours = ThisWeekHours();

    if (totHours > 40.0) {
        overHours = totHours - 40.0;
    } else {
        overHours = 0.0;
    }

    return (Wage() * totHours) + ( (Wage() / 2.0) * overHours);
}

#endif
/* driver.cc                                                        -*- C++ -*-
**    Driver for the Employee Example
** 
** COPYRIGHT (C) 1994 Bradley M. Kuhn
**
** Written   :   Bradley M. Kuhn                                 Loyola College
**   By
**
** Written   :   David W. Binkley                                Loyola College
**   For         
**
** RCS       :
**
** $Source: /home/bkuhn/CURRENT/fbc/inline/EMPLOYEE-EXAMPLE/NON-OPT/RCS/driver.cc $
** $Revision: 0.1 $
** $Date: 1994/12/24 01:48:26 $
**
** $Log: driver.cc $
** Revision 0.1  1994/12/24  01:48:26  bkuhn
**   # initial version
**
**
*/

#include <stdio.h>

int
main(int argc, char *argv[])
{
    int        ii, totalWeeks;
    int        classCode;
    char       first[80], last[80];
    float      firstArg, secondArg;
    Employee * e;
    Company *  c;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <number_of_weeks>\n", argv[0]);
        return 1;
    }

    cout << setiosflags(ios::showpoint | ios::fixed) << setprecision(2);

    totalWeeks = atoi(argv[1]);

    c = new Company();

    while (scanf("%d%s%s%f", &classCode, first, last, &firstArg) == 4) {
        switch(classCode) {
          case BOSS_ID:
            e = new Boss(first, last, firstArg);
            break;

          case HOURLY_WORKER_NO_OVERTIME_ID:
            e = new HourlyWorkerNoOvertime(first, last, firstArg);
            break;

          case HOURLY_WORKER_OVERTIME_ID:
            e = new HourlyWorkerOvertime(first, last, firstArg);
            break;

          case PIECE_WORKER_ID:
            e = new PieceWorker(first, last, firstArg);
            break;

          case COMMISSIONED_WORKER_ID:
            scanf("%f", &secondArg);
            e = new CommissionedWorker(first, last, firstArg, secondArg);
            break;

          default:
            fprintf(stderr, "INVALID EMPLOYEE CODE(%d)\n", classCode);
            return -1;
        }

        c->AddEmployee(e);
    }

    for(ii = 0; ii < totalWeeks; ii++) {
        c->NewWeek();
        c->PrintWithEarnings();
        if ( (ii % 10) == 0) c->AcrossTheBoardRaise(1);
    }
    return 0;
}
