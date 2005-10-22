#ifndef _SAGE2OA_H
#define _SAGE2OA_H

// Treat config.h separately from other include files
#ifdef HAVE_CONFIG_H
// This avoids requiring the user to use config.h and follows 
// the automake manual request that we use <> instead of ""
#include <config.h>
#endif
#include "rose.h"

#include <OAConfig.h>

#include <OpenAnalysis/IRInterface/CFGIRInterfaceDefault.hpp>
#include <OpenAnalysis/IRInterface/SSAIRInterface.hpp>
#include <OpenAnalysis/IRInterface/CallGraphIRInterface.hpp>
#include <OpenAnalysis/IRInterface/AliasIRInterfaceDefault.hpp>
#include <OpenAnalysis/IRInterface/ReachDefsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/UDDUChainsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/XAIFIRInterface.hpp>
#include <OpenAnalysis/IRInterface/ParamBindingsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/InterSideEffectIRInterfaceDefault.hpp>
#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/SideEffect/ManagerSideEffectStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Alias/ManagerInsNoPtrInterAliasMap.hpp>
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>

class SageIRInterface;

class NumberTraversal
  : public SgSimpleProcessing
{
    public:
         void visit(SgNode* astNode);
         SageIRInterface * ir;
         NumberTraversal(SageIRInterface * in): ir(in) {}
};

class SageNodeNumAttr : public AstAttribute, public SgAttribute
{
  public:
    int number; // 0 based
    SageNodeNumAttr(int in): number(in){}
};

class SageStmtOAAttr : public AstAttribute, public SgAttribute
{
  public:
    SageStmtOAAttr(OA::OA_ptr<std::list<OA::MemRefHandle>  > tl) {all_mem_ref_lst=tl;}
    void set_lst(OA::OA_ptr<std::list<OA::MemRefHandle>  > tl) { all_mem_ref_lst=tl; }
    OA::OA_ptr<std::list<OA::MemRefHandle>  > get_lst() { return all_mem_ref_lst; }
  private:
    OA::OA_ptr<std::list<OA::MemRefHandle>  > all_mem_ref_lst;
};

class FindAllStmtsPass : public SgSimpleProcessing {
private:
 SgStatementPtrList & stmt_lst;
protected:
  void visit(SgNode* node);
public:
  FindAllStmtsPass(SgStatementPtrList& p) : stmt_lst(p) {}
};

	
class SageIRRegionStmtIterator: public OA::IRRegionStmtIterator {
public:
  SageIRRegionStmtIterator (SgStatementPtrList & lst, SageIRInterface * ir);
  SageIRRegionStmtIterator (SageIRInterface * in) { valid=FALSE; mTheList=NULL; mLength=0; mIndex=0; ir=in;}
  ~SageIRRegionStmtIterator () {}

  OA::StmtHandle current () const;
  bool isValid () const { return (valid && (mIndex<mLength)); } 
  void operator++ (); 

  void reset();
  SageIRInterface * ir;
private:
  SgStatementPtrList::iterator st_iter;
  //SgStatementPtrList::iterator begin;
  //SgStatementPtrList::iterator end;
  SgStatementPtrList * mTheList;
  bool valid;
  int mLength;
  int mIndex;
};

//
//as requested by Michelle Strout this iterator does not go inside initialization statements of the for loops and the like
//
class SageIRStmtIterator: public OA::IRStmtIterator 
{
 public:
  SageIRStmtIterator (SgFunctionDefinition * node, SageIRInterface * ir); 
  SageIRStmtIterator (SageIRInterface * in) {valid=FALSE; mLength=0; mIndex=0; ir=in;}
  ~SageIRStmtIterator () {}

  OA::StmtHandle current () const;
  bool isValid () const { return (valid && (mIndex<mLength)); } 
  void operator++ (); 

  void reset();
 private:
  SgStatementPtrList all_stmts; 
  SgStatementPtrList::iterator st_iter;
  //SgStatementPtrList::iterator begin;
  //SgStatementPtrList::iterator end;
  bool valid;
 int mLength;
 int mIndex;
  void FindAllStmts(SgNode *node, SgStatementPtrList& lst);
  SageIRInterface * ir;
};

