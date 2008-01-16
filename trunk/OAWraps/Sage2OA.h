#ifndef _SAGE2OA_H
#define _SAGE2OA_H

#include "rose.h"

#include <OpenAnalysis/IRInterface/CFGIRInterfaceDefault.hpp>
#include <OpenAnalysis/IRInterface/SSAIRInterface.hpp>
#include <OpenAnalysis/IRInterface/CallGraphIRInterface.hpp>
#include <OpenAnalysis/IRInterface/ICFGIRInterface.hpp>
#include <OpenAnalysis/IRInterface/ActivityIRInterface.hpp>
#include <OpenAnalysis/IRInterface/AliasIRInterfaceDefault.hpp>
#include <OpenAnalysis/IRInterface/ReachDefsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/UDDUChainsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/XAIFIRInterface.hpp>
#include <OpenAnalysis/IRInterface/ParamBindingsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/InterSideEffectIRInterfaceDefault.hpp>
#include <OpenAnalysis/IRInterface/LinearityIRInterface.hpp>
#include <OpenAnalysis/IRInterface/DataDepIRInterface.hpp>
#include <OpenAnalysis/IRInterface/LoopIRInterface.hpp>
#include <OpenAnalysis/IRInterface/auto_ReachingDefsIRInterface.hpp>
#include <OpenAnalysis/IRInterface/LivenessIRInterface.hpp>
#include <OpenAnalysis/SideEffect/ManagerInterSideEffectStandard.hpp>
#include <OpenAnalysis/SideEffect/ManagerSideEffectStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>
#include <OpenAnalysis/ExprTree/ExprTree.hpp>
#include <OpenAnalysis/Loop/LoopAbstraction.hpp>

// ReachConsts
#include <OpenAnalysis/IRInterface/ConstValBasicInterface.hpp>
#include <OpenAnalysis/IRInterface/ConstValIntInterface.hpp>
#include <OpenAnalysis/IRInterface/ReachConstsIRInterface.hpp>

using namespace OA;

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

/*! template to construct an implementation of one of the MRE iterator
    types.  P specifies the class we're inheriting from, T specifies the
    type of objects this iterators over, L should be the
    list<OA_ptr<T> >::iterator.  L is needed since explicitly decleraing an
    iterator in the class causes an error.
*/
template<class P, class T, class L>
class SageMREIteratorImplementation : public P {
  public:
    SageMREIteratorImplementation(
        OA_ptr<list<OA_ptr<T> > > pList,
        SageIRInterface * in)
    :
        mList(pList)
    {
        mIter = mList->begin();
        ir=in;
    }
    ~SageMREIteratorImplementation() {}
                    
    OA_ptr<T> current() const { 
      return *mIter;
    }

    bool isValid() const { return mIter!=mList->end(); }

    int count() const { return mList->size(); }
                                             
    void operator++() { if (isValid()) mIter++; }
    void operator++(int) { ++*this; }
    void reset() { mIter = mList->begin(); }
    SageIRInterface * ir;
  private:
    L mIter;
    OA_ptr<list<OA_ptr<T> > > mList;
};

// Construct a SageMREIteratorImplementation class
#define BuildSgMREIter(A) \
    typedef SageMREIteratorImplementation< \
        A ## Iterator, A, list<OA_ptr< A > >::iterator >  \
        Sage ## A ## Iterator;

BuildSgMREIter(MemRefExpr);
BuildSgMREIter(NamedRef);
BuildSgMREIter(UnnamedRef);
BuildSgMREIter(UnknownRef);
BuildSgMREIter(RefOp);
BuildSgMREIter(Deref);
BuildSgMREIter(SubSetRef);
BuildSgMREIter(IdxAccess);
BuildSgMREIter(IdxExprAccess);
BuildSgMREIter(FieldAccess);



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
  SgParamBindPtrAssignIterator(OA::CallHandle call, 
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
  void create(OA::CallHandle call);

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

  void createPtrAssignPairsFromReturnStmt(SgReturnStmt *returnStmt);

  SgExpression *createPtrAssignPairsFromAssignment(SgNode *assign);

  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > mMemRefList;
  
  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > >::iterator mEnd;
  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > >::iterator mBegin;
  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > >::iterator mIter;
  bool mValid;
  SageIRInterface *mIR;
};

