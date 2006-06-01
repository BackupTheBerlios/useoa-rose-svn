#ifndef _SAGEOACALLGRAPH_H
#define _SAGEOACALLGRAPH_H

#include "rose.h"

#include <OpenAnalysis/IRInterface/CallGraphIRInterface.hpp>
#include "Sage2OA.h"

class FindCallsitesPass : public SgSimpleProcessing {
private:
 SgExpressionPtrList & call_lst;
 OA::OA_ptr<SageIRInterface> mIR;
protected:
  void visit(SgNode* node);
public:
  FindCallsitesPass(OA::OA_ptr<SageIRInterface> ir, SgExpressionPtrList& p) 
    : mIR(ir), call_lst(p) {}
};

class FindProcsPass : public SgSimpleProcessing {
private:
 SgStatementPtrList & proc_lst;
protected:
  void visit(SgNode* node);
public:
  FindProcsPass(SgStatementPtrList& p)  
    : proc_lst(p) {}
};




//	ProcHAndle corresponds to SgFunctionDefinition

/*!
 * Enumerate over all procedures in the IR.
 */

class SageIRProcIterator: public OA::IRProcIterator 
{
 public:
  // This should take a SgNode *, not a SgProject *.  The former
  // is more general.  bwhite
  //  SageIRProcIterator (SgProject * sageproject, SageIRInterface * in); 
  SageIRProcIterator (SgNode *node, OA::OA_ptr<SageIRInterface> in,
		      bool excludeInputFiles = false); 
  // MMS, must take OA_ptr to SageIRInterface because can't have raw ptrs
  // and OA_ptrs to the same object
  //should really be a list of SgFunctionDefinition pointers? or maybe just SgProject ptr?
                                                 //maybe both and SgFile too?
  SageIRProcIterator () { valid=FALSE;}
  ~SageIRProcIterator () {}

  OA::ProcHandle current () const;
  bool isValid () const { return (valid && (st_iter!=end)); } 
  void operator++ (); 

  void reset();
  OA::OA_ptr<SageIRInterface> ir;
 private:
  SgStatementPtrList procs_in_proj; 
  SgStatementPtrList::iterator st_iter;
  SgStatementPtrList::iterator begin;
  SgStatementPtrList::iterator end;
  bool valid;
  void FindProcsInSgTree(SgNode *node, SgStatementPtrList& lst);
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
  SageIRCallsiteIterator(SgStatement * sgstmt, OA::OA_ptr<SageIRInterface> in); 
  SageIRCallsiteIterator() { valid=FALSE;}
  ~SageIRCallsiteIterator() { }

  OA::CallHandle current() const;  // Returns the current item.
  bool isValid() const { return (valid && (st_iter!=end)); }        // False when all items are exhausted.
        
  void operator++();
  void operator++(int) { ++*this; } ;

  void reset();
  OA::OA_ptr<SageIRInterface> ir;
 private:
  SgExpressionPtrList calls_in_stmt; 
  SgExpressionPtrList::iterator st_iter;
  SgExpressionPtrList::iterator begin;
  SgExpressionPtrList::iterator end;
  bool valid;
  void FindCallsitesInSgStmt(SgStatement *sgstmt, SgExpressionPtrList& lst);
};

#endif