class SageSymHandleIterator : public OA::SymHandleIterator {
 public:
  SageSymHandleIterator(OA::OA_ptr<std::list<OA::SymHandle> > symHandleList)
    : mList(symHandleList)
  { 
    mIter = mList->begin();
  }

  ~SageSymHandleIterator() { }

  OA::SymHandle current() const { return *mIter; }
  bool isValid() const { return ( mIter != mList->end() ); }

  void operator++() { if ( isValid() ) ++mIter; }

  void reset() { mIter = mList->begin(); }

 private:
  std::list<OA::SymHandle>::iterator mIter;
  OA::OA_ptr<std::list<OA::SymHandle> >  mList;
};

typedef SageSymHandleIterator SageIRSymIterator;
typedef SageSymHandleIterator SageIRFormalParamIterator;

class SageExprHandleIterator: public OA::ExprHandleIterator
{
 public:

  SageExprHandleIterator(OA::OA_ptr<std::list<OA::ExprHandle> > exprHandleList)
    : mList(exprHandleList)
  {
    mIter = mList->begin();
  }
  ~SageExprHandleIterator() { }

  OA::ExprHandle current() const { return *mIter; }
  bool isValid() const { return ( mIter != mList->end() ); }

  void operator++() { if ( isValid() ) ++mIter; }

  void reset() { mIter = mList->begin(); }

 private:
  std::list<OA::ExprHandle>::iterator mIter;
  OA::OA_ptr<std::list<OA::ExprHandle> >  mList;

};

typedef SageExprHandleIterator SageIRCallsiteParamIterator;

class SageMemRefExprIterator : public OA::MemRefExprIterator {
  public:      
  //#define TOOMANYCOLONS
#ifdef TOOMANYCOLONS
    SageMemRefExprIterator(OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr::MemRefExpr> > > pList,
      SageIRInterface * in)
      : mList(pList) { mIter = mList->begin(); ir=in;}
#else
    SageMemRefExprIterator(OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr> > > pList,
#endif
      SageIRInterface * in)
      : mList(pList) { mIter = mList->begin(); ir=in;}
    ~SageMemRefExprIterator() {}
                    
#ifdef TOOMANYCOLONS
    OA::OA_ptr<OA::MemRefExpr::MemRefExpr> current() const
#else
    OA::OA_ptr<OA::MemRefExpr> current() const
#endif
      { return *mIter; }

                        
    bool isValid() const { return mIter!=mList->end(); }
                                             
    void operator++() { if (isValid()) mIter++; }
    void operator++(int) { ++*this; }
    void reset() { mIter = mList->begin(); }
    SageIRInterface * ir;
  private:
#ifdef TOOMANYCOLONS
    OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr::MemRefExpr> > > mList;
#else
    OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr> > > mList;
#endif

#ifdef TOOMANYCOLONS
    std::list<OA::OA_ptr<OA::MemRefExpr::MemRefExpr> >::iterator mIter;
#else
    std::list<OA::OA_ptr<OA::MemRefExpr> >::iterator mIter;
#endif
};

class SageIRUseDefIterator
: public OA::IRHandleListIterator<OA::LeafHandle>,
  public virtual OA::SSA::IRUseDefIterator 
{
 public:
  SageIRUseDefIterator (OA::OA_ptr<std::list<OA::LeafHandle> > pList)
    : OA::IRHandleListIterator<OA::LeafHandle>(pList) {}
  ~SageIRUseDefIterator () {}

  void operator ++ ()
  { OA::IRHandleListIterator<OA::LeafHandle>::operator++(); }

  //! is the iterator at the end
  bool isValid()  const
  { return OA::IRHandleListIterator<OA::LeafHandle>::isValid(); }

  //! return current node
  OA::LeafHandle current()  const
  { return OA::IRHandleListIterator<OA::LeafHandle>::current(); }

  void reset()
  { return OA::IRHandleListIterator<OA::LeafHandle>::reset(); }
};