typedef std::pair<OA::MemRefHandle,OA::ExprHandle>AssignPair;
typedef std::list<AssignPair> AssignPairList;
class SageIRAssignPairIterator 
    : public OA::AssignPairIterator {
  public:
    SageIRAssignPairIterator(OA::OA_ptr<AssignPairList> pAssignList) 
        : mAssignList(pAssignList) { reset(); }
    virtual ~SageIRAssignPairIterator() {}

    //! left hand side
    OA::MemRefHandle currentTarget() const 
      { if (isValid()) { return mIter->first; } 
        else { return OA::MemRefHandle(0); } }
    //! right hand side
    OA::ExprHandle currentSource() const 
      { if (isValid()) { return mIter->second; } 
        else { return OA::ExprHandle(0); } }

    bool isValid() const { return mIter!=mAssignList->end(); }
                    
    void operator++() { if (isValid()) mIter++; }
    void operator++(int) { ++*this; }

    void reset() { mIter = mAssignList->begin(); }
  private:
    OA::OA_ptr<AssignPairList> mAssignList;
    AssignPairList::iterator mIter;
};

enum typeEnum { reference, other };


class SageConstVal : public virtual OA::ConstValBasicInterface {
  public:
    SageConstVal() {}
    virtual ~SageConstVal() {}

    // Methods needed by OA, default behavior
    virtual bool operator<(OA::ConstValBasicInterface& x)
    { return false; }

    virtual bool operator==(OA::ConstValBasicInterface& x)
    { return false; }

    virtual bool operator!=(OA::ConstValBasicInterface& x)
    { return true; }

    virtual std::string toString() { return ""; }

    // Methods used by source IR, default behavior
    virtual bool isaInteger() const { return false; }

    virtual int getIntegerVal() const { return 0; }
    // FIXME: THROW EXCEPTION?

    virtual bool isaDouble() const { return false; }

    virtual double getDoubleVal() const { return 0.0; }
    // FIXME: EXCEPTION?

    virtual bool isaChar() const { return false; }
    virtual char getCharVal() const { return '0'; }
    // FIXME: THROW EXCEPTION?

    // bool isaComplex() { return false; }
    // ...

    /*
    // eval: Given an operator and two operands
       (one being the current

    // object), return a new object representing the result.
    virtual OA::OA_ptr<ConstValBasicInterface>
    eval(OPERATOR opr,
        const OA::OA_ptr<OA::ConstValBasicInterface> op2) const;
    */

    virtual OA::OA_ptr<OA::ConstValBasicInterface>
    eval( SgBinaryOp * oper, 
        const OA::OA_ptr<OA::ConstValBasicInterface> op2) const;
};



class SageIntegerConstVal
  : public SageConstVal,
    public virtual OA::ConstValIntInterface {
  public:
    SageIntegerConstVal() {}
    SageIntegerConstVal(int aVal) : SageConstVal(), mVal(aVal) {}
    ~SageIntegerConstVal() {}

    // Methods used by OpenAnalysis
    bool operator<(OA::ConstValBasicInterface& other)
    {
        SageConstVal& otherRecast = dynamic_cast<SageConstVal&>(other);
        if (otherRecast.isaInteger()) {
            return (otherRecast.getIntegerVal() < mVal);
        }
        return false;
    }
    bool operator==(OA::ConstValBasicInterface& other) {
        SageConstVal& otherRecast = dynamic_cast<SageConstVal&>(other);
        if (otherRecast.isaInteger()) {
            return (otherRecast.getIntegerVal() == mVal);
        }
        return false;
    }
    bool operator!=(OA::ConstValBasicInterface& other) {
        SageConstVal& otherRecast = dynamic_cast<SageConstVal&>(other);
        if (otherRecast.isaInteger()) {
            return (otherRecast.getIntegerVal() != mVal);
        }
        return true;
    }

    std::string toString()
        { std::ostringstream oss; oss << mVal; return oss.str(); }


    // Methods used by source IR specific to this data type
    bool isaInteger() const { return true; }
    int getIntegerVal() const { return mVal; }

    
    /*! PLM 10/30/06 Not Implemented yet.
    // eval: Given an operator and two operands (one being the current
    // object), return a new object representing the result.
    virtual OA::OA_ptr<ConstValBasicInterface>
    eval(OPERATOR opr, const OA::OA_ptr<OA::ConstValBasicInterface> op2) const;
    */
    OA::OA_ptr<OA::ConstValBasicInterface>
        eval( SgBinaryOp *oper, 
                const OA::OA_ptr<OA::ConstValBasicInterface> op2) const;

  private:
    int mVal;
};


