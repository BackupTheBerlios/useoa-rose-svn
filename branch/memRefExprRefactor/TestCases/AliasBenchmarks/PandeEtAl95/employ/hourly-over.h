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

#include "hourly.h"

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
