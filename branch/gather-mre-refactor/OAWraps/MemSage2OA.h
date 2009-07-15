#ifndef MemSage2OA_H
#define MemSage2OA_H

#include <list>
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include "rose.h"

#include "Sage2OA.h"


class SageIRMemRefIterator : public OA::MemRefHandleIterator 
{
  public:
    SageIRMemRefIterator(OA::StmtHandle h, SageIRInterface& ir);
    virtual ~SageIRMemRefIterator() { };
  
    virtual OA::MemRefHandle current() const; 
    virtual bool isValid() const { 
      return ( mValid && ( mMemRefIter != mEnd ) ); 
    }
          
    virtual void operator++();
    virtual void reset();

  private:
    void create(OA::StmtHandle h);

    std::list<OA::MemRefHandle> mMemRefList;
    
    std::list<OA::MemRefHandle>::iterator mEnd;
    std::list<OA::MemRefHandle>::iterator mBegin;
    std::list<OA::MemRefHandle>::iterator mMemRefIter;
    bool mValid;
    SageIRInterface& mIR;
};

// This is identical to Open64MemRefHandleIterator.
class SageMemRefHandleIterator
: public OA::IRHandleListIterator<OA::MemRefHandle>,
  public virtual OA::MemRefHandleIterator
{
 public:
  SageMemRefHandleIterator (OA::OA_ptr<std::list<OA::MemRefHandle> > pList);
  
  ~SageMemRefHandleIterator () {}

  void operator ++ ()
  { OA::IRHandleListIterator<OA::MemRefHandle>::operator++(); }

  //! is the iterator at the end
  bool isValid()  const
  { return OA::IRHandleListIterator<OA::MemRefHandle>::isValid(); }

  //! return current node
  OA::MemRefHandle current()  const
  { return OA::IRHandleListIterator<OA::MemRefHandle>::current(); }

  void reset()
  { return OA::IRHandleListIterator<OA::MemRefHandle>::reset(); }
};

#endif

