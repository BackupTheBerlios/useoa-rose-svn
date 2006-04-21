#ifndef MemSage2OA_H
#define MemSage2OA_H

#include <list>
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
//#include <OpenAnalysis/IRInterface/MemRefExprIRShortCircuit.hpp>
//#include <OpenAnalysis/MemRefExpr/MemRefExprAll.hpp>
#include <config.h>
#include "rose.h"
#include "../ArrayReferenceVisitor.hpp"

#include "Sage2OA.h"

class SynthesizedRefExpInfo : SgSynthesizedAttribute
{
  public:
    SynthesizedRefExpInfo(): m_loc_id_symbol(NULL), m_num_derefs(0), m_ignore_me(false), m_parent_partial(false){}
    SgInitializedName * loc_id_symbol(){return m_loc_id_symbol;}
    void set_loc_id_symbol(SgInitializedName * in){m_loc_id_symbol=in;}
    int num_derefs(){return m_num_derefs;}
    void set_num_derefs(int i){m_num_derefs=i;}
    bool ignore_me(){return m_ignore_me;}
    void set_ignore_me(){m_ignore_me=true;}
    void unset_ignore_me(){m_ignore_me=false;}
    void set_parent_partial(){m_parent_partial=true;}
    void unset_parent_partial(){m_parent_partial=false;}
    bool parent_partial(){return m_parent_partial;}
  private:
    SgInitializedName* m_loc_id_symbol;
    int m_num_derefs;
    bool m_ignore_me;
    bool m_parent_partial;
  
};

class SageMemRefAttribute : public AstAttribute, public SgAttribute
{
  public:
    int lhs;
    int rhs;
    int num_deref;
    OA::MemRefExpr::MemRefType type;
};

class InheritedTopRefInfo : SgInheritedAttribute 
{
	public:
	InheritedTopRefInfo(): m_lhs(false), m_rhs(false), m_found_top_ref(false) {}
  void set_lhs(){m_lhs=true;}
  void set_rhs(){m_rhs=true;}
  void set_found_top_ref(){m_found_top_ref=true;}
  void unset_lhs(){m_lhs=false;}
  void unset_rhs(){m_rhs=false;}
  void unset_found_top_ref(){m_found_top_ref=false;}
  bool is_lhs(){return m_lhs;}
  bool is_rhs(){return m_rhs;}
  bool found_top_ref(){return m_found_top_ref;}
  
  private:
    bool m_lhs;
    bool m_rhs;
    bool m_found_top_ref;
};

class SageTopRefTraversal
  : public SgTopDownProcessing<InheritedTopRefInfo>
{
    private:
      OA::OA_ptr<std::list<OA::MemRefHandle> >mMemRefList;
    public:
      InheritedTopRefInfo evaluateInheritedAttribute(SgNode * n, InheritedTopRefInfo);
      SageTopRefTraversal(OA::OA_ptr<std::list<OA::MemRefHandle> > inMemRefList,
                          SageIRInterface * in):mMemRefList(inMemRefList), ir(in){}
      SageIRInterface * ir;  
};


//! Enumerate all the statements in a program 
// This iterator DOES step into compound statements.
class SageStmtTraversal
  : public SgSimpleProcessing
{
    public:
      SageStmtTraversal(OA::OA_ptr<std::list<OA::StmtHandle> > inStmtList,
			SageIRInterface * in): ir(in), mStmtList(inStmtList) {}
      void visit(SgNode * astNode);
      bool DeclWithoutInit(SgStatement *);
      SageIRInterface * ir;          
    private:
      OA::OA_ptr<std::list<OA::StmtHandle> > mStmtList;
};
  

/* no longer needed?
class MemSageIRStmtIterator : public OA::IRStmtIterator {
public:
  MemSageIRStmtIterator(std::list<OA::StmtHandle> &slist);
  //MemSageIRStmtIterator(): mSlist(emptyStmtList) { mValid = false; }
  ~MemSageIRStmtIterator() { };

  OA::StmtHandle current();
  bool isValid() { return (mValid && (mStmtIter != mEnd)); }
        
  void operator++();

  void reset();

private:
  std::list<OA::StmtHandle>::iterator mEnd;
  std::list<OA::StmtHandle>::iterator mBegin;
  std::list<OA::StmtHandle>::iterator mStmtIter;
  std::list<OA::StmtHandle>& mSlist;  
  bool mValid;
};
*/


