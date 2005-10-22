#ifndef _SAGEOACALLGRAPH_H
#define _SAGEOACALLGRAPH_H

// Treat config.h separately from other include files
#ifdef HAVE_CONFIG_H
// This avoids requiring the user to use config.h and follows 
// the automake manual request that we use <> instead of ""
#include <config.h>
#endif
#include "rose.h"
#
#include <OpenAnalysis/IRInterface/CallGraphIRInterface.hpp>
#include "Sage2OA.h"

class FindCallsitesPass : public SgSimpleProcessing {
private:
 SgExpressionPtrList & call_lst;
protected:
  void visit(SgNode* node);
public:
  FindCallsitesPass(SgExpressionPtrList& p) : call_lst(p) {}
};

class FindProcsPass : public SgSimpleProcessing {
private:
 SgStatementPtrList & proc_lst;
protected:
  void visit(SgNode* node);
public:
  FindProcsPass(SgStatementPtrList& p) : proc_lst(p) {}
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
  SageIRProcIterator (SgNode *node, SageIRInterface * in); 
  //should really be a list of SgFunctionDefinition pointers? or maybe just SgProject ptr?
                                                 //maybe both and SgFile too?
  SageIRProcIterator () { valid=FALSE;}
  ~SageIRProcIterator () {}

  OA::ProcHandle current () const;
  bool isValid () const { return (valid && (st_iter!=end)); } 
  void operator++ (); 

  void reset();
  SageIRInterface *ir;
 private:
  SgStatementPtrList procs_in_proj; 
  SgStatementPtrList::iterator st_iter;
  SgStatementPtrList::iterator begin;
  SgStatementPtrList::iterator end;
  bool valid;
  void FindProcsInSgTree(SgNode *node, SgStatementPtrList& lst);

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
  SageIRCallsiteIterator(SgStatement * sgstmt, SageIRInterface * in); 
  SageIRCallsiteIterator() { valid=FALSE;}
  ~SageIRCallsiteIterator() { }

  OA::CallHandle current() const;  // Returns the current item.
  bool isValid() const { return (valid && (st_iter!=end)); }        // False when all items are exhausted.
        
  void operator++();
  void operator++(int) { ++*this; } ;

  void reset();
  SageIRInterface * ir;
 private:
  SgExpressionPtrList calls_in_stmt; 
  SgExpressionPtrList::iterator st_iter;
  SgExpressionPtrList::iterator begin;
  SgExpressionPtrList::iterator end;
  bool valid;
  void FindCallsitesInSgStmt(SgStatement *sgstmt, SgExpressionPtrList& lst);
};

#endif
