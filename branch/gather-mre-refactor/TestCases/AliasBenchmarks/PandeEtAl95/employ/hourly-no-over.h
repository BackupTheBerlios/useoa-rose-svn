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

#include "hourly.h"

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
