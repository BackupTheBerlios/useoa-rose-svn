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

#include "wage.h"

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
