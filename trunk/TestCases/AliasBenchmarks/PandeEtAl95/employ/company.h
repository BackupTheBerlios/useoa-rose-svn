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

#include "employee.h"

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