//#include "AttributePropagationIRInterface.hpp"

class 
SageIRInterface : public virtual OA::SSA::SSAIRInterface,
  public virtual OA::CFG::CFGIRInterfaceDefault,  
  public virtual OA::ICFG::ICFGIRInterface,
  public virtual OA::CallGraph::CallGraphIRInterface,
  public virtual OA::Activity::ActivityIRInterface,
  public virtual OA::Alias::AliasIRInterfaceDefault,
  public virtual OA::ReachDefs::ReachDefsIRInterface,
  public virtual OA::UDDUChains::UDDUChainsIRInterface,
  public virtual OA::XAIF::XAIFIRInterface,
  public virtual OA::DataFlow::ParamBindingsIRInterface,
  public virtual OA::SideEffect::InterSideEffectIRInterfaceDefault,
  public virtual OA::SideEffect::SideEffectIRInterface,
  public virtual OA::SideEffect::InterSideEffectIRInterface,
  public virtual OA::Linearity::LinearityIRInterface,
  public virtual OA::ReachConsts::ReachConstsIRInterface,
		  //  public virtual OA::AttributePropagation::AttributePropagationIRInterface,
  public virtual OA::DataDep::DataDepIRInterface,
  public virtual OA::Loop::LoopIRInterface,
  public virtual OA::Liveness::LivenessIRInterface,
  public virtual OA::ReachingDefs::ReachingDefsIRInterface
{
  friend class SageIRMemRefIterator;
  friend class FindCallsitesPass;
  friend class SgPtrAssignPairStmtIterator;
  friend class SgParamBindPtrAssignIterator;
  friend class ExprTreeTraversal;
  friend class NumberTraversal;
  friend class SageExprHandleIterator;
  
  public:
  //! Constructor.
  SageIRInterface(SgNode *root, 
                  std::vector<SgNode*> *na, 
                  bool use_persistent_handles = FALSE,
                  bool useVtableOpt = TRUE,
                  bool excludeInputFiles = FALSE);
  ~SageIRInterface();

    
  //! Given a subprogram return an IRStmtIterator for the entire
  //! subprogram
  OA::OA_ptr<OA::IRStmtIterator> getStmtIterator(OA::ProcHandle h); 
  
  //! Return an iterator over all of the callsites in a given stmt
  OA::OA_ptr<OA::IRCallsiteIterator> getCallsites(OA::StmtHandle h);

  //! get iterator over formal parameters in a function declaration 
  OA::OA_ptr<OA::IRFormalParamIterator> getFormalParamIterator(OA::SymHandle h); 

  //! Given a ProcHandle, return its SymHandle
  OA::SymHandle getProcSymHandle(OA::ProcHandle h);

  // helper routines
  OA::SymHandle getProcSymHandle(SgFunctionDeclaration *);
  OA::SymHandle getVarSymHandle(SgInitializedName *);
  OA::MemRefHandle getSymMemRefHandle(OA::SymHandle h);

  // ??
  OA::SymHandle getSymHandle(OA::ProcHandle h) {return getProcSymHandle(h);} //ask Michelle

  OA::SymHandle getSymHandle(OA::ExprHandle expr);

  //########################################################
  // Procedures and call sites
  //########################################################
  // Given a ProcHandle, return an OA::CFG::IRRegionStmtIterator for the
  // procedure. 
  OA::OA_ptr<OA::IRRegionStmtIterator> procBody(OA::ProcHandle h) ;

  // Get IRCallsiteParamIterator for a callsite.
  // Iterator visits actual parameters in called order.
  OA::OA_ptr<OA::IRCallsiteParamIterator> getCallsiteParams(OA::CallHandle h); 

  //########################################################
  // Statements: General
  //########################################################

  bool returnStatementsAllowed();

  OA::CFG::IRStmtType getCFGStmtType (OA::StmtHandle h);
  OA::StmtLabel getLabel (OA::StmtHandle h);

  //------------------------------
  // for compound statement. 
  //------------------------------
  OA::OA_ptr<OA::IRRegionStmtIterator> getFirstInCompound (OA::StmtHandle h);

  void createNodeArray(SgNode * root);
  void numberASTNodes(SgNode *astNode);
  SgNode * getNodePtr(OA::IRHandle h){if((int)h==0) return 0; else if(persistent_handles) return (*nodeArrayPtr)[h.hval()-1]; else return (SgNode*)h.hval();} //hvals start at 1
  long getNodeNumber(SgNode *);  //can be zero
  std::vector <SgNode *> * nodeArrayPtr;

  SgNode *getProject() { return wholeProject; }

  //------------------------------
  // for procedure, loop
  //------------------------------
private:
  SgNode * wholeProject;
  OA::OA_ptr<OA::IRRegionStmtIterator> body (OA::StmtHandle h);
  bool persistent_handles;
  bool mUseVtableOpt;
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
 
  /* Helper routine?
  // getAllMemRefs(OA::IRHandle h) is intended to be called by
  // ROSE, which may neither have nor know how to create a StmtHandle.
  // e.g., may_alias(AstNodePtr r1, AstNodePtr r2);
  OA::OA_ptr<OA::MemRefHandleIterator> getAllMemRefs(OA::IRHandle h);
  */

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
  /* If needed should be moved to helper routine area
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
              SgNode *node = getNodePtr(h);
              ROSE_ASSERT(node != NULL);
 
              /*! Loop increments are SgExpression's, but 
              they are given as a StmtHandle for CFGIRInterface
              fix. 10/26/06 PLM 
              */ 
              SgExpression * sgExp= isSgExpression(node);
              if( sgExp ) {
                 return sgExp->unparseToString();
              }

              SgStatement * sgstmt= isSgStatement(node);
              if ( sgstmt ) {
                  return sgstmt->unparseToString();
              }
              // We stash new expression and mallocs
              // into the statements of UnnamedRefs.
              SgNewExp *newExp = isSgNewExp(node);
              if ( newExp ) {
                  return newExp->unparseToString();
              }
              SgFunctionCallExp *funcCall = isSgFunctionCallExp(node);
              if ( funcCall ) {
                  return funcCall->unparseToString();
              }
              SgStringVal *stringVal = isSgStringVal(node);
              if ( stringVal ) {
                  return stringVal->unparseToString();
              }
              ROSE_ABORT();
          }
          else return ("NULL stmt handle\n");
      }
  std::string toString(const OA::ExprHandle h) 
      {
          SgExpression * ex=(SgExpression*)getNodePtr(h);
          if ( ex == NULL ) { return ("NULL ExprHandle\n"); }
	  //	  std::cout << "exprType = " << ex->sage_class_name() << std::endl;
          return ex->unparseToString();
    }
  std::string toString(const OA::CallHandle h);
  std::string toString(const OA::OpHandle h);
  std::string toString(const OA::MemRefHandle h);
  std::string toString(const OA::SymHandle h); 
  std::string toStringWithoutScope(const OA::SymHandle h); 
  std::string toStringWithoutScope(SgNode *node);
  std::string toString(const OA::ConstSymHandle h) {
    //-----
    // reasoning by BK:
    // when constructing an ExprTree, and deteriming if an ExprHandle
    // is a ConstSymHandle, at some point it determines whether the ExprHandle
    // isConst.  If it is, then it makes a ConstSymNode.  If it isn't, then
    // it makes a MemRefNode.  Therefore, we should be able to convert
    // our ConstSymHandle to a MemRefHandle and use toString(MemRefHandle)
    //-----
    SgNode * astNode;
    astNode = getNodePtr(h);
    OA::MemRefHandle mrh = getNodeNumber(astNode);
    return ( toString(mrh) );
    //return ""; 
  }
  std::string toString(const OA::ConstValHandle h);
  
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

  
  //-------------------------------------------------------------------------
  // AliasIRInterfaceDefault
  //-------------------------------------------------------------------------
    
  //! If this is a PTR_ASSIGN_STMT then return an iterator over MemRefHandle
  //! pairs where there is a source and target such that target
  OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator>
  getPtrAssignStmtPairIterator(OA::StmtHandle stmt);
  
  OA::OA_ptr<OA::Alias::ParamBindPtrAssignIterator>
  getParamBindPtrAssignIterator(OA::CallHandle call);

  // returns true if given symbol is a reference variable.