#if 0
class SageIRUseDefIterator : public OA::SSA::IRUseDefIterator 
{
  public:
    SageIRUseDefIterator(something missing here) { mIter = mList.begin(); }
    ~SageIRUseDefIterator() { };
  
    OA::LeafHandle current() const { return *mIter; }
    bool isValid() const { return ( mIter != mList.end() ); }
          
    void operator++() { if ( isValid() ) ++mIter; }

    void reset() { mIter = mList.begin(); }
  private:
    std::list<OA::LeafHandle>::iterator mIter;
    std::list<OA::LeafHandle>           mList;
};
#endif

class SgParamBindPtrAssignIterator 
    : public OA::Alias::ParamBindPtrAssignIterator
{ 
 public:
  SgParamBindPtrAssignIterator() : mValid(false), mIR(NULL) { }
  SgParamBindPtrAssignIterator(OA::ExprHandle call, 
			       SageIRInterface * ir)
    : mIR(ir) 
  { create(call); reset(); mValid = true; }
  virtual ~SgParamBindPtrAssignIterator() { };
  
  //! left hand side (i.e., formal)
  //  virtual OA::SymHandle currentTarget() const { return (*mIter).first; }
  virtual int currentFormalId() const { return (*mIter).first; }
  //! right hand side (i.e., actual)
  //  virtual OA::OA_ptr<OA::MemRefExpr> currentSource() const { return (*mIter).second; }
  virtual OA::OA_ptr<OA::MemRefExpr> currentActual() const { return (*mIter).second; }

  virtual bool isValid() const { 
    return ( mValid && ( mIter != mEnd ) ); 
  }
          
  virtual void operator++() { if (isValid()) mIter++; }
  virtual void reset();

 private:
  void create(OA::ExprHandle call);

  std::list<std::pair<int, OA::OA_ptr<OA::MemRefExpr> > > mPairList;
  
  std::list<std::pair<int, OA::OA_ptr<OA::MemRefExpr> > >::iterator mEnd;
  std::list<std::pair<int, OA::OA_ptr<OA::MemRefExpr> > >::iterator mBegin;
  std::list<std::pair<int, OA::OA_ptr<OA::MemRefExpr> > >::iterator mIter;
  bool mValid;
  SageIRInterface *mIR;
};

class SgPtrAssignPairStmtIterator 
    : public OA::Alias::PtrAssignPairStmtIterator
{ 
 public:
  SgPtrAssignPairStmtIterator() : mValid(false), mIR(NULL) { }
  SgPtrAssignPairStmtIterator(OA::StmtHandle stmt, SageIRInterface * ir)
    : mIR(ir) 
  { create(stmt); reset(); mValid = true; }
  virtual ~SgPtrAssignPairStmtIterator() { };
  
  //! left hand side
  virtual OA::OA_ptr<OA::MemRefExpr> currentTarget() const { return (*mIter).first; }
  //! right hand side
  virtual OA::OA_ptr<OA::MemRefExpr> currentSource() const { return (*mIter).second; }

  virtual bool isValid() const { 
    return ( mValid && ( mIter != mEnd ) ); 
  }
          
  virtual void operator++() { if (isValid()) mIter++; }
  virtual void reset();

 private:
  void create(OA::StmtHandle h);
  SgExpression *createPtrAssignPairsFromAssignment(SgNode *assign);

  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > mMemRefList;
  
  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > >::iterator mEnd;
  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > >::iterator mBegin;
  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > >::iterator mIter;
  bool mValid;
  SageIRInterface *mIR;
};

#if 0
// this worked (as did alias commented out)
class SageIRInterface : public OA::SSA::SSAIRInterface,
                        public OA::CFG::CFGIRInterfaceDefault,  
			public virtual OA::CallGraph::CallGraphIRInterface,
                        // public OA::CallGraph::CallGraphIRInterface,
                        public OA::Alias::AliasIRInterfaceDefault,
                        public virtual OA::ReachDefs::ReachDefsIRInterface,
                        // public OA::ReachDefs::ReachDefsIRInterface,
			public virtual OA::UDDUChains::UDDUChainsIRInterface,
                        // public OA::UDDUChains::UDDUChainsIRInterface,
			public virtual OA::XAIF::XAIFIRInterface
                        // public OA::XAIF::XAIFIRInterface