#if 0
// It looks like this interface went away? 
// Possibly replaced by MemRefHandleIterator?  bwhite
//! Enumerate all the top memory references in a stmt
class MemSageIRTopMemRefIterator : public OA::IRTopMemRefIterator 
{
  public:
    MemSageIRTopMemRefIterator(OA::OA_ptr<std::list<OA::MemRefHandle> >   mrlist);
    MemSageIRTopMemRefIterator() { mValid = false; thelist=NULL; mLength=0; mIndex=0;}
    ~MemSageIRTopMemRefIterator() { };
  
    OA::MemRefHandle current() const; 
    bool isValid() const        { return (mValid && /*(mMemRefIter!=thelist->end())*/  mIndex<mLength); }        
          
    void operator++();
    //void operator++(int) { ++*this; } ;
  
    void reset();
  private:
    //std::list<OA::MemRefHandle>::iterator mEnd;
    //std::list<OA::MemRefHandle>::iterator mBegin;
    std::list<OA::MemRefHandle>::iterator mMemRefIter;
    OA::OA_ptr<std::list<OA::MemRefHandle> > thelist;
    int mLength;
    int mIndex;
    bool mValid;
};
#endif

// This is similar to Open64IRMemRefIterator.
class SageIRMemRefIterator : public OA::MemRefHandleIterator 
{
  public:
    SageIRMemRefIterator(OA::IRHandle h, SageIRInterface *ir);
    SageIRMemRefIterator() : mValid(false) { }
    virtual ~SageIRMemRefIterator() { };
  
    virtual OA::MemRefHandle current() const; 
    virtual bool isValid() const { 
      return ( mValid && ( mMemRefIter != mEnd ) ); 
    }
          
    virtual void operator++();
    virtual void reset();

  private:
    void create(OA::IRHandle h);

    void handleDefaultCase(SgNode *astNode,
			   std::list<OA::MemRefHandle>& memRefs,
			   unsigned flags,
			   unsigned &synthesizedFlags);

    std::list<OA::MemRefHandle>*
      findAllMemRefsAndMemRefExprs(SgNode *astNode);

    std::list<OA::OA_ptr<OA::MemRefExpr> >
      findAllMemRefsAndMemRefExprs(SgNode *astNode,
				   std::list<OA::MemRefHandle>& memRefs,
				   unsigned inheritedFlags,
				   unsigned &synthesizedFlags);
    
    // takeAddressOfMre creates a MemRefExpr that represents the
    // address of mre.  
    OA::OA_ptr<OA::MemRefExpr> takeAddressOfMre(OA::OA_ptr<OA::MemRefExpr> mre);

    // Return true if mre computes an lvalue-- that is, we could take
    // its address.
    bool mreComputesLValue(OA::OA_ptr<OA::MemRefExpr> mre);

    // Apply the reference conversion rules.
    void applyReferenceConversionRules(OA::OA_ptr<OA::MemRefExpr> mre,
				       SgNode *astNode, 
				       bool appearsOnRhsOfRefAssignment,
				       bool hasRhsThatComputesLValue,
				       bool hasRhsThatDoesntComputeLValue,
				       bool initializedRef,
				       std::list<OA::OA_ptr<OA::MemRefExpr> > &convertedMemRefs);


    std::list<OA::MemRefHandle> mMemRefList;
    
    std::list<OA::MemRefHandle>::iterator mEnd;
    std::list<OA::MemRefHandle>::iterator mBegin;
    std::list<OA::MemRefHandle>::iterator mMemRefIter;
    bool mValid;
    SageIRInterface *mIR;
};

// This is identical to Open64MemRefHandleIterator.
class SageMemRefHandleIterator
: public OA::IRHandleListIterator<OA::MemRefHandle>,
  public virtual OA::MemRefHandleIterator
{
 public:
  SageMemRefHandleIterator (OA::OA_ptr<std::list<OA::MemRefHandle> > pList)\

    : OA::IRHandleListIterator<OA::MemRefHandle>(pList) {}
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


class memRefExpList : public AstAttribute,
                      public std::list<OA::OA_ptr<OA::MemRefExpr > >
{
 public:
  memRefExpList() { }

  memRefExpList(const memRefExpList &list) {
    memRefExpList::const_iterator it = list.begin();
    for (; it != list.end(); ++it)
      push_back(*it);
  }
};

OA::OA_ptr<memRefExpList> 
getMemRefs(SgNode *node, SageIRInterface &irInterface);

#endif