//  bool isRefParam(OA::SymHandle); has been deprecated

  // returns true if given MemRefExpr is a reference variable.
//  bool isReferenceExpr(OA::OA_ptr<OA::MemRefExpr>);

  // returns true if given symbol is a pointer variable.
//  bool isPointerVar(OA::SymHandle);
  
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
  //OA::OA_ptr<OA::IRSymIterator> getRefSymIterator(OA::ProcHandle h);

  //! returns true if given symbol is a parameter 
  bool isParam(OA::SymHandle);

  // return the formal parameter that an actual parameter is associated with 
  OA::SymHandle getFormalForActual(OA::ProcHandle caller, OA::CallHandle call, 
                                   OA::ProcHandle callee, OA::ExprHandle param);

  // Given an ExprHandle, return an ExprTree 
  OA::OA_ptr<OA::ExprTree> getExprTree(OA::ExprHandle h);

  //-------------------------------------------------------------------------
  // ActivityIRInterface
  //-------------------------------------------------------------------------

  //! Given a statement return a list to the pairs of 
  //! target MemRefHandle, ExprHandle where
  //! target = expr
  OA::OA_ptr<OA::AssignPairIterator> 
    getAssignPairIterator(OA::StmtHandle h);

  // Iterator over Expressions in the given Statement
  OA::OA_ptr<OA::ExprHandleIterator> getExprHandleIterator(OA::StmtHandle stmt);

  //! Return an iterator over all independent MemRefExpr for given proc
  OA::OA_ptr<OA::MemRefExprIterator> getIndepMemRefExprIter(OA::ProcHandle h);

  //! Return an iterator over all dependent MemRefExpr for given proc
  OA::OA_ptr<OA::MemRefExprIterator> getDepMemRefExprIter(OA::ProcHandle h);
  
  //! given a symbol return the size in bytes of that symbol
  int getSizeInBytes(OA::SymHandle h);

  //-------------------------------------------------------------------------
  // LinearityIRInterface
  //-------------------------------------------------------------------------
  
  //! get the operation type and returns a LinOpType
  OA::Linearity::LinOpType getLinearityOpType(OA::OpHandle op);


  //-------------------------------------------------------------------------
  // ReachConstsInterface 
  //-------------------------------------------------------------------------
  //! Given an OpHandle and two operands (unary ops will just
  //! use the first operand and the second operand should be NULL)
  //! return a ConstValBasicInterface
  OA::OA_ptr<OA::ConstValBasicInterface> evalOp(OA::OpHandle op,
      OA::OA_ptr<OA::ConstValBasicInterface> operand1,
      OA::OA_ptr<OA::ConstValBasicInterface> operand2);

  std::string
    toString(OA::OA_ptr<OA::ConstValBasicInterface> cvPtr);

  //! Given a ConstValHandle return an abstraction representing the
  //! constant value
  //! User must free the ConstValBasicInterface
  OA::OA_ptr<OA::ConstValBasicInterface> 
    getConstValBasic(OA::ConstValHandle c);

  OA::OA_ptr<OA::ConstValBasicInterface>
    getConstValBasic (unsigned int val); 

  OA::OA_ptr<OA::ConstValBasicInterface> 
      getConstValBasic(OA::ConstSymHandle c);

  int returnOpEnumValInt(OA::OpHandle op);

  //-------------------------------------------------------------------------
  // DataDepIRInterface
  //-------------------------------------------------------------------------
  int constValIntVal(OA::ConstValHandle h);
  OA::AffineExpr::OpType getOpType(OA::OpHandle h);
  OA::OA_ptr<OA::IdxExprAccessIterator>  getIdxExprAccessIter(ProcHandle p);

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
  void dump(OA::OA_ptr<OA::FieldAccess> memRefExp, std::ostream& os);
  void dump(OA::OA_ptr<OA::MemRefExpr> memRefExp, std::ostream &os);
  void dump(OA::OA_ptr<OA::AddressOf> memRefExp, std::ostream& os);
  void dump(OA::OA_ptr<OA::SubSetRef> memRefExp, std::ostream& os);

  string refTypeToString(OA::OA_ptr<OA::MemRefExpr> memRefExp);

  //-------------------------------------------------------------------------
  // Interface to ROSE
  //-------------------------------------------------------------------------
  OA::IRHandle getHandle(SgNode *astNode) { return getNodeNumber(astNode); }
  OA::MemRefHandle getMemRefHandle(SgNode *astNode) { 
    OA::MemRefHandle memRefHandle = (OA::MemRefHandle) 0;
    //if ( isMemRefNode(astNode) ) { // causes infinite recursion since
    //called within initMemRefs and calls initMemRefs
      memRefHandle = getNodeNumber(astNode);
    //}
    return memRefHandle;
  }
  OA::SymHandle getSymHandle(SgNode *astNode);
  OA::SymHandle getVTableBaseSymHandle(SgClassDefinition *classDefn);
  OA::CallHandle getCallHandle(SgNode *astNode) { 
      switch(astNode->variantT()) {
      case V_SgFunctionCallExp:
      case V_SgConstructorInitializer:
      case V_SgDeleteExp:
          {
              // We expect that a call handle is one of
              // these three cases.
	      break;
          }
      case V_Sg_File_Info:
          {
              // A SgFileInfo can represent the call to a constructor.
              break;
          }
      default:
          {
              std::cerr << "astNode type: " << astNode->sage_class_name();
	      std::cerr << " not expected to be represented by";
	      std::cerr << " a call handle!" << std::endl;
              ROSE_ABORT();
          }
      }
      OA::CallHandle retval = getNodeNumber(astNode);
      return retval;
  }
  OA::ProcHandle getProcHandle(SgFunctionDefinition *astNode);
  SgFunctionDefinition *getSgNode(OA::ProcHandle h);

  OA::CallHandle getProcExprHandle(SgNode *astNode);
  //  OA::ProcHandle getProcHandle(SgFunctionDeclaration *astNode);
  SgNode *getSgNode(OA::IRHandle h) { return getNodePtr(h); }
  bool isMemRefNode(SgNode *astNode);

  void verifyCallHandleType(OA::CallHandle call);
  void verifyStmtHandleType(OA::StmtHandle stmt);

  bool excludeInputFiles() { return mExcludeInputFiles; }

 protected:

  //! Return the attribute associated with a Sage node.
  AstAttributeMechanism &getAttribute(SgNode *n);

 private:
  bool mExcludeInputFiles;
  //-------------------------------------------------------------------------
  // Memory References
  //-------------------------------------------------------------------------
  //! traverses AST and initializes the maps involving MemRefHandles and MREs
  void initMemRefAndPtrAssignMaps();
  void findAllMemRefsAndPtrAssigns(SgNode *astNode, OA::StmtHandle stmt);

  //! finds the topmost MemRefHandle in the subtree rooted at the given node
  //! if the tree represents only an rvalue, then MemRefHandle(0) is returned
  //! unless the rvalue is an addressOf, which is assigned a MemRefHandle
  OA::MemRefHandle findTopMemRefHandle(SgNode *node);

  //! finds the topmost ExprHandle in the subtree rooted at the given node
  OA::ExprHandle findTopExprHandle(SgNode *node);

  //! Determine if all the MREs associated with a given memref are lval
  //  bool is_lval(OA::MemRefHandle memref);

  //! A utility function _only_ to be invoked from within makePtrAssignPair.
  void _makePtrAssignPair(OA::StmtHandle stmt,
                          OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                          OA::OA_ptr<OA::MemRefExpr> rhs_mre);

  void makePtrAssignPair(OA::StmtHandle stmt,
                         OA::MemRefHandle lhs_memref,
                         OA::MemRefHandle rhs_memref);

  void makePtrAssignPair(OA::StmtHandle stmt,
                         OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                         OA::MemRefHandle rhs_memref);

  void makePtrAssignPair(OA::StmtHandle stmt,
                         OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                         OA::OA_ptr<OA::MemRefExpr> rhs_mre);


  void makeParamPtrPair(OA::CallHandle call,
                       int formal,
                        OA::OA_ptr<OA::MemRefExpr> actual);

  //! Create an assignment pair, where the memrefs are not
  //! necessarily pointers.
  void makeAssignPair(OA::StmtHandle stmt,
                      OA::MemRefHandle lhs_memref,
                      OA::ExprHandle rhs_expr);

  std::string findFieldName(OA::MemRefHandle memref);

  void createImplicitPtrAssignPairsForVirtualMethods(OA::StmtHandle stmt,
          OA::OA_ptr<OA::MemRefExpr> lhsMRE,
          SgClassDefinition *classDefinition,
          std::list<SgMemberFunctionDeclaration *> &visitedVirtualMethods);

  void createImplicitPtrAssignPairsForDynamicObjectAllocation(
          OA::StmtHandle stmt, OA::OA_ptr<OA::MemRefExpr> lhs_mre, 
          OA::OA_ptr<OA::MemRefExpr> rhs_mre);


   void createImplicitPtrAssignPairsForObjectDeclaration(OA::StmtHandle stmt, 
                                             OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                                             SgInitializedName *initName);

   void createImplicitPtrAssignPairsForClassDefinition(OA::StmtHandle stmt,
               SgClassDefinition *topClassDefinition,
               SgClassDefinition *classDefinition,
               std::list<SgMemberFunctionDeclaration *> &visitedVirtualMethods);

  bool createMemRefExprsForPtrArith(SgExpression* node, 
                                    SgExpression* child, OA::StmtHandle stmt,
                                    bool removeChild);

  void createUseDefForVarArg(OA::MemRefHandle memref,
                             OA::MemRefHandle valist_memref);

  OA::OA_ptr<OA::MemRefExpr> 
  createConstructorInitializerReceiverMRE( SgConstructorInitializer *ctorInitializer);

  OA::SymHandle getThisFormalSymHandle(SgNode *astNode);

  //! Given a SgNode return a symbol handle to represent the
  //! corresponding SgThisExp.  This symbol handle references
  //! a SgFunctionParameterList.
  OA::SymHandle getThisExpSymHandle(SgNode *node);

  // assumption is that StmtHandles and MemRefHandles are unique across
  // different program and procedure contexts for which analysis is being
  // currently performed
  std::map<OA::StmtHandle,std::set<OA::MemRefHandle> >
    mStmt2allMemRefsMap;

  std::map<OA::StmtHandle,std::set<OA::ExprHandle> >
    mStmt2allExprsMap;

  std::map<OA::MemRefHandle,OA::StmtHandle> mMemRef2StmtMap;

  std::map<OA::MemRefHandle,std::set<OA::OA_ptr<OA::MemRefExpr> > >
    mMemref2mreSetMap;

  std::map<OA::CallHandle,OA::OA_ptr<OA::MemRefExpr> > mCallToMRE;

  std::map<OA::OA_ptr<OA::MemRefExpr>, typeEnum >
    mMre2TypeMap;

  //! given a memory reference handle and a statement handle asscociate them
  //! in the mStmt2allMemRefsMap and mMemRef2StmtMap maps
  void relateMemRefAndStmt(OA::MemRefHandle mref, OA::StmtHandle stmt);

  //! remove relations between the specified memref and stmt in the
  //! mStmt2allMemRefsMap and mMemRef2StmtMap maps
  void deleteMemRefStmtRelation(OA::MemRefHandle mref, OA::StmtHandle stmt);

  public:
  //! Temporary hack to get a MemRefHandle given a MemRefExpr, code
  //! iterates through the 'MemRefHandle -> set<MemRefExpr>' map until
  //! it finds the one.
  //OA::MemRefHandle getMemRefHandle(OA::OA_ptr<OA::MemRefExpr>);

  OA::StmtHandle getStmt(OA::MemRefHandle mref);

  private:

  // why would we need this one?
  //std::map<OA::OA_ptr<OA::MemRefExpr>,OA::MemRefHandle >
  //  mMre2MemrefMap;

  //-------------------------------------------------------------------------
  // Pointer Assignments
  //-------------------------------------------------------------------------
  //! traverses AST and initializes the pointer assignments that occur
  //! in statements and at parameter bindings
  //void initPointerAssignMaps();
  //void findAllPtrAssignAndParamBindPairs(SgNode *astNode, OA::StmtHandle stmt);

  void createParamBindPtrAssignPairs(OA::StmtHandle stmt, SgNode *node);

  OA::OA_ptr<OA::MemRefExpr> 
  applyReferenceConversionRules2And4(OA::StmtHandle stmt,
                                     SgType *lhs_type,
                                     SgNode *lhs,
                                     SgNode *rhs,
                                     OA::MemRefHandle rhs_memref,
                                     OA::OA_ptr<OA::MemRefExpr> rhs_mre);

  std::map<OA::StmtHandle,
           std::set<
               std::pair<OA::OA_ptr<OA::MemRefExpr>, 
                         OA::OA_ptr<OA::MemRefExpr> > > > mStmtToPtrPairs;

  std::map<OA::StmtHandle,
           std::set<
               std::pair<OA::MemRefHandle, 
                         OA::ExprHandle> > > mStmtToAssignPairs;

  std::map<OA::CallHandle,
           std::set<
               std::pair<int, 
                         OA::OA_ptr<OA::MemRefExpr> > > > mCallToParamPtrPairs;

  //-------------------------------------------------------------------------
  // Loop Assignments
  //-------------------------------------------------------------------------
  public:
  OA_ptr<std::list<OA_ptr<Loop::LoopAbstraction> > >
    gatherLoops(const ProcHandle &proc);

  StmtHandle findEnclosingLoop(const StmtHandle &stmt);

  //-------------------------------------------------------------------------
  // Helper data structures and methods
  //-------------------------------------------------------------------------
  public:
  // returns an iterator over all symbols in the program
  OA::OA_ptr<SageIRSymIterator> getSymIterator(SgProject *proj);

  private:
  // List of all SgFunctionDeclarations in the program, stored as
  // a map from the first non-defining declaration (if non-NULL)
  // to the defining declaration.
  std::map<SgFunctionDeclaration *, SgFunctionDeclaration *> mFunctions;

  //! Returns true if the contructor initializer creates a base type.
  bool createsBaseType(SgConstructorInitializer *ctorInitializer) const;

  // Strip off any leading SgCastExps/SgAssignInitializers in 
  // the tree root at node.
  SgNode *lookThroughCastExpAndAssignInitializer(SgNode *node); 

};

#define OA_VTABLE_STR "__oa_vtable_ptr"

#endif