#endif

class SageIRInterface : public virtual OA::SSA::SSAIRInterface,
                        public virtual OA::CFG::CFGIRInterfaceDefault,  
			public virtual OA::CallGraph::CallGraphIRInterface,
                        public virtual OA::Alias::AliasIRInterfaceDefault,
                        public virtual OA::ReachDefs::ReachDefsIRInterface,
			public virtual OA::UDDUChains::UDDUChainsIRInterface,
                        public virtual OA::XAIF::XAIFIRInterface,
                        public virtual OA::DataFlow::ParamBindingsIRInterface,
                        public virtual OA::SideEffect::InterSideEffectIRInterfaceDefault,
                        public virtual OA::SideEffect::SideEffectIRInterface
{
  public:
  SageIRInterface (SgNode * root, std::vector<SgNode*> * na, bool use_persistent_handles=FALSE): nodeArrayPtr(na), wholeProject(root), persistent_handles(use_persistent_handles)
    { if(persistent_handles){createNodeArray(root);} }
  ~SageIRInterface () {}

    
  //! Given a subprogram return an IRStmtIterator* for the entire
  //! subprogram
  //! The user must free the iterator's memory via delete.
  OA::OA_ptr<OA::IRStmtIterator> getStmtIterator(OA::ProcHandle h); 
  
  //! Return an iterator over all of the callsites in a given stmt
  OA::OA_ptr<OA::IRCallsiteIterator> getCallsites(OA::StmtHandle h);

  //! get iterator over formal parameters in a function declaration 
  OA::OA_ptr<OA::IRFormalParamIterator> getFormalParamIterator(OA::SymHandle h); 

  //! Given a ProcHandle, return its SymHandle
  OA::SymHandle getProcSymHandle(OA::ProcHandle h);
  OA::MemRefHandle SageIRInterface::getSymMemRefHandle(OA::SymHandle h);
  OA::SymHandle getSymHandle(OA::ProcHandle h) {return getProcSymHandle(h);} //ask Michelle

  OA::SymHandle getSymHandle(OA::ExprHandle expr);

  //########################################################
  // Procedures and call sites
  //########################################################
/*
  // Given a procedure, return its IRProcType.
  IRProcType GetProcType(ProcHandle h) ;
*/ 
  // Given a ProcHandle, return an OA::CFG::IRRegionStmtIterator for the
  // procedure. 
  OA::OA_ptr<OA::IRRegionStmtIterator> procBody(OA::ProcHandle h) ;

  // Get IRCallsiteParamIterator for a callsite.
  // Iterator visits actual parameters in called order.
  OA::OA_ptr<OA::IRCallsiteParamIterator> getCallsiteParams(OA::ExprHandle h); 

/*  
  bool IsParamProcRef(ExprHandle h) ;

  // Given an expression representing a callsite, is this an
  // invocation through a procedure parameter.
  bool IsCallThruProcParam(ExprHandle h) ;
*/
  //########################################################
  // Statements: General
  //########################################################

  bool SageIRInterface::returnStatementsAllowed();

  OA::CFG::IRStmtType getCFGStmtType (OA::StmtHandle h);
  OA::StmtLabel getLabel (OA::StmtHandle h);

  //------------------------------
  // for compound statement. 
  //------------------------------
  OA::OA_ptr<OA::IRRegionStmtIterator> getFirstInCompound (OA::StmtHandle h);

  void createNodeArray(SgNode * root);
  SgNode * getNodePtr(OA::IRHandle h){if((int)h==0) return 0; else if(persistent_handles) return (*nodeArrayPtr)[h.hval()-1]; else return (SgNode*)h.hval();} //hvals start at 1
  int getNodeNumber(SgNode *);  //can be zero
  std::vector <SgNode *> * nodeArrayPtr;

  //------------------------------
  // for procedure, loop
  //------------------------------
private:
  SgNode * wholeProject;
  OA::OA_ptr<OA::IRRegionStmtIterator> body (OA::StmtHandle h);
  bool persistent_handles;
  //------------------------------
  // loops
  //------------------------------
public:
  OA::OA_ptr<OA::IRRegionStmtIterator> loopBody (OA::StmtHandle h);
  OA::StmtHandle loopHeader (OA::StmtHandle h);
  bool loopIterationsDefinedAtEntry (OA::StmtHandle h);
  OA::ExprHandle getLoopCondition (OA::StmtHandle h); 
  OA::StmtHandle getLoopIncrement (OA::StmtHandle h);

  //------------------------------
  // invariant: a two-way conditional or a multi-way conditional MUST provide
  // provided either a target, or a target label
  //------------------------------

  //------------------------------
  // unstructured two-way conditionals: 
  //------------------------------
  // two-way branch, loop continue
  OA::StmtLabel  getTargetLabel (OA::StmtHandle h, int n);

  OA::ExprHandle getCondition (OA::StmtHandle h);

  //------------------------------
  // unstructured multi-way conditionals
  //------------------------------
  int numUMultiTargets (OA::StmtHandle h);
  OA::StmtLabel getUMultiTargetLabel (OA::StmtHandle h, int targetIndex);
  OA::StmtLabel getUMultiCatchallLabel (OA::StmtHandle h);
  OA::ExprHandle getUMultiCondition (OA::StmtHandle h, int targetIndex);

  //--------------------------------------------------------
  // Special, for assembly-language level instructions only.
  // These are necessary because there are some intricacies involved
  // in building a CFG for an instruction set which has delayed branches,
  // and in particular, allows branches within branch delay slots. 
  //--------------------------------------------------------

  // Given a statement, return true if it issues in parallel with its
  // successor.  This would be used, for example, when the underlying IR
  // is a low-level/assembly-level language for a VLIW or superscalar
  // instruction set. The default implementation (which is appropriate
  // for most IR's) is to return false.
  //bool parallelWithSuccessor(OA::StmtHandle h) { return false; } 

  // Given an unstructured branch/jump statement, return the number
  // of delay slots. Again, this would be used when the underlying IR
  // is a low-level/assembly-level language for a VLIW or superscalar
  // instruction set. The default implementation (which is appropriate
  // for most IR's) is to return 0.
  //int numberOfDelaySlots(OA::StmtHandle h) { return 0; } 

  //------------------------------
  // structured multiway conditionals
  //------------------------------
  int numMultiCases (OA::StmtHandle h);
  // condition for multi body 
  OA::ExprHandle getSMultiCondition (OA::StmtHandle h, int bodyIndex);
  // multi-way beginning expression
  OA::ExprHandle getMultiExpr (OA::StmtHandle h);
  OA::OA_ptr<OA::IRRegionStmtIterator> 
      multiBody (OA::StmtHandle h, int bodyIndex);
  bool isBreakImplied (OA::StmtHandle multicond);
  bool isCatchAll(OA::StmtHandle h, int bodyIndex);
  OA::OA_ptr<OA::IRRegionStmtIterator> getMultiCatchall (OA::StmtHandle h);

  //------------------------------
  // structured conditionals
  //------------------------------
  OA::OA_ptr<OA::IRRegionStmtIterator> trueBody (OA::StmtHandle h);
  OA::OA_ptr<OA::IRRegionStmtIterator> elseBody (OA::StmtHandle h);

  //Alias stuff
  OA::OA_ptr<OA::MemRefHandleIterator> getAllMemRefs(OA::StmtHandle stmt);
  // getAllMemRefs(OA::IRHandle h) is intended to be called by
  // ROSE, which may neither have nor know how to create a StmtHandle.
  // e.g., may_alias(AstNodePtr r1, AstNodePtr r2);
  OA::OA_ptr<OA::MemRefHandleIterator> getAllMemRefs(OA::IRHandle h);
  OA::Alias::IRStmtType getAliasStmtType(OA::StmtHandle h);
  OA::OA_ptr<OA::IRSymIterator> getVisibleSymIterator(OA::ProcHandle h);
  OA::OA_ptr<OA::IRStmtIterator> getUnnamedDynAllocStmtIterator(OA::ProcHandle h); 
  OA::OA_ptr<OA::MemRefExprIterator> 
  getMemRefExprIterator(OA::MemRefHandle h);

  OA::OA_ptr<OA::Location::Location> 
  getLocation(OA::ProcHandle p, OA::SymHandle s);

  OA::SymHandle getFormalSym(OA::ProcHandle, int);

  OA::OA_ptr<OA::MemRefExpr> getCallMemRefExpr(OA::CallHandle h);

  OA::ProcHandle getProcHandle(OA::SymHandle sym);

  //ReachDefs
  OA::OA_ptr<OA::MemRefHandleIterator> getDefMemRefs(OA::StmtHandle stmt);
  OA::OA_ptr<OA::MemRefHandleIterator> getUseMemRefs(OA::StmtHandle stmt);
  
  //------------------------------
  //------------------------------

  // Given a SymHandle, return the textual name.
  char *getSymNameFromSymHandle (OA::SymHandle h) 
  {
    SgNode * sgn=getNodePtr(h);
    SgFunctionDeclaration * fd=isSgFunctionDeclaration(sgn);
    if(fd)
    {
      SgName nm=fd->get_name();
      //      SgName nm=fd->get_mangled_name();
      
      return nm.str(); 
    } 
  }
/*
  // Given a ConstHandle, return the textual name.
  char *GetConstNameFromConstHandle(ConstHandle ch) {
      return NULL;
  }
*/
  //-------------------------------------------------------------------------
  // IRHandlesIRInterface
  //-------------------------------------------------------------------------
     
  //--------------------------------------------------------
  // create a string for the given handle, should be succinct
  // and there should be no newlines
  //--------------------------------------------------------  
  std::string toString(const OA::ProcHandle h);
  std::string toString(const OA::StmtHandle h) 
      {
          if(h!=0)
          {
          SgStatement * sgstmt=(SgStatement*)(getNodePtr(h));
          return sgstmt->unparseToString();
          }
          else return ("NULL stmt handle\n");
      }
  std::string toString(const OA::ExprHandle h) 
      {
          SgExpression * ex=(SgExpression*)getNodePtr(h);
          return ex->unparseToString();
    }
  std::string toString(const OA::CallHandle h) 
  {
    SgNode *node = getNodePtr(h);
    SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
    ROSE_ASSERT(functionCallExp != NULL);
    return functionCallExp->unparseToString();
  }
  std::string toString(const OA::OpHandle h) { return ""; }
  std::string toString(const OA::MemRefHandle h);
  std::string toString(const OA::SymHandle h); 
  std::string toString(const OA::ConstSymHandle h) { return ""; }
  std::string toString(const OA::ConstValHandle h) { return ""; }
  std::string toString(OA::Alias::IRStmtType x);
  
  void dump(OA::OA_ptr<OA::MemRefExprIterator> memRefIterator,
	    std::ostream& os);

  void dump(OA::MemRefHandle h, std::ostream& os) 
  {
    if(!h)
      os<<"mem ref handle is NULL"<<std::endl;
    else
    {
      //Sg_File_Info * finfo=((SgExpression*)(getNodePtr(h)))->get_file_info();
      std::string strdump;
      //os<<finfo->get_filename()<<"  "<<finfo->get_line()<<"  "<<
      //finfo->get_col()<<"  " ;
      if(isSgExpression(getNodePtr(h)))
        strdump=((SgExpression*)(getNodePtr(h)))->unparseToString();
      else if(isSgInitializedName(getNodePtr(h)))
        strdump=((((SgInitializedName*)getNodePtr(h)))->get_name()).getString();
      os<<"  "<<strdump<<"   "<<(int)(h.hval())<<std::endl;
    }
  }
  void dump(OA::StmtHandle h, std::ostream& os) { os << toString(h); }

  
  void currentProc(OA::ProcHandle p);

  //-------------------------------------------------------------------------
  // AliasIRInterfaceDefault
  //-------------------------------------------------------------------------
    
  //! If this is a PTR_ASSIGN_STMT then return an iterator over MemRefHandle
  //! pairs where there is a source and target such that target
  OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator>
  getPtrAssignStmtPairIterator(OA::StmtHandle stmt);
  
  OA::OA_ptr<OA::Alias::ParamBindPtrAssignIterator>
  getParamBindPtrAssignIterator(OA::ExprHandle call);

  // returns true if given symbol is a reference variable.
  bool isRefParam(OA::SymHandle);

  // returns true if given MemRefExpr is a reference variable.
  bool isReferenceExpr(OA::OA_ptr<OA::MemRefExpr>);

  // returns true if given symbol is a pointer variable.
  bool isPointerVar(OA::SymHandle);
  
  //-------------------------------------------------------------------------
  // SSAIRInterface
  //-------------------------------------------------------------------------

  // Getsymhandle Given a LeafHandle containing a use or def, return
  // the referened SymHandle.
  OA::SymHandle getSymHandle(OA::LeafHandle h);

  //! Given a statement, return uses (variables referenced)
  OA::OA_ptr<OA::SSA::IRUseDefIterator> getUses(OA::StmtHandle h);

  //! Given a statement, return defs (variables defined)
  OA::OA_ptr<OA::SSA::IRUseDefIterator> getDefs(OA::StmtHandle h);

  //-------------------------------------------------------------------------
  // ParamBindingsIRInterface
  //-------------------------------------------------------------------------
  
  //! Given a subprogram return an IRSymIterator for all
  //! symbols that are referenced within the subprogram
  OA::OA_ptr<OA::IRSymIterator> getRefSymIterator(OA::ProcHandle h);

  //! returns true if given symbol is a parameter 
  bool isParam(OA::SymHandle);

  // return the formal parameter that an actual parameter is associated with 
  OA::SymHandle getFormalForActual(OA::ProcHandle caller, OA::ExprHandle call, 
				   OA::ProcHandle callee, OA::ExprHandle param);

  // Given an ExprHandle, return an ExprTree 
  OA::OA_ptr<OA::ExprTree> getExprTree(OA::ExprHandle h);

  //-------------------------------------------------------------------------
  // output methods
  //-------------------------------------------------------------------------
  void dump(OA::OA_ptr<OA::NamedLoc> loc, std::ostream& os);
  void dump(OA::OA_ptr<OA::UnnamedLoc> loc, std::ostream& os);
  void dump(OA::OA_ptr<OA::InvisibleLoc> loc, std::ostream& os);
  void dump(OA::OA_ptr<OA::UnknownLoc> loc, std::ostream& os);
  void dump(OA::OA_ptr<OA::Location> loc, std::ostream& os);
  void dump(OA::OA_ptr<OA::NamedRef> memRefExp, std::ostream& os);
  void dump(OA::OA_ptr<OA::UnnamedRef> memRefExp, std::ostream& os);
  void dump(OA::OA_ptr<OA::UnknownRef> memRefExp, std::ostream& os);
  void dump(OA::OA_ptr<OA::Deref> memRefExp, std::ostream& os);
  void dump(OA::OA_ptr<OA::MemRefExpr> memRefExp, std::ostream &os);

  //-------------------------------------------------------------------------
  // Interface to ROSE
  //-------------------------------------------------------------------------
  OA::IRHandle getHandle(SgNode *astNode) { return getNodeNumber(astNode); }
  OA::MemRefHandle getMemRefHandle(SgNode *astNode) { 
    OA::MemRefHandle memRefHandle = (OA::MemRefHandle) 0;
    if ( isMemRefNode(astNode) ) {
      memRefHandle = getNodeNumber(astNode);
    }
    return memRefHandle;
  }
  OA::ProcHandle getProcHandle(SgFunctionDefinition *astNode);
  //  OA::ProcHandle getProcHandle(SgFunctionDeclaration *astNode);
  SgNode *getSgNode(OA::IRHandle h) { return getNodePtr(h); }
  bool isMemRefNode(SgNode *astNode);

 protected:

  std::list<SgNode *> *findTopMemRefs(SgNode *astNode);
  std::list<SgNode *> *findIndependentMemRefs(SgNode *astNode);
  void findIndependentMemRefs(SgNode *astNode, std::list<SgNode *>& topMemRefs,
			      bool collectOnlyTopMemRefs);
  void getChildrenWithMemRefs(SgNode *astNode,
			      std::vector<SgNode *>& independtChildren,
			      std::vector<SgNode *>& children);

 private:

  // Return the MemRefType of a MemRefExpr.
  OA::MemRefExpr::MemRefType getMemRefType(OA::OA_ptr<OA::MemRefExpr> mre);

  // Look through typedefs to return a type.
  SgType *SageIRInterface::getBaseType(SgType *type); 

  SgFunctionDefinition *getEnclosingMethod(SgNode *node);

  SgStatement *getEnclosingStatement(SgNode *node);

  bool isArrowExp(SgExpression *function);

  string refTypeToString(OA::OA_ptr<OA::MemRefExpr> memRefExp);

  OA::OA_ptr<OA::MemRefHandleIterator> 
    getMemRefIterator(OA::IRHandle h);

  // assumption is that StmtHandles and MemRefHandles are unique across
  // different program and procedure contexts for which analysis is being
  // currently performed
  static std::map<OA::IRHandle,std::set<OA::MemRefHandle> >
    sStmt2allMemRefsMap;

  static std::map<OA::MemRefHandle,OA::IRHandle> sMemRef2StmtMap;

  static std::map<OA::MemRefHandle,set<OA::OA_ptr<OA::MemRefExpr> > >
    sMemref2mreSetMap;

  static std::map<OA::OA_ptr<OA::MemRefExpr>,OA::MemRefHandle >
    sMre2MemrefMap;

  // Given a SgNode return a symbol handle to represent the
  // corresponding SgThisExp.  This symbol handle references
  // a SgClassSymbol.
  OA::SymHandle getThisExpSymHandle(SgNode *node);

  // Given a SgFunctionCallExp representing a method invocation,
  // return a MemRefHandle to the object upon which the method
  // was invoked.
  //  OA::MemRefHandle getDispatchingObjectMemRefHandle(SgFunctionCallExp *func);

  // Strip off any leading SgCastExps/SgAssignInitializers in 
  // the tree root at node.
  SgNode *lookThroughCastExpAndAssignInitializer(SgNode *node);

  // initializerHasPtrAssign returns true if the SgInitializedName
  // is a pointer or reference and it has an initializer.
  // If collectPtrAssigns is true, it returns the corresponding 
  // MemRefHandle/MemRefExpr pairs in memRefList.
  bool initializerHasPtrAssign(SgInitializedName *initName,
			       bool collectPtrAssigns,
			       std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList);

  // varDeclHasPtrAssign returns true if the variable declaration
  // assigns values to a pointer.  If collectPtrAssigns is true,
  // it returns the corresponding MemRefHandle/MemRefExpr pairs
  // in memRefList.
  bool varDeclHasPtrAssign(SgVariableDeclaration *varDeclaration,
			   bool collectPtrAssigns,
			   std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList);

  // cotrInitListHasPtrAssign returns true if the constructor
  // initializer list initializes any pointers or references.
  // If collectPtrAssigns is true, it returns the corresponding 
  // MemRefHandle/MemRefExpr pairs in memRefList.
  bool ctorInitListHasPtrAssign(SgCtorInitializerList *initializerList,
				bool collectPtrAssigns,
				std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList);

    

  friend class SageIRMemRefIterator;
  friend class SgPtrAssignPairStmtIterator;
  friend class SgParamBindPtrAssignIterator;
  friend class ExprTreeTraversal;
};

#endif

