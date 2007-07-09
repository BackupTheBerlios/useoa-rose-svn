#ifndef _SAGEOACALLGRAPH_H
#define _SAGEOACALLGRAPH_H

// Treat config.h separately from other include files
#ifdef HAVE_CONFIG_H
// This avoids requiring the user to use config.h and follows 
// the automake manual request that we use <> instead of ""
//#include <config.h>
#endif
#include "rose.h"
#
#include <OpenAnalysis/IRInterface/CallGraphIRInterface.hpp>
#include "Sage2OA.h"

class FindCallsitesPass : public SgSimpleProcessing {
private:
  // (BW 5/3/07)  Since we now represent constructor invocations
  // via their Sg_File_Info, rather than the SgConstructorInitializer,
  // the call_lst can no longer be an expression list.
  // SgExpressionPtrList & call_lst;
  std::list<SgNode *> &call_lst;
 SageIRInterface & mIR;
protected:
  void visit(SgNode* node);
public:
  //  FindCallsitesPass(SageIRInterface &ir, SgExpressionPtrList& p) 
  FindCallsitesPass(SageIRInterface &ir, std::list<SgNode*> & p) 
    : mIR(ir), call_lst(p) {}
};

class FindProcsPass : public SgSimpleProcessing {
private:
  std::set<SgStatement*> & proc_lst;
protected:
  void visit(SgNode* node);
public:
  FindProcsPass(std::set<SgStatement *>& p)  
    : proc_lst(p) {}
};




//      ProcHAndle corresponds to SgFunctionDefinition

/*!
 * Enumerate over all procedures in the IR.
 */

class SageIRProcIterator: public OA::IRProcIterator 
{
 public:
  SageIRProcIterator (SgNode *node, SageIRInterface& in);
  // MMS, can't take an OA_ptr to SageIRInterface because need to
  // construct one of these within a SageIRInterface and can't pass this to
  // an OA_ptr
  ~SageIRProcIterator () {}

  OA::ProcHandle current () const;
  bool isValid () const { return (valid && (st_iter!=end)); } 
  void operator++ (); 

  void reset();

  //! Add any ProcHandles from otherIter to this.
  //! Warning this might disrupt the current pointer.
  void unionIterators(SageIRProcIterator &otherIter);

  SageIRInterface& ir;
 private:
  std::set<SgStatement*> procs_in_proj; 
  std::set<SgStatement*>::iterator st_iter;
  std::set<SgStatement*>::iterator begin;
  std::set<SgStatement*>::iterator end;
  bool valid;
  void FindProcsInSgTree(SgNode *node, std::set<SgStatement*>& lst);
  bool mExcludeInputFiles;

};

/*!
 * Enumerate over all procedure calls in a statement
 *
 *as requested by Michelle Strout this iterator does not go inside the body of any compound statements
*
 */
class SageIRCallsiteIterator: public OA::IRCallsiteIterator
{
 public:
  SageIRCallsiteIterator(SgStatement * sgstmt, SageIRInterface &in); 
//  SageIRCallsiteIterator() : { valid=FALSE; }
  ~SageIRCallsiteIterator() { }

  OA::CallHandle current() const;  // Returns the current item.
  bool isValid() const { return (valid && (st_iter!=end)); }        // False when all items are exhausted.
        
  void operator++();
  void operator++(int) { ++*this; } ;

  void reset();
  SageIRInterface &ir;
 private:
#if 1
  // (BW 5/3/07)  Since we now represent constructor invocations
  // via their Sg_File_Info, rather than the SgConstructorInitializer,
  // the calls_in_stmt can no longer be an expression list.
  // SgExpressionPtrList & call_lst;
  std::list<SgNode*> calls_in_stmt; 
  std::list<SgNode*>::iterator st_iter;
  std::list<SgNode*>::iterator begin;
  std::list<SgNode*>::iterator end;
  void FindCallsitesInSgStmt(SgStatement *sgstmt, std::list<SgNode*> & lst);
#else
  SgExpressionPtrList calls_in_stmt; 
  SgExpressionPtrList::iterator st_iter;
  SgExpressionPtrList::iterator begin;
  SgExpressionPtrList::iterator end;
  void FindCallsitesInSgStmt(SgStatement *sgstmt, SgExpressionPtrList& lst);
#endif
  bool valid;
};

#endif
