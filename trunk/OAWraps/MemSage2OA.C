#include "MemSage2OA.h"

//! memRefExpWrapper is used to pass MemRefExprs up the AST
//! during a bottom-up traversal.  The node member variable
//! indicates the SgNode at which the MemRefExpr originated.
//! An alternative strategy places a list of MemRefExprs
//! within a node attribute.  This isn't a good idea
//! because we must clear these attribute lest they
//! interfere with subsequent invocations of the traversal.
class memRefExpWrapper
{
 public:
  
  //! Constructor.
  memRefExpWrapper(SgNode *node, OA::OA_ptr<OA::MemRefExpr> memRefExpr)
    : mNode(node), mMemRefExpr(memRefExpr) { }

  //! Copy constructor.
  memRefExpWrapper(const memRefExpWrapper &wrapper)
    : mNode(wrapper.mNode), mMemRefExpr(wrapper.mMemRefExpr) { }

  //! Return the node at which the MemRefExpr occurs.
  SgNode *getNode() const { return mNode; }

  //! Return the memory reference expression.
  OA::OA_ptr<OA::MemRefExpr> getMemRefExpr() const { return mMemRefExpr; }

 private:
  //! The node at which the MemRefExpr occurs.
  SgNode                     *mNode;

  //! The memory reference expression.
  OA::OA_ptr<OA::MemRefExpr>  mMemRefExpr;
};

class memRefExpWrapperList : public AstAttribute,
                             public std::list<OA::OA_ptr<memRefExpWrapper > >
{
 public:
  memRefExpWrapperList() { }

  memRefExpWrapperList(const memRefExpWrapperList &list) {
    memRefExpWrapperList::const_iterator it = list.begin();
    for (; it != list.end(); ++it)
      push_back(*it);
  }
};

class SageRefExpFlagsAttr : public AstAttribute {
 public:
  
  SageRefExpFlagsAttr() : 
    mPartial(false),
    mFlags(0) 
  { }

  SageRefExpFlagsAttr(const SageRefExpFlagsAttr &info) :
    mPartial(info.mPartial),
    mFlags(info.mFlags)
  { }

  void setPartial(bool partial) { mPartial = partial; }
  bool getPartial() const { return mPartial; }

  // See OpenAnalysis/MemRefExpr/MemRefExpr.hpp
  enum SageRefExpFlagType { 
    expectUse    = 1,
    expectDef    = 2,
    expectUseDef = 4,  // use and then define
    expectDefUse = 8,  // define and then use
    expectAddrOf = 16,
    ignore       = 32  // do not treat node as def/use
  };

  void setFlags(int flags) { mFlags = flags; }

  int  getFlags() const { return mFlags; }

  void addFlag(const SageRefExpFlagType &flag) {

    switch(flag) {

    case expectUse:
      {

	if ( ( mFlags & expectDef ) != 0 ) {
	  mFlags &= ~expectDef;
	  mFlags |= expectDefUse;
	} else if ( ( ( mFlags & expectDefUse ) != 0 ) ||
		    ( ( mFlags & expectUseDef ) != 0 ) ) {
	  ;
	} else {
	  mFlags |= expectUse;
	}
	
	break;
      }

    case expectDef:
      {
	if ( ( mFlags & expectUse ) != 0 ) {
	  mFlags &= ~expectUse;
	  mFlags |= expectUseDef;
	} else if ( ( ( mFlags & expectDefUse ) != 0 ) ||
		    ( ( mFlags & expectUseDef ) != 0 ) ) {
	  ;
	} else {
	  mFlags |= expectDef;
	}

	break;
      }

    case expectUseDef:
      {
	mFlags &= ~expectUse;
	mFlags &= ~expectDef;

	if ( ( mFlags & expectDefUse ) == 0 ) 
	  mFlags |= expectUseDef;

	break;
      }
      
    case expectDefUse:
      {
	mFlags &= ~expectUse;
	mFlags &= ~expectDef;

	if ( ( mFlags & expectUseDef ) == 0 ) 
	  mFlags |= expectDefUse;

	break;
      }

    case expectAddrOf:
      {
	mFlags |= expectAddrOf;

	break;
      }

    case ignore:
      {
	mFlags |= ignore;

	break;
      }

    default:
      {
	break;
      }

    }

  }

  void removeFlag(const SageRefExpFlagType &flag) {

    switch(flag) {

    case expectUse:
      {
	mFlags &= ~expectUse;

	if ( ( ( mFlags & expectDefUse ) != 0 ) ||
	     ( ( mFlags & expectUseDef ) != 0 ) ) {
	  mFlags |= expectDef;
	}
	
	break;
      }

    case expectDef:
      {
	mFlags &= ~expectDef;

	if ( ( ( mFlags & expectDefUse ) != 0 ) ||
	     ( ( mFlags & expectUseDef ) != 0 ) ) {
	  mFlags |= expectDef;
	}

	break;
      }

    case expectUseDef:
    case expectDefUse:
      {
	mFlags &= ~expectUse;
	mFlags &= ~expectDef;
	mFlags &= ~expectUseDef;
	mFlags &= ~expectDefUse;

	break;
      }

    case expectAddrOf:
      {
	mFlags &= ~expectAddrOf;

	break;
      }

    case ignore:
      {
	mFlags &= ~ignore;

	break;
      }

    default:
      {
	break;
      }

    }

  }

 private:
  bool           mPartial;
  int            mFlags;
};

//-----------------------------------------------------------------------------
// Convenience functions to manipulate a node's SageRefExpFlagsAttr 
// attribute.
//-----------------------------------------------------------------------------

static SageRefExpFlagsAttr *
getOrCreateRefExpFlagsAttr(SgNode *astNode)
{
  SageRefExpFlagsAttr *ret = NULL;
 
  if (!astNode->attribute.exists("SageRefExpFlagsAttr")) {
    ret = new SageRefExpFlagsAttr;
    ROSE_ASSERT(ret != NULL);
    astNode->attribute.set("SageRefExpFlagsAttr", ret);
    return ret;
  }
 
  AstAttribute* attr = astNode->attribute["SageRefExpFlagsAttr"];
  ROSE_ASSERT(attr != NULL);
   
  ret = dynamic_cast<SageRefExpFlagsAttr *>(attr); 
  return ret; 

}

static void
removeRefExpFlagsAttr(SgNode *astNode)
{
  if (!astNode->attribute.exists("SageRefExpFlagsAttr")) return;
 
  astNode->attribute.remove("SageRefExpFlagsAttr");
}

static SageRefExpFlagsAttr *
getRefExpFlagsAttr(SgNode *astNode)
{
  SageRefExpFlagsAttr *ret = NULL;
 
  if (!astNode->attribute.exists("SageRefExpFlagsAttr")) return ret; 
 
  AstAttribute* attr = astNode->attribute["SageRefExpFlagsAttr"];
  if (attr == NULL) return ret; 
   
  ret = dynamic_cast<SageRefExpFlagsAttr *>(attr); 
  return ret; 

}

static void setFlags(SgNode *astNode, int flags)
{
  // Retrieve the memory reference expression attribute
  // from this node.
  SageRefExpFlagsAttr *refExpFlagsAttr =
    getOrCreateRefExpFlagsAttr(astNode);
  ROSE_ASSERT(refExpFlagsAttr != NULL);

  refExpFlagsAttr->setFlags(flags);
}

static int getFlags(SgNode *astNode)
{
  // Retrieve the memory reference expression attribute
  // from this node.
  SageRefExpFlagsAttr *refExpFlagsAttr =
    getRefExpFlagsAttr(astNode);
  ROSE_ASSERT(refExpFlagsAttr != NULL);

  return refExpFlagsAttr->getFlags();
}

static int hasFlagsSet(SgNode *astNode)
{
  // Retrieve the memory reference expression attribute
  // from this node.
  SageRefExpFlagsAttr *refExpFlagsAttr =
    getRefExpFlagsAttr(astNode);
  return(refExpFlagsAttr != NULL);
}

static bool getAddressTaken(SgNode *astNode)
{
  int flags = getFlags(astNode);

  return ( flags & SageRefExpFlagsAttr::expectAddrOf );
}

static bool getPartial(SgNode *astNode)
{
  // Retrieve the memory reference expression attribute
  // from this node.
  SageRefExpFlagsAttr *refExpFlagsAttr =
    getRefExpFlagsAttr(astNode);
  ROSE_ASSERT(refExpFlagsAttr != NULL);

  return refExpFlagsAttr->getPartial();
}

static void addFlag(SgNode *astNode, 
		    const SageRefExpFlagsAttr::SageRefExpFlagType &flag)
{
  // Retrieve the memory reference expression attribute
  // from this node.
  SageRefExpFlagsAttr *refExpFlagsAttr =
    getOrCreateRefExpFlagsAttr(astNode);
  ROSE_ASSERT(refExpFlagsAttr != NULL);

  refExpFlagsAttr->addFlag(flag);
}

static void removeFlag(SgNode *astNode, 
		       const SageRefExpFlagsAttr::SageRefExpFlagType &flag)
{
  // Retrieve the memory reference expression attribute
  // from this node.
  SageRefExpFlagsAttr *refExpFlagsAttr =
    getRefExpFlagsAttr(astNode);
  ROSE_ASSERT(refExpFlagsAttr != NULL);

  refExpFlagsAttr->removeFlag(flag);
}

static void setPartial(SgNode *astNode, bool partial)
{
  // Retrieve the memory reference expression attribute
  // from this node.
  SageRefExpFlagsAttr *refExpFlagsAttr =
    getRefExpFlagsAttr(astNode);
  ROSE_ASSERT(refExpFlagsAttr != NULL);

  return refExpFlagsAttr->setPartial(partial);
 }

static OA::MemRefExpr::MemRefType flagsToMemRefType(SgNode *node)
{
  int flags = getFlags(node);

  // By default, we assume that any node passed to this function
  // represents a USE, unless it has flags that indicate otherwise.
  // Therefore, we only invoke this function when we expect a
  // DEF and/or USE.
  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
  
  if ( ( flags & SageRefExpFlagsAttr::expectDef ) != 0 )
    memRefType = OA::MemRefExpr::DEF;
  else if ( ( flags & SageRefExpFlagsAttr::expectDefUse ) != 0 )
    memRefType = OA::MemRefExpr::DEFUSE;
  else if ( ( flags & SageRefExpFlagsAttr::expectUseDef ) != 0 )
    memRefType = OA::MemRefExpr::USEDEF;
 
  return memRefType;
}



/*
 * usedInConditionalExp returns true if node's parent
 * is a SgConditionalExp and it is that parent's
 * true or false expression.
 */
static
bool usedInConditionalExp(SgNode *node)
{
  if ( node == NULL ) return false;

  SgNode *parent = node->get_parent();
  ROSE_ASSERT(parent != NULL);

  SgConditionalExp *conditionalExp = isSgConditionalExp(parent);
  if ( conditionalExp == NULL ) return false;

  return ( ( conditionalExp->get_true_exp() == node ) ||
	   ( conditionalExp->get_false_exp() == node ) );
}

/*
 * usedInExpressionContext returns true if node is accessed
 * within an expression.  Accesses within an argument
 * list (without an intervening SgExpression) are not considered 
 * expression accesses.  Further,
 * accesses within a conditionalExpression's true/false
 * expression is only considered an expression access
 * if the node's parent is usedInExpressionContext.
 * Note one exceptional case in which we return true,
 * although it's parent is a scope statement and not
 * an expression:  a SgVariableDeclaration may have
 * DEFs and should not be ignored.
 */
static
bool usedInExpressionContext(SgNode *node)
{
  if ( node == NULL ) return false;

  SgNode *parent = node->get_parent();
  ROSE_ASSERT(parent != NULL);

  SgExprListExp *exprListExp = isSgExprListExp(parent);
  
  if ( exprListExp != NULL ) {

    SgNode *grandParent = exprListExp->get_parent();
    ROSE_ASSERT(grandParent != NULL);

    if ( isSgFunctionCallExp(grandParent) ) {

      // Node is in an argument list and is therefore
      // not considered to be used in an expression.
      return false;

    }

    if ( usedInConditionalExp(exprListExp) ) {

      // The parent of this node is used in a SgConditionalExp.
      // Recurse to see if the grand parent is used in
      // an expression.
      return usedInExpressionContext(grandParent);

    }

  } else {

    // The node is not used (directly) in an argument list.
    
    if ( usedInConditionalExp(node) ) {

      // The node is used in a SgConditionalExp.
      // Recurse to see if the parent is used in
      // an expression.
      return usedInExpressionContext(parent);

    }

  }

  // Base case:  The node is not used in an argument list
  // nor is it used in a conditional expression.  If it 
  // is an expression, then return true;
  if ( isSgExpression(node) ) return true;

  // Also return true if it is a SgVariableDeclaration.
  if ( isSgVariableDeclaration(node) ) return true;

  // If this is a SgInitializedName we need to
  // differentiate between the cases in which its
  // parent is a SgVariableDeclaration and 
  // all others.
  if ( isSgInitializedName(node) ) 
    return usedInExpressionContext(parent);

  return false;

}

static 
bool isMalloc(SgFunctionCallExp *functionCallExp)
{
  if( functionCallExp == NULL) return false;

  SgExpression *expression = functionCallExp->get_function();
  ROSE_ASSERT(expression != NULL);

  SgFunctionRefExp *functionRefExp = 
    isSgFunctionRefExp(expression);

  if (functionRefExp == NULL) return false;

  // Found a standard function reference.  
  SgFunctionDeclaration *functionDeclaration = 
    functionRefExp->get_symbol_i()->get_declaration(); 
  ROSE_ASSERT(functionDeclaration != NULL);
  
  SgName name = functionDeclaration->get_name();
  return ( !strcmp("malloc", name.str() ) );
}


OA::OA_ptr<OA::MemRefExpr> 
copyBaseMemRefExpr(OA::OA_ptr<OA::MemRefExpr> memRefExp)
{
  OA::OA_ptr<OA::MemRefExpr> memRefExpr;

  if ( memRefExp->isaNamed() ) {

    OA::OA_ptr<OA::NamedRef> namedRef = memRefExp.convert<OA::NamedRef>();
    ROSE_ASSERT(!namedRef.ptrEqual(0));

    memRefExpr = new OA::NamedRef(*namedRef);

  } else if ( memRefExp->isaUnnamed() ) {

    OA::OA_ptr<OA::UnnamedRef> unnamedRef = memRefExp.convert<OA::UnnamedRef>();
    ROSE_ASSERT(!unnamedRef.ptrEqual(0));

    memRefExpr = new OA::UnnamedRef(*unnamedRef);

  } else if ( memRefExp->isaUnknown() ) {

    OA::OA_ptr<OA::UnknownRef> unknownRef = memRefExp.convert<OA::UnknownRef>();
    ROSE_ASSERT(!unknownRef.ptrEqual(0));

    memRefExpr = new OA::UnknownRef(*unknownRef);

  } else if ( memRefExp->isaRefOp() ) {

    OA::OA_ptr<OA::RefOp> refOp = memRefExp.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = refOp->getMemRefExpr();
    memRefExpr = copyBaseMemRefExpr(baseMemRefExpr);

  } else {

    cerr << "Unknown memRefExp type!" << endl;
    ROSE_ABORT();

  }

  return memRefExpr;
}

bool isFunctionCallWithPtrReturn(SgNode *node)
{
  SgFunctionCallExp *functionCallExp = 
    isSgFunctionCallExp(node);

  if ( functionCallExp == NULL ) return false;

  SgType *retType = functionCallExp->get_type();
  ROSE_ASSERT(retType != NULL);

  return ( isSgPointerType(retType) );
}

bool SageStmtTraversal::DeclWithoutInit(SgStatement * stmt)
{
  SgVariableDeclaration * vardecl=isSgVariableDeclaration(stmt);
  if(!vardecl)
    return false;
  SgInitializedName * nmi=*(vardecl->get_variables().begin());
		SgInitializer * varinit=nmi->get_initializer();
		//printf("vardecl is %s type is %s varinitroot is %x\n", vardecl->unparseToString().c_str(), nmlst.front().get_type()->sage_class_name(), varinit);
  if(varinit)
    return false;

  //this should work but does not
//it looks like get_initializer always return false;
/*
  for(SgInitializedNameList::iterator iter=nmlst.begin(); iter!=nmlst.end(); iter++)
  {
    SgInitializedName initname=*iter;
    printf("init name is %s\n", initname.get_name().str());
    if(initname.get_initializer())
    {
      printf("found initializer\n");
      return false;
    }
  }
  */
  return true;
}

void SageStmtTraversal::visit(SgNode * astNode)
{
  SgStatement * stmt=isSgStatement(astNode);
  if(stmt)
  {
    //for the purpose of stmt iterator for memory references, we can skip certain types of statements
    //this can be changed easily, need to talk to Michelle
    if(
      (!(isSgBasicBlock(stmt))) &&
      (!(isSgFunctionDefinition(stmt))) &&
      (!(isSgFunctionDeclaration(stmt))) &&
    (!(DeclWithoutInit(stmt))) &&
      (!(isSgFunctionParameterList(stmt)))
      )
      mStmtList->push_back(OA::StmtHandle((OA::irhandle_t)(ir->getNodeNumber(stmt))));
  }
}

/*
//-----------------------------------------------------------------------------
// Stmt Iterator
//-----------------------------------------------------------------------------
MemSageIRStmtIterator::MemSageIRStmtIterator(std::list<OA::StmtHandle>& slist): mSlist(slist)
{
    mStmtIter = slist.begin();
    mEnd = slist.end();
    mBegin = slist.begin();
    mValid = true;
}
        
OA::StmtHandle MemSageIRStmtIterator::current()
{
    if (mValid) { 
        return (*mStmtIter); 
    } else { 
        return OA::StmtHandle(0); 
    }
}

void MemSageIRStmtIterator::operator++()
{
    if (mValid) {
        mStmtIter++;
    }
}

void MemSageIRStmtIterator::reset()
{
    mStmtIter = mBegin;
}
*/

#if 0
//-----------------------------------------------------------------------------
// Memory Reference Iterator
//-----------------------------------------------------------------------------
MemSageIRTopMemRefIterator::MemSageIRTopMemRefIterator(OA::OA_ptr<std::list<OA::MemRefHandle> >  mrlist): thelist(mrlist)
{
    mMemRefIter = mrlist->begin();
    //mEnd = mrlist.end();
    //mBegin = mrlist.begin();
    if(mrlist->size()!=0)
    {
      mValid = true;
      mLength=mrlist->size();
      mIndex=0;
    }
    else
    {
      mValid = false;
      mLength=0;
      mIndex=0;
    }
  }
        
OA::MemRefHandle MemSageIRTopMemRefIterator::current() const
{
    if (mValid) { 
        return (*mMemRefIter); 
    } else { 
        return OA::MemRefHandle(0); 
    }
}

void MemSageIRTopMemRefIterator::operator++()
{
    if (mValid) 
    {
      mMemRefIter++;
      mIndex++;
      if(mIndex<mLength)
        mValid=true;
      else
        mValid=false;
    }
}

void MemSageIRTopMemRefIterator::reset()
{
    mMemRefIter = thelist->begin();
    mIndex=0;
}
#endif


std::map<OA::StmtHandle,std::set<OA::MemRefHandle> >
SageIRInterface::sStmt2allMemRefsMap;

std::map<OA::MemRefHandle,OA::StmtHandle> SageIRInterface::sMemRef2StmtMap;

std::map<OA::MemRefHandle,set<OA::OA_ptr<OA::MemRefExpr> > >
SageIRInterface::sMemref2mreSetMap;

SageIRMemRefIterator::SageIRMemRefIterator(OA::StmtHandle h, 
					   SageIRInterface *ir)
  : mValid(true), mIR(ir)
{
  create(h);
  reset();
}

OA::MemRefHandle
SageIRMemRefIterator::current() const 
{
  if (mValid) {
    return (*mMemRefIter);
  } else {
    return OA::MemRefHandle(0);
  }
}

void
SageIRMemRefIterator::operator++()
{
  if (mValid) mMemRefIter++;
}

void
SageIRMemRefIterator::reset()
{
  mMemRefIter = mMemRefList.begin();
  mEnd = mMemRefList.end();
  mBegin = mMemRefList.begin();
}

/*! this method sets up sMemRef2StmtMap, sStmt2MemRefSet?, and
    sMemRef2mreSetMap?
    Is only way to get MemRefHandle's therefore no queries should
    be logically made on MemRefHandle's before one of these
    iterators has been requested.
    Very similar to Open64IRMemRefIterator::create.
*/
void
SageIRMemRefIterator::create(OA::StmtHandle stmt)
{
  SgNode *node = mIR->getNodePtr(stmt);
  ROSE_ASSERT(node != NULL);

  // if haven't already determined the set of memrefs for this stmt
  // then do so by finding all the top memory references and then
  // initializing the mapping of MemRefHandle's to a set of MemRefExprs,
  // and based off that map get all the MemRefHandle's
  if (SageIRInterface::sStmt2allMemRefsMap[stmt].empty() ) {

    // get all the top memory references
    list<SgNode *>* topMemRefs = findTopMemRefs(node);
    for (list<SgNode *>::iterator it = topMemRefs->begin();
         it != topMemRefs->end(); ++it)
      {
	SgNode *memRefNode = *it;
	ROSE_ASSERT(memRefNode != NULL);

	// get all the sub memory references including top memory reference
	list<OA::MemRefHandle>* subMemRefList
	  = findAllMemRefsAndMemRefExprs(memRefNode);

	// put those memory references in the static mapping of statements to
	// the set of all memory references
	list<OA::MemRefHandle>::iterator mrIter;
	for (mrIter=subMemRefList->begin(); mrIter!=subMemRefList->end();
	     mrIter++)
	  {
	    SageIRInterface::sStmt2allMemRefsMap[stmt].insert(*mrIter);
	    SageIRInterface::sMemRef2StmtMap[*mrIter] = stmt;
	  }
      }
  }

  // loop through MemRefHandle's for this statement and for now put them
  // into our own list
  std::set<OA::MemRefHandle>::iterator setIter;
  for (setIter=SageIRInterface::sStmt2allMemRefsMap[stmt].begin();
       setIter!=SageIRInterface::sStmt2allMemRefsMap[stmt].end(); setIter++)
    {
      mMemRefList.push_back(*setIter);
    }

}

// isMemRefNode returns true if there should be a
// MemRefHandle associated with astNode.  
bool isMemRefNode(SgNode *astNode)
{
  
  ROSE_ASSERT(astNode != NULL);

  bool retVal = false;

  switch(astNode->variantT()) {

  case V_SgArrowExp:
  case V_SgDotExp:
    {
      // We represent the field access (i.e., rhs) at the
      // SgArrowExp or SgDotExp node.  Therefore, it is not
      // true, for example, that all SgVarRefExps have
      // a MemRefHandle.  In particular, the rhs SgVarRefExp
      // of a->b will not have a MemRefHandle/MemRefExpr
      // as this field access to b will be associated with
      // the arrow/dot node.
      retVal = true;
      break;
    }
  case V_SgPntrArrRefExp:
    {
      // We represent an array access at a SgPntrArrRefExp.
      // Therefore, as above, it is not true, for example, 
      // that all SgVarRefExps have a MemRefHandle.  
      // In particular, the sta SgVarRefExp
      // of sta[3][4] will not have a MemRefHandle/MemRefExpr
      // as this field access will be associated with
      // the SgPntrArrRefExp node.
      retVal = true;
      break;
    }
  case V_SgAndAssignOp:
  case V_SgAssignOp:
  case V_SgDivAssignOp:
  case V_SgIorAssignOp:
  case V_SgMinusAssignOp:
  case V_SgModAssignOp:
  case V_SgMultAssignOp:
  case V_SgPlusAssignOp:
  case V_SgXorAssignOp:
    {
      retVal = true;
      break;
    }
  case V_SgMinusMinusOp:
  case V_SgPlusPlusOp:
    {
      retVal = true;
      break;
    }
  case V_SgAddressOfOp:
    {
      retVal = true;
      break;
    }
  case V_SgVarRefExp:
  case V_SgNewExp:
  case V_SgThisExp:
  case V_SgPointerDerefExp:
  case V_SgMemberFunctionRefExp:  // i.e., the rhs of a->method();
    {
      retVal = true;
      break;
    }
  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(astNode);
      ROSE_ASSERT(initName != NULL);

      // If the initialized name has an initializer, it is
      // a definition (i.e., a DEF) and not simply a declration.
      SgInitializer *initializer = initName->get_initializer();
      if ( initializer != NULL ) 
	retVal = true;

      break;
    }
  case V_SgFunctionRefExp:
    {
      // A FunctionRefExp can appear on the right-hand side of
      // an assignment to a function pointer.  We could check
      // that it is the right-hand side of an assignment.
      // For now, assume that any time its parent is not a
      // SgFunctionCallExp that it is a USE.
      SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(astNode);
      ROSE_ASSERT(functionRefExp != NULL);

      if ( !isSgFunctionCallExp(astNode->get_parent()) ) 
	retVal = true;
      
      break;
    }
  case V_SgFunctionCallExp:
    {
      // A SgFunctionCallExp should be represented by a MemRefHandle
      // if it is a call to malloc or if it has a pointer/reference
      // return type that is used in an expression.
      // Note that a method invocation: a->method() is handled
      // above by the SgMemberFunctionRefExp case.  
      // The use of a function pointer on a rhs is handled 
      // above by the SgFunctionRefExp case.

      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(astNode);
      ROSE_ASSERT(functionCallExp != NULL);

      if ( isMalloc(functionCallExp) ) {

	// Represent malloc'ed memory by an unnamed memory reference.
	retVal = true;

      } else {

	// Create an unknown reference for this function if it
	// has a pointer or reference return type and that
	// return value is used in an expression.

	SgType *type = functionCallExp->get_type();
	ROSE_ASSERT(type != NULL);

	if ( ( isSgReferenceType(type) || isSgPointerType(type) ) && 
	     isSgExpression(functionCallExp->get_parent()) ) {

	  retVal = true;

	} 

      }

      break;
    }

  case V_SgConditionalExp:
    {
      SgConditionalExp *conditionalExp = isSgConditionalExp(astNode);
      ROSE_ASSERT(conditionalExp != NULL);

      // If either of the true or the false expressions of a 
      // conditional expression have memory reference handles
      // associated with them, then these will be propagated
      // up to the conditional expression.  i.e., it is an
      // expression whose value is either the true or false expression.

      SgExpression *trueExpression = conditionalExp->get_true_exp();
      ROSE_ASSERT(trueExpression != NULL);

      SgExpression *falseExpression = conditionalExp->get_false_exp();
      ROSE_ASSERT(falseExpression != NULL);

      if ( isMemRefNode(trueExpression) || isMemRefNode(falseExpression) ) {
	retVal = true;
      }

      break;
    }
  case V_SgAddOp:
  case V_SgSubtractOp:
    {
      // If at least one of the operands is a pointer type,
      // consider this pointer arithmetic, which will
      // generate a MemRefHandle.

      SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
      ROSE_ASSERT(binaryOp != NULL);

      SgExpression *lhs = binaryOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      SgType *lhsType = lhs->get_type();
      ROSE_ASSERT(lhsType != NULL);

      if ( isSgPointerType(lhsType) ) {
	retVal = true;
      }

      SgExpression *rhs = binaryOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);
      
      SgType *rhsType = rhs->get_type();
      ROSE_ASSERT(rhsType != NULL);
      
      if ( isSgPointerType(rhsType) ) {
	retVal = true;
	
      }

      // If this is the addition of something to an 
      // array base, treat it like a SgPntrArrRefExp.
      SgAddOp *addOp = isSgAddOp(astNode);
      if ( addOp != NULL ) {

	// If the lhs is a SgPntrArrRefExp, then consider
	// this an address of operation.  e.g., I have 
	// seen &(sta[3][4]) translated to (SgPntrArrRefExp + 4).
	
	SgExpression *lhs = addOp->get_lhs_operand();
	ROSE_ASSERT(lhs != NULL);
	
	retVal = ( isSgPntrArrRefExp(lhs) );

      }
	
      break;

    }
  default:
    {
      retVal = false;
      break;
    }

  }

  return retVal;

}

// getChildrenWithMemRefs returns in children all AST successors
// of astNode that could potentially have MemRefHandles/MemRefExprs.
// It returns in independentChildren the subset placed in children
// that are independent of astNode.  Independent means that any
// MemRefExpr attributes (e.g., numDerefs, addressOf, DEF/USE, etc.)
// of a node are independent of astNode's attribute values.
// e.g., a SgFunctionCallExp's actual arguments are independent
// of the SgFunctionCallExp, but *p is not independent of **p.
void
SageIRMemRefIterator::getChildrenWithMemRefs(SgNode *astNode,
					     std::vector<SgNode *>& independentChildren,
					     std::vector<SgNode *>& children)
{
  ROSE_ASSERT(astNode != NULL);

  // We are not expecting a scope statement here.
  ROSE_ASSERT(!isSgScopeStatement(astNode));

  // Determine which children to visit based on the type of astNode.
  switch(astNode->variantT()) {
  case V_SgArrowExp:
  case V_SgDotExp:
    {
      SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
      ROSE_ASSERT(binaryOp != NULL);

      // We represent the field access at the SgArrowExp
      // or SgDotExp itself.  Therefore, the corresponding
      // MemRefHandle/MemRefExpr is associated with this
      // node rather than the rhs.  Therefore, only return 
      // MemRefHandles from the lhs.

      SgExpression *lhs = binaryOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      children.push_back(lhs);

      break;
    }
  case V_SgPntrArrRefExp:
    {
      SgPntrArrRefExp *arrRefExp = isSgPntrArrRefExp(astNode);
      ROSE_ASSERT(arrRefExp != NULL);

      // We represent the array access at the SgPntrArrRefExp.
      // Therefore, the corresponding MemRefHandle/MemRefExpr is 
      // associated with this node rather than the array base.  
      // Therefore, do not return the lhs (i.e., array base)
      // as a potential source of MemRefHandles.

      // However, we should treat the array index expressions
      // as independent children, in the same way that we
      // handle actual arguments to a function.
      SgExpression *indexExpression = arrRefExp->get_rhs_operand();
      ROSE_ASSERT(indexExpression != NULL);

      children.push_back(indexExpression);
      independentChildren.push_back(indexExpression);

      break;
    }
  case V_SgFunctionCallExp:
    {
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(astNode);
      ROSE_ASSERT(functionCallExp != NULL);

      // Add the actual arguments and the function expression for
      // method invocations.

      // Notice we are not adding all successors of SgFunctionCallExp,
      // in particular we are not adding the function expression 
      // (SgFunctionRefExp) if this is a function call (as opposed
      // to a method call).  Also, we look through the SgExprListExp
      // that represents the args and return the args directly.

      SgExpression *expression = functionCallExp->get_function();
      ROSE_ASSERT(expression != NULL);
      
      if ( !isSgFunctionRefExp(expression) ) {
	
	// This looks like a method invocation (i.e., either
	// a.method() or a->method().  Return the dot/arrow expression.
	children.push_back(expression);
	
      }

      // Add the arguments as well.  Notice that the arguments are
      // independent.
      
      // Get the list of actual arguments from the function call.
      SgExprListExp* exprListExp = functionCallExp->get_args();  
      ROSE_ASSERT (exprListExp != NULL);  
      
      SgExpressionPtrList & actualArgs =  
	exprListExp->get_expressions();  
      
      // Iterate over the actual arguments as represented by
      // SgExpressions and set the ignore flag on each.
      for(SgExpressionPtrList::iterator actualIt = actualArgs.begin(); 
	  actualIt != actualArgs.end(); ++actualIt) { 
	
	SgExpression *actualArg = *actualIt;
	ROSE_ASSERT(actualArg != NULL);
	
	children.push_back(actualArg);
	independentChildren.push_back(actualArg);
	
      }
      
      break;
    }
  default:
    {
      // The default case is to visit all children.
      vector<SgNode *> containerList = 
	astNode->get_traversalSuccessorContainer();
      
      // Iterate over all children.
      for(vector<SgNode *>::iterator it = containerList.begin();
	  it != containerList.end(); ++it) {

	SgNode *node = *it;
	if ( node != NULL ) {

	  children.push_back(node);

	}

      }

      break;
    }
  }
}

// findTopMemRefs: Given a SgNode representing a statement, 
// recursively find the top memory references in the 
// statement/expression.  A top memory reference is in the 
// same basic block as astNode and has no memory references
// proceeding it in the basic block.  
// Function/method calls get slightly different treatement
// since allow each of their actual arguments to be top
// memory references as well.
// For example, if ( a < b ) { c = d; } has two top memory
// references:  a and b.  
// Inspired by Open64IRMemRefIterator::findTopMemRefs.
std::list<SgNode *>* 
SageIRMemRefIterator::findTopMemRefs(SgNode *astNode)
{
  std::list<SgNode *> *topMemRefs = new std::list<SgNode *>;
  findTopMemRefs(astNode, *topMemRefs);
  return topMemRefs;
}


void 
SageIRMemRefIterator::findTopMemRefs(SgNode *astNode, 
				     std::list<SgNode *>& topMemRefs)
{

  ROSE_ASSERT(astNode != NULL);

  // Collect a list of nodes to recurse on.
  vector<SgNode *> independentChildren;
  vector<SgNode *> children;

  // Base case.
  // If this is node corresponds to a MemRefHandle, append
  // it to the list of MemRefHandles.  
  if ( isMemRefNode(astNode) ) {

    topMemRefs.push_back(astNode);

    // Recurse only on independent children.  Currently,
    // the only independent children are the actual arguments
    // of a SgFunctionCallExp and the index expressions of
    // a SgPntrArrRefExp.
    getChildrenWithMemRefs(astNode, independentChildren, children);

    for(vector<SgNode *>::iterator it = independentChildren.begin();
	it != independentChildren.end(); ++it) {

      SgNode *node = *it;
      ROSE_ASSERT(node != NULL);
      
      findTopMemRefs(node, topMemRefs);
      
    }

    return;
  }

  // Build up recursive case.

  // If we have not found any memory references yet, we need
  // to recurse.  But do not recurse into compound statements
  // (e.g., do not visit the true/false bodies of an if statement).
  // We only want to return SgNodes that are in the same basic
  // block as astNode.  (Or would be, if ROSE had such a concept.)

  // Determine which children to visit based on the type of astNode.
  if ( !isSgScopeStatement(astNode)) {

    getChildrenWithMemRefs(astNode, independentChildren, children);

  } else {

    // Examine only conditionals of control flow statements.
    // An exception is SgConditionalExp (which is technically 
    // an expression and not a statement). Thus it is handled
    // above by getChildrenWithMemRefs where we visit all of
    // its children.  
    switch(astNode->variantT()) {

    case V_SgCatchOptionStmt:
      {
	SgCatchOptionStmt *catchOptionStmt = 
	  isSgCatchOptionStmt(astNode);
	ROSE_ASSERT(catchOptionStmt != NULL);

	SgVariableDeclaration *condition =
	  catchOptionStmt->get_condition();

	if ( condition != NULL )
	  children.push_back(condition);

	break;
      }
    case V_SgDoWhileStmt:
      {
	SgDoWhileStmt *doWhileStmt =
	  isSgDoWhileStmt(astNode);
	ROSE_ASSERT(doWhileStmt != NULL);

	SgStatement *condition = 
	  doWhileStmt->get_condition();
	if ( condition != NULL )
	  children.push_back(condition);

	break;
      }
    case V_SgForStatement:
      {
	SgForStatement *forStatement =
	  isSgForStatement(astNode);
	ROSE_ASSERT(forStatement != NULL);

	SgForInitStatement *forInitStmt =
	  forStatement->get_for_init_stmt();
	if (forInitStmt != NULL)
	  children.push_back(forInitStmt);

	SgExpression *testExpr =
	  forStatement->get_test_expr();
	if (testExpr != NULL)
	  children.push_back(testExpr);

	// I'm pretty suspicious of putting this here.  bwhite.
	SgExpression *incrExpr =
	  forStatement->get_increment_expr();
	if (incrExpr != NULL)
	  children.push_back(incrExpr);

	break;
      }
    case V_SgIfStmt:
      {
	SgIfStmt *ifStmt = isSgIfStmt(astNode);
	ROSE_ASSERT(ifStmt != NULL);

	SgStatement *condition = 
	  ifStmt->get_conditional();
	if ( condition != NULL )
	  children.push_back(condition);

	break;
      }
    case V_SgWhileStmt:
      {
	SgWhileStmt *whileStmt =
	  isSgWhileStmt(astNode);
	ROSE_ASSERT(whileStmt != NULL);

	SgStatement *condition = 
	  whileStmt->get_condition();
	if ( condition != NULL )
	  children.push_back(condition);

	break;
      }
    default:
      {
	break;
      }
    }
  }

  // Finally, recurse on the children.
  for(vector<SgNode *>::iterator it = children.begin();
      it != children.end(); ++it) {

    SgNode *node = *it;
    ROSE_ASSERT(node != NULL);
    
    findTopMemRefs(node, topMemRefs);

  }
}

/*! Given a mem-ref, find all mem-ref-exprs and sub mem-refs
    and return a list of them, also maps MemRefHandle's to a set of MemRefExpr's
    describing them for OA.
    Based on Open64IRMemRefIterator::findAllMemRefsAndMapToMemRefExprs.
*/
list<OA::MemRefHandle>*
SageIRMemRefIterator::findAllMemRefsAndMemRefExprs(SgNode *astNode)
{
  list<OA::MemRefHandle>* memRefs;
  memRefs = new list<OA::MemRefHandle>;
  findAllMemRefsAndMemRefExprs(astNode, *memRefs, 0);
  return memRefs;
}

enum SageRefExpFlagType { 
  expectUse         = 1,
  expectDef         = 2,
  expectUseDef      = 4,  // use and then define
  expectDefUse      = 8,  // define and then use
  expectAddrOf      = 16,
  ignore            = 32, // do not treat node as def/use
  partial           = 64,
  expectArrayBase   = 128,
  expectArrowDotRHS = 256,
};

static void addFlag(unsigned &flags, const SageRefExpFlagType &flag) {
  
  switch(flag) {
    
  case expectUse:
    {
      
      if ( ( flags & expectDef ) != 0 ) {
	flags &= ~expectDef;
	flags |= expectDefUse;
      } else if ( ( ( flags & expectDefUse ) != 0 ) ||
		  ( ( flags & expectUseDef ) != 0 ) ) {
	;
      } else {
	flags |= expectUse;
      }
      
      break;
    }
    
  case expectDef:
    {
      if ( ( flags & expectUse ) != 0 ) {
	flags &= ~expectUse;
	flags |= expectUseDef;
      } else if ( ( ( flags & expectDefUse ) != 0 ) ||
		  ( ( flags & expectUseDef ) != 0 ) ) {
	;
      } else {
	flags |= expectDef;
      }
      
      break;
    }
    
  case expectUseDef:
    {
      flags &= ~expectUse;
      flags &= ~expectDef;
      
      if ( ( flags & expectDefUse ) == 0 ) 
	flags |= expectUseDef;
      
      break;
    }
    
  case expectDefUse:
    {
      flags &= ~expectUse;
      flags &= ~expectDef;
      
      if ( ( flags & expectUseDef ) == 0 ) 
	flags |= expectDefUse;
      
      break;
    }
    
  case expectAddrOf:
    {
      flags |= expectAddrOf;
      
      break;
    }
    
  case ignore:
    {
      flags |= ignore;
      
      break;
    }

  case partial:
    {
      flags |= partial;
      
      break;
    }

  case expectArrayBase:
    {
      flags |= expectArrayBase;

      break;
    }

  case expectArrowDotRHS:
    {
      flags |= expectArrowDotRHS;

      break;
    }

  default:
    {
      break;
    }
    
  }
  
}

static void removeFlag(unsigned &flags, const SageRefExpFlagType &flag) {

  switch(flag) {
    
  case expectUse:
    {
      flags &= ~expectUse;
      
      if ( ( ( flags & expectDefUse ) != 0 ) ||
	   ( ( flags & expectUseDef ) != 0 ) ) {
	flags |= expectDef;
      }
      
      break;
    }
    
  case expectDef:
    {
      flags &= ~expectDef;
      
      if ( ( ( flags & expectDefUse ) != 0 ) ||
	   ( ( flags & expectUseDef ) != 0 ) ) {
	flags |= expectDef;
      }
      
      break;
    }
    
  case expectUseDef:
  case expectDefUse:
    {
      flags &= ~expectUse;
      flags &= ~expectDef;
      flags &= ~expectUseDef;
      flags &= ~expectDefUse;
      
      break;
    }
    
  case expectAddrOf:
    {
      flags &= ~expectAddrOf;
      
      break;
    }
    
  case ignore:
    {
      flags &= ~ignore;
      
      break;
    }

  case partial:
    {
      flags &= ~partial;
      
      break;
    }
    
  case expectArrayBase:
    {
      flags &= ~expectArrayBase;

      break;
    }

  case expectArrowDotRHS:
    {
      flags &= ~expectArrowDotRHS;

      break;
    }

  default:
    {
      break;
    }
    
  }
  
}

static OA::MemRefExpr::MemRefType flagsToMemRefType(unsigned flags)
{
  // By default, we assume that any node passed to this function
  // represents a USE, unless it has flags that indicate otherwise.
  // Therefore, we only invoke this function when we expect a
  // DEF and/or USE.
  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
  
  if ( ( flags & expectDef ) != 0 )
    memRefType = OA::MemRefExpr::DEF;
  else if ( ( flags & expectDefUse ) != 0 )
    memRefType = OA::MemRefExpr::DEFUSE;
  else if ( ( flags & expectUseDef ) != 0 )
    memRefType = OA::MemRefExpr::USEDEF;
 
  return memRefType;
}

/*! Given a memory-reference SgNode, find all MemRefHandles
    contained within and place them in 'memRefs'.
    Also map each MemRefHandle to a set of MemRefExprs in sMemRef2mreSetMap.
    Returns the number of dereferences within the mem-ref and the location block
    used within the top-most mem-ref-expr of the mem-ref.  Outside
    callers of this routine should pass a recursion level 'lvl' of 0
    and flags 'flags' of 0.

    Note: The returned information is consumed by recursive calls and
    will not be of interest to others.  The returned information
    should be *copied* if it is to be used.)
    Based on Open64IRMemRefIterator::findAllMemRefsAndMapToMemRefExprs.
*/

void
SageIRMemRefIterator::handleDefaultCase(SgNode *astNode,
					list<OA::MemRefHandle>& memRefs,
					unsigned flags)
{
  // Pass flags down to children.
  
  vector<SgNode *> containerList = 
    astNode->get_traversalSuccessorContainer();
  
  // Iterate over all children and recurse.
  for(vector<SgNode *>::iterator it = containerList.begin();
      it != containerList.end(); ++it) {

    SgNode *node = *it;
    if ( node != NULL ) {

      findAllMemRefsAndMemRefExprs(node, memRefs, flags);
      
    }
 
  }
  
}

list<OA::OA_ptr<OA::MemRefExpr> >
SageIRMemRefIterator::findAllMemRefsAndMemRefExprs(SgNode *astNode,
						   list<OA::MemRefHandle>& memRefs,
						   unsigned flags)
{
  bool isMemRefExpr                     = false;
  OA::SymHandle symHandle               = 0;
  OA::StmtHandle stmtHandle             = 0;
  bool addressTaken                     = true;
  bool fullAccuracy                     = false;
  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
  bool isNamed                          = false;
  bool isUnnamed                        = false;
  bool isUnknown                        = false;
  bool isDeref                          = false;

  // A list of mem-ref infos, *some* of which correspond to
  // mem-ref-handles added to 'memRefs' for this SgNode.
  list<OA::OA_ptr<OA::MemRefExpr> > curMemRefExprs;
  
  // -------------------------------------------------------
  // 0. Special base cases
  // -------------------------------------------------------
  if ( astNode == NULL ) { return curMemRefExprs; }

  // We do not expect to see a scope statement here.
  ROSE_ASSERT(!isSgScopeStatement(astNode));

  //  cout << "Type: " << astNode->sage_class_name() << endl;

  switch(astNode->variantT()) {

  case V_SgAndAssignOp:
  case V_SgAssignOp:
  case V_SgDivAssignOp:
  case V_SgIorAssignOp:
  case V_SgMinusAssignOp:
  case V_SgModAssignOp:
  case V_SgMultAssignOp:
  case V_SgPlusAssignOp:
  case V_SgXorAssignOp:
    {
      // Assignment operators.  
      SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
      ROSE_ASSERT(binaryOp != NULL);

      // Add DEF to the lhs operand's flags.
      SgExpression *lhs = binaryOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      unsigned lhsFlags = flags;

      // Consume expectArrayBase and expectArrowDotRHS.  Memory
      // reference expressions within an assignment need not
      // account for them.
      removeFlag(lhsFlags, expectArrayBase);
      removeFlag(lhsFlags, expectArrowDotRHS);
      
      // Add DEF to the lhs child's flags.
      if ( lhsFlags & expectUse ) {

	// This assignment is already a USE.  e.g., a = b = c.
	// This node is about to become a USEDEF/DEFUSE.
	// Which should it be?  We are encountering
	// DEFs and USEs top-down in this traversal.
	// However, assignments (think a = b = c)
	// will be evaluated right-to-left, that is,
	// bottom-up.  Therefore, if we encounter a
	// USE and then a DEF on our top-down traversal,
	// this is (counter-intuitivitely) a DEFUSE
	// (DEF first, then USE) when evaluated.
	addFlag(lhsFlags, expectDefUse);

      } else {

	addFlag(lhsFlags, expectDef);

      }

      // Recurse on lhs.
      list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;
      lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags);
#if 0
      // Iterate over the MemRefExprs on the lhs and turn each
      // into a DEF (as it is on the lhs of an assign).  Notice
      // that we do not store the MemRefExpr/MemRefHandle on the lhs,
      // but we do here.
      bool hasOneOrMoreChildMemRefExps = false;

      memRefType = flagsToMemRefType(lhsFlags);

      for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = lhsMemRefExprs.begin();
	  it != lhsMemRefExprs.end(); ++it) {
	
	OA::OA_ptr<OA::MemRefExpr> lhsMemRefExp = *it;
	ROSE_ASSERT(!lhsMemRefExp.ptrEqual(0));
	
	// Given the memory reference expression lhsMemRefExp (e.g., 
	// representing mem ref expr a), we need
	// to create a new memory reference expression
	// (e.g., representing mem ref expr a = ...).  This
	// new mem ref expression is simply a copy of the
	// lhs mem ref expression with the DEF flag set.
	OA::OA_ptr<OA::MemRefExpr> memRefExpr = lhsMemRefExp;
	
#ifdef BWHITE_VERSION
	memRefExpr->setMemRefType(memRefType);
#endif	
	curMemRefExprs.push_back(memRefExpr);
	
	isMemRefExpr = true;

	hasOneOrMoreChildMemRefExps = true;
	
      } // end iteration over lhs' memory references
      
      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);
#endif
      // Add USE to the rhs child's flags.
      unsigned rhsFlags = flags;

      // Consume expectArrayBase and 
      // expectArrowDotRHS.  Memory
      // reference expressions within an assignment need not
      // account for them.
      removeFlag(rhsFlags, expectArrayBase);
      removeFlag(rhsFlags, expectArrowDotRHS);

      SgExpression *rhs = binaryOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      if ( rhsFlags & expectDef ) {

	// See note above.
	addFlag(rhsFlags, expectUseDef);

      } else {

	addFlag(rhsFlags, expectUse);

      }

      // Recurse on rhs.
      findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags);

      // The result of evaluating an assignment operator is its
      // left-hand side.  Therefore, return any memory reference
      // exressions.  These will needed, e.g., if we dereference 
      // the result of an assignment:
      // int *a; int *b;  *(a = b) = 5;
      // We do not create any new memory reference expressions
      // at this node.  Actually, not true, we store the lhs
      // MemRefExprs here.
      curMemRefExprs.splice(curMemRefExprs.end(), lhsMemRefExprs);

      //      isMemRefExpr = true;

      break;
    }

  case V_SgMinusMinusOp:
  case V_SgPlusPlusOp:
    {
      // Unary assignment operators.
      SgUnaryOp *unaryOp = isSgUnaryOp(astNode);
      ROSE_ASSERT(unaryOp != NULL);

      // Add DEFUSE or USEDEF to the operand's flags.
      SgExpression *operand = unaryOp->get_operand_i();
      ROSE_ASSERT(operand != NULL);

      unsigned childFlags = flags;

      SgUnaryOp::Sgop_mode mode = unaryOp->get_mode();

      // Consume expectArrayBase and expectArrowDotRHS.  
      removeFlag(childFlags, expectArrayBase);
      removeFlag(childFlags, expectArrowDotRHS);

      if ( mode == SgUnaryOp::prefix ) {
	// Memory reference is defined first and then used.
	addFlag(childFlags, expectDefUse);
      } else {
	// Memory reference is used first and then defined.
	addFlag(childFlags, expectUseDef);
      }

      // Recurse on operand.
      list<OA::OA_ptr<OA::MemRefExpr> > operandMemRefExprs;
      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(operand, memRefs, childFlags);
#if 0
      // Iterate over the operand's MemRefExprs and turn each
      // into a DEF (as this is an assignment).  Notice
      // that we do not store the MemRefExpr/MemRefHandle at the operand,
      // but we do here.
      bool hasOneOrMoreChildMemRefExps = false;

      memRefType = flagsToMemRefType(childFlags);

      for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = operandMemRefExprs.begin();
	  it != operandMemRefExprs.end(); ++it) {
	
	OA::OA_ptr<OA::MemRefExpr> operandMemRefExp = *it;
	ROSE_ASSERT(!operandMemRefExp.ptrEqual(0));
	
	// Given the memory reference expression lhsMemRefExp (e.g., 
	// representing mem ref expr a), we need
	// to create a new memory reference expression
	// (e.g., representing mem ref expr a++/a--/++a/--a).  This
	// new mem ref expression is simply a copy of the
	// lhs mem ref expression with the DEF flag set.
	OA::OA_ptr<OA::MemRefExpr> memRefExpr = operandMemRefExp;
	
#ifdef BWHITE_VERSION
	memRefExpr->setMemRefType(memRefType);
#endif	
	curMemRefExprs.push_back(memRefExpr);
	
	isMemRefExpr = true;

	hasOneOrMoreChildMemRefExps = true;
	
      } // end iteration over operand's memory references
      
      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);
#endif

      // The result of evaluating an assignment operator is its
      // left-hand side.  Therefore, return any memory reference
      // exressions.  These will needed, e.g., if we dereference 
      // the result of an assignment:
      // int *a; int *b;  *(a = b) = 5;
      // We do not create any new memory reference expressions
      // at this node.  Actually, not true, we store the lhs
      // MemRefExprs here.
      curMemRefExprs.splice(curMemRefExprs.end(), operandMemRefExprs);

      //      isMemRefExpr = true;

      break;
    }

  case V_SgAddressOfOp:
    {
      SgAddressOfOp *addressOfOp = isSgAddressOfOp(astNode);
      ROSE_ASSERT(addressOfOp != NULL);

      // Add addressOf to the operand's flags.
      SgExpression *operand = addressOfOp->get_operand_i();
      ROSE_ASSERT(operand != NULL);

      unsigned childFlags = flags;

      // Consume expectArrayBase and expectArrowDotRHS.  
      removeFlag(childFlags, expectArrayBase);
      removeFlag(childFlags, expectArrowDotRHS);

      addFlag(childFlags, expectAddrOf);

      // Recurse on operand.
      list<OA::OA_ptr<OA::MemRefExpr> > operandMemRefExprs;
      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(operand, memRefs, childFlags);

      // Store here any memory reference expressions created at our
      // children.
      curMemRefExprs.splice(curMemRefExprs.end(), operandMemRefExprs);

      isMemRefExpr = true;

      break;

    }

  case V_SgArrowExp:
  case V_SgDotExp:
    {
      // Because we do not account for field names in memory
      // reference expressions, field accesses (the right-hand
      // side of arrow and dot expressions) are represented
      // with partial (or incomplete) accuracy.
      SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
      ROSE_ASSERT(binaryOp != NULL);

      // Represent the field access at this node, rather than
      // at the rhs.  We create a MemRefExpr representing
      // that field access for each potential lhs MemRefExpr,
      // so first determine that set of MemRefExprs.
      SgExpression *lhs = binaryOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      unsigned lhsFlags = flags;

      // Consume expectArrayBase and expectArrowDotRHS.  
      removeFlag(lhsFlags, expectArrayBase);
      removeFlag(lhsFlags, expectArrowDotRHS);

      // The addressOf flag only applies to the top-most
      // memory reference.  Therefore, remove it from the
      // lhs memory reference expression attribute.
      // If the address of an expression a->b or a.b is taken,
      // it applies to the expression as a whole (which is
      // represented by the SgArrowExp/SgDotExp).  
      // Therefore, remove from the lhs.  
      removeFlag(lhsFlags, expectAddrOf);

      // Don't propagate DEF down to the lhs child
      // If this arrow/dot expression is used on the lhs,
      // it is the memRefExpr created at the SgArrowExp/SgDotExp
      // that will reflect this DEF.
      removeFlag(lhsFlags, expectDef);

      // Recurse on lhs.
      list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;
      lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags);

      // For each MemRefExpr on the lhs, create a new MemRefExpr
      // to represent the field access.

      bool isArrowExp = ( isSgArrowExp(binaryOp) != NULL );

      if ( !isArrowExp ) {
	
	SgDotExp *dotExp = isSgDotExp(binaryOp);
	ROSE_ASSERT(dotExp != NULL);

	if ( isSgPointerDerefExp(lhs) ) {

	  // Treat (*a).b as a->b.  Note this requires
	  // special treatement at a SgPointerDerefExp.
	  isArrowExp = true;

	}

      }

      // Is the address taken in this memory reference?
      addressTaken = ( flags & expectAddrOf );
      
      // This memory reference expression does _not_
      // accurately represent the memory expression since
      // we do not account for fields in the access.
      fullAccuracy = false;
	  
      // Get the memory reference type (DEF, USE, etc.) for 
      // this node.
      memRefType = flagsToMemRefType(flags);
      
      // The arrow expression is a dereference of the lhs
      // memory reference expression.  A dot expression is simply
      // a partial Named memory ref exprssion.

      bool hasOneOrMoreChildMemRefExps = false;


      for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = lhsMemRefExprs.begin();
	  it != lhsMemRefExprs.end(); ++it) {

	OA::OA_ptr<OA::MemRefExpr> lhsMemRefExp = *it;
	
	OA::OA_ptr<OA::MemRefExpr> arrowOrDotMemRefExpr;
	arrowOrDotMemRefExpr = NULL;
	
	if ( isArrowExp ) {
	  
	  // Given the memory reference expression lhsMemRefExp 
	  // (e.g., representing mem ref expr a),
	  // we need to create a new memory reference expression
	  // (e.g., representing mem ref expr a->b).  We don't
	  // even have to increment the number of derefs,
	  // we simply create a new Deref mem ref expression 
	  // whose base mem ref is the one at the lhs, with only
	  // one dereference.  A chain of derefs is handled
	  // by having the base mem ref by a Deref, _not_
	  // by having numDerefs > 1.
	  int numDerefs = 1;
	  
#ifndef BWHITE_VERSION
	  arrowOrDotMemRefExpr = new OA::Deref(lhsMemRefExp, numDerefs);
#else
	  arrowOrDotMemRefExpr = new OA::Deref(addressTaken,
					       fullAccuracy,
					       memRefType,
					       lhsMemRefExp,
					       numDerefs);
#endif	  
	} else {
	  
	  // This is a dot expression.  Given the memory
	  // reference expression lhsMemRefExp (e.g., 
	  // representing mem ref expr a), we need
	  // to create a new memory reference expression
	  // (e.g., representing mem ref expr a.b).  This
	  // new mem ref expression is simply a copy of the
	  // lhs mem ref expression with the flags set
	  // appropriately.  Unless it's a Deref; we don't
	  // want this mem ref to be a dereference (since
	  // it isn't), so instead copy it's (first non-deref)
	  // base mem ref.  This is really begging for FieldAccess
	  // mem ref type to be implemented.
	  
	  arrowOrDotMemRefExpr = 
	    copyBaseMemRefExpr(lhsMemRefExp);
	  
	}
	
	ROSE_ASSERT(!arrowOrDotMemRefExpr.ptrEqual(0));
#ifdef BWHITE_VERSION
	arrowOrDotMemRefExpr->setAccuracy(fullAccuracy);
	arrowOrDotMemRefExpr->setAddressTaken(addressTaken);
	arrowOrDotMemRefExpr->setMemRefType(memRefType);
#endif
	curMemRefExprs.push_back(arrowOrDotMemRefExpr);
	
	isMemRefExpr = true;

	hasOneOrMoreChildMemRefExps = true;


      } // end iteration over lhs MemRefExprs

      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);

      SgExpression *rhs = binaryOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      unsigned rhsFlags = flags;

      // Do _not_ consume expectArrayBase or expectArrowDotRHS
      // from the rhs of an arrow/dot expression since we
      // are going to ignore it (due to expectArrayBase).

      // Consume expectDef so that the children don't become DEFs.
      // We will create a mem ref expression at this node to
      // represent the DEF.
      removeFlag(rhsFlags, expectDef);

      addFlag(rhsFlags, partial);
      addFlag(rhsFlags, expectArrowDotRHS);

      // Recurse on rhs.
      list<OA::OA_ptr<OA::MemRefExpr> > rhsMemRefExprs;
      rhsMemRefExprs = findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags);

      // Notice that we
      // expect only one memory reference expression on the
      // rhs.  On the lhs, we allow expressions such
      // as ( a < b : c ? d ) which could yield multiple
      // mem ref expressions.  This does not make sense
      // on the right-hand side.
      ROSE_ASSERT(rhsMemRefExprs.size() == 1);

      break;
    }

  case V_SgAddOp:
  case V_SgSubtractOp:
    {
      bool isArrayOperation = false;

      SgAddOp *addOp = isSgAddOp(astNode);
      if ( addOp != NULL ) {

	// If the lhs is a SgPntrArrRefExp, then consider
	// this an address of operation.  e.g., I have 
	// seen &(sta[3][4]) translated to (SgPntrArrRefExp + 4).
	
	SgExpression *lhs = addOp->get_lhs_operand();
	ROSE_ASSERT(lhs != NULL);
	
	if ( isSgPntrArrRefExp(lhs) ) {
	  
	  isArrayOperation = true;
	  
	  // Treat this SgAddOp as we treat a SgPntrArrRefExp,
	  // though with the address taken flag set.  
	  // We usually create the MemRefExpr for an array access
	  // at the SgPntrArrRefExp, so create it here. 
	  
	  // Propagate this node's flags to its lhs child.
	  unsigned lhsFlags = flags;
	  
	  // Do not consume expectArrayBase and expectArrowDotRHS
	  // from the lhs since we are going to ignore it.
	  // Pass the expectArrayBase flag to the lhs to indicate
	  // that we should not create a MemRefHandle/MemRefExpr
	  // for the base of the array access.
	  addFlag(lhsFlags, expectArrayBase);
	  
	  // Consume expectDef so that the children don't become DEFs.
	  // We will create a mem ref expression at this node to
	  // represent the DEF.
	  removeFlag(lhsFlags, expectDef);

	  // Recurse on the lhs.
	  list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;
	  lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags);

	  // Also visit the array index expression.
	  // Notice that the array index expressions
	  // should be evaluated independently of the lhs--
	  // as are the actual arguments to a function call.
	  // i.e., we zero out the flags.
	  
	  // The addressOf flag only applies to the top-most
	  // memory reference.  Therefore, remove it from the
	  // array index expressions.
	  SgExpression *rhs = addOp->get_rhs_operand();
	  ROSE_ASSERT(rhs != NULL);
	  
	  // Notice there aren't any expectArrayBase, expectDef, or
	  // expectArrowDotRHS flags to consume since we've
	  // zeroed out the flags.

	  // Zero out the rhs flags.
	  unsigned rhsFlags = 0;

	  findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags);
	  
	  // Add the implicit addrOf operator to this node.
	  addressTaken = true;
	  
	  // Because we do not model the indices of the array
	  // dereference, this is not a fully accurate model
	  // of the access.
	  fullAccuracy = false;
	  
	  // Get the memory reference type (DEF, USE, etc.) for 
	  // this node.
	  memRefType = flagsToMemRefType(flags);
	  
	  // Create the memory reference expressions for the array
	  // access based on the mem ref type of the lhs.
	  SgExpression *arrayBase = lhs;
	  ROSE_ASSERT(arrayBase != NULL);
	  
	  // If the lhs of the SgPntrArrRefExp is itself a
	  // SgPntrArrRefExp, then this is a multidimensional
	  // array access.  We only create one MemRefHandle/MemRefExpr
	  // for a multidimensional array access and we do
	  // it at the last access (i.e., the first one
	  // encountered on a top down traversal).  Therefore
	  // pass the expectArrayBase flag to the lhs children
	  // when visiting them.
	  
	  bool hasOneOrMoreChildMemRefExps = false;
	  
	  for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = lhsMemRefExprs.begin();
	      it != lhsMemRefExprs.end(); ++it) {
	    
	    OA::OA_ptr<OA::MemRefExpr> lhsMemRefExp = *it;
	    ROSE_ASSERT(!lhsMemRefExp.ptrEqual(0));
	    
	    OA::OA_ptr<OA::MemRefExpr> arrMemRefExpr;
	    arrMemRefExpr = NULL;
	    
	    // Given the memory reference expression lhsMemRefExp (e.g., 
	    // representing mem ref expr a), we need
	    // to create a new memory reference expression
	    // (e.g., representing mem ref expr a[b]).  This
	    // new mem ref expression is simply a copy of the
	    // lhs mem ref expression with the flags set
	    // appropriately.  Unless it's a Deref; we don't
	    // want this mem ref to be a dereference (since
	    // it isn't), so instead copy it's (first non-deref)
	    // base mem ref.  
	    
	    arrMemRefExpr = 
	      copyBaseMemRefExpr(lhsMemRefExp);
	    
	    ROSE_ASSERT(!arrMemRefExpr.ptrEqual(0));
#ifdef BWHITE_VERSION
	    arrMemRefExpr->setAccuracy(fullAccuracy);
	    arrMemRefExpr->setAddressTaken(addressTaken);
	    arrMemRefExpr->setMemRefType(memRefType);
#endif	    
	    curMemRefExprs.push_back(arrMemRefExpr);
	    
	    isMemRefExpr = true;

	    hasOneOrMoreChildMemRefExps = true;
	    
	  } // end iteration over lhs' memory references
	  
	  ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);
	  
	}

      }

      if ( !isArrayOperation ) {

	// If this addition or subtraction involves a pointer
	// type, treat it as pointer arithmetic, which creates
	// an unknown memory reference.

	bool isPointerArithmetic = false;

	SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
	ROSE_ASSERT(binaryOp != NULL);
	
	SgExpression *lhs = binaryOp->get_lhs_operand();
	ROSE_ASSERT(lhs != NULL);

	SgType *lhsType = lhs->get_type();
	ROSE_ASSERT(lhsType != NULL);
	
	if ( isSgPointerType(lhsType) ) {
	  isPointerArithmetic = true;
	}

	if ( !isPointerArithmetic ) {

	  SgExpression *rhs = binaryOp->get_rhs_operand();
	  ROSE_ASSERT(rhs != NULL);
	  
	  SgType *rhsType = rhs->get_type();
	  ROSE_ASSERT(rhsType != NULL);
	  
	  if ( isSgPointerType(rhsType) ) {
	    isPointerArithmetic = true;
	  }

	}

	if ( isPointerArithmetic ) {

	  // Get the memory reference type (DEF, USE, etc.) for 
	  // this node.
	  memRefType = flagsToMemRefType(flags);
	  
	  OA::OA_ptr<OA::UnknownRef> unknown;
	  unknown = new OA::UnknownRef(memRefType);
	  
	  isMemRefExpr = true;

	  curMemRefExprs.push_back(unknown);

	}

	// Whether or not this is pointer arithmetic, we
	// need to visit the children.  Notice that we do 
	// not reach here when treating a SgAddOp as an
	// array access.  In that case, we explicitly visit the
	// children.
	handleDefaultCase(astNode, memRefs, flags);

      }

      break;
    }

  case V_SgPntrArrRefExp:
    {
      // We create the MemRefExpr for an array access
      // at the lhs.

      // Because we do not account for array indices in memory
      // reference expressions, array accesses (the left-hand
      // side of SgPntrArrRefExps) are represented
      // with partial (or incomplete) accuracy.
      SgPntrArrRefExp *arrRefExp = isSgPntrArrRefExp(astNode);
      ROSE_ASSERT(arrRefExp != NULL);

      SgExpression *lhs = arrRefExp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      // Propagate this node's flags to its lhs child.
      unsigned lhsFlags = flags;

      // Do not consume expectArrayBase or expectArrowDotRHS
      // from the lhs since we are going to ignore it.
      // Pass the expectArrayBase flag to the lhs to indicate
      // that we should not create a MemRefHandle/MemRefExpr
      // for the base of the array access.
      addFlag(lhsFlags, expectArrayBase);

      // Consume expectDef so that the children don't become DEFs.
      // We will create a mem ref expression at this node to
      // represent the DEF.
      removeFlag(lhsFlags, expectDef);

      // Recurse on the lhs.
      list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;
      lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags);

      // Also visit the array index expression.
      // Notice that the array index expressions
      // should be evaluated independently of the lhs--
      // as are the actual arguments to a function call.
      // i.e., we zero out the flags.

      // The addressOf flag only applies to the top-most
      // memory reference.  Therefore, remove it from the
      // array index expressions.
      SgExpression *rhs = arrRefExp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      // Notice there aren't any expectArrayBase, expectDef, or
      // expectArrowDotRHS flags to consume since we've
      // zeroed out the flags.

      // Zero out the rhs flags.
      unsigned rhsFlags = 0;

      findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags);

      // Is the address taken in this memory reference?
      addressTaken = ( flags & expectAddrOf );
      
      // Because we do not model the indices of the array
      // dereference, this is not a fully accurate model
      // of the access.
      fullAccuracy = false;
	  
      // Get the memory reference type (DEF, USE, etc.) for 
      // this node.
      memRefType = flagsToMemRefType(flags);
      
      // Create the memory reference expressions for the array
      // access based on the mem ref type of the lhs.
      SgExpression *arrayBase = arrRefExp->get_lhs_operand();
      ROSE_ASSERT(arrayBase != NULL);
      
      // If the lhs of the SgPntrArrRefExp is itself a
      // SgPntrArrRefExp, then this is a multidimensional
      // array access.  We only create one MemRefHandle/MemRefExpr
      // for a multidimensional array access and we do
      // it at the last access (i.e., the first one
      // encountered on a top down traversal).  Therefore
      // pass the expectArrayBase flag to the lhs children
      // when visiting them.

      bool hasOneOrMoreChildMemRefExps = false;

      for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = lhsMemRefExprs.begin();
	  it != lhsMemRefExprs.end(); ++it) {
	
	OA::OA_ptr<OA::MemRefExpr> lhsMemRefExp = *it;
	ROSE_ASSERT(!lhsMemRefExp.ptrEqual(0));
	
	OA::OA_ptr<OA::MemRefExpr> arrMemRefExpr;
	arrMemRefExpr = NULL;
	
	// Given the memory reference expression lhsMemRefExp (e.g., 
	// representing mem ref expr a), we need
	// to create a new memory reference expression
	// (e.g., representing mem ref expr a[b]).  This
	// new mem ref expression is simply a copy of the
	// lhs mem ref expression with the flags set
	// appropriately.  Unless it's a Deref; we don't
	// want this mem ref to be a dereference (since
	// it isn't), so instead copy it's (first non-deref)
	// base mem ref.  
	
	arrMemRefExpr = 
	  copyBaseMemRefExpr(lhsMemRefExp);
	
	ROSE_ASSERT(!arrMemRefExpr.ptrEqual(0));
#ifdef BWHITE_VERSION
	arrMemRefExpr->setAccuracy(fullAccuracy);
	arrMemRefExpr->setAddressTaken(addressTaken);
	arrMemRefExpr->setMemRefType(memRefType);
#endif	
	curMemRefExprs.push_back(arrMemRefExpr);
	
	isMemRefExpr = true;

	hasOneOrMoreChildMemRefExps = true;
	
      } // end iteration over lhs' memory references
      
      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);

      break;

    }

  case V_SgFunctionCallExp:
    {

      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(astNode);
      ROSE_ASSERT(functionCallExp != NULL);
      
      // Notice that we do not propagate a function call's
      // flags to its true children or its logical children (arguments)
      // in the AST since they should be treated independently
      // of any flags previously encountered.
      
      // Get the list of actual arguments from the function call.
      SgExprListExp* exprListExp = functionCallExp->get_args();  
      ROSE_ASSERT (exprListExp != NULL);  
      
      SgExpressionPtrList & actualArgs =  
	exprListExp->get_expressions();  

      // Iterate over the actual arguments as represented by
      // SgExpressions and recurse.
      for(SgExpressionPtrList::iterator actualIt = actualArgs.begin(); 
	  actualIt != actualArgs.end(); ++actualIt) { 
 
	SgExpression *actualArg = *actualIt;
	ROSE_ASSERT(actualArg != NULL);

	// The actual arguments are treated
	// independently of whatever precedes them in the
	// AST.  Therefore, do not propagate the parent
	// node's flags to them, but rather zero out their flags.

	// Notice that we don't need to consume expectArrayBase, expectDef,
	// and expectArrowDotRHS since we have zeroed out the flags.

	findAllMemRefsAndMemRefExprs(actualArg, memRefs, 0);
      }

      // Create a MemRefExpr for this function call if required.
      if ( isMalloc(functionCallExp) ) {

	// Represent malloc'ed memory by an unnamed memory reference.

	memRefType = OA::MemRefExpr::USE;

	// Create an OpenAnalysis handle for this node.
	stmtHandle = mIR->getNodeNumber(functionCallExp);

	// malloc computes the address of a memory location,
	// so consider it an addressOf operation.
	addressTaken = true;

	// The malloc call does _not_ accurately represent the memory 
	// expression, as this would require the precise calling context.
	fullAccuracy = false;

	// Create an unnamed memory reference expression.
	OA::OA_ptr<OA::MemRefExpr> memRefExp;
	memRefExp = new OA::UnnamedRef(addressTaken, fullAccuracy,
				       memRefType, stmtHandle);
	ROSE_ASSERT(!memRefExp.ptrEqual(0));

	isMemRefExpr = true;

	curMemRefExprs.push_back(memRefExp);

      } else {

	// Create an unknown reference for this function if it
	// has a pointer or reference return type.
	// Also, if it is a method call, we need to visit the
	// arrow or dot expression.  This will be held in the
	// function child.

	SgType *type = functionCallExp->get_type();
	ROSE_ASSERT(type != NULL);

	if ( isSgReferenceType(type) ) {

	  // This function has a reference return type.
	  // If the parent of this call site is an expression,
	  // (other than a parameter list)
	  // then we consider it a use or def (as specified
	  // by its flags).  If the parent is not an 
	  // expression then its return value is not 
	  // used and we needn't consider it a def/use.

	  SgNode *parent = functionCallExp->get_parent();
	  ROSE_ASSERT(parent != NULL);

	  if ( isSgExpression(parent) ) {

	    // Get the memory reference type (DEF, USE, etc.) for 
	    // this function call.
	    memRefType = flagsToMemRefType(flags);

	    // Create an OpenAnalysis handle for this node.
	    stmtHandle = mIR->getNodeNumber(functionCallExp);
	    
	    // The address of the memory location is not taken
	    // during a function call.
	    addressTaken = false;
	    
	    // The function call does _not_ accurately represent the memory 
	    // expression, as this would require the precise calling context.
	    fullAccuracy = false;
	    
	    // Create an unknown memory reference expression.
	    OA::OA_ptr<OA::MemRefExpr> memRefExp;
#if 0
	    memRefExp = new OA::UnnamedRef(addressTaken, fullAccuracy,
					   memRefType, stmtHandle);
#else
	    memRefExp = new OA::UnknownRef(memRefType);
#endif
	    ROSE_ASSERT(!memRefExp.ptrEqual(0));
	    
	    isMemRefExpr = true;

	    curMemRefExprs.push_back(memRefExp);

	  }  // usedInExpression

	} else if ( isSgPointerType(type) ) {

	  // This function has a pointer return type.

	  // Get the memory reference type (DEF, USE, etc.) for 
	  // this function call.
	  memRefType = flagsToMemRefType(flags);
	  
	  // Create an OpenAnalysis handle for this node.
	  stmtHandle = mIR->getNodeNumber(functionCallExp);
	  
	  // This function computes the address of a memory
	  // location, consider it an addressOf operation.
	  addressTaken = true;
	  
	  // The function call does _not_ accurately represent the memory 
	  // expression, as this would require the precise calling context.
	  fullAccuracy = false;
	  
	  // Create an unknown memory reference expression.
	  OA::OA_ptr<OA::MemRefExpr> memRefExp;
#if 0
	  memRefExp = new OA::UnnamedRef(addressTaken, fullAccuracy,
					 memRefType, stmtHandle);
#else
	  memRefExp = new OA::UnknownRef(memRefType);
#endif
	  ROSE_ASSERT(!memRefExp.ptrEqual(0));
	  
	  isMemRefExpr = true;

	  curMemRefExprs.push_back(memRefExp);

	}

	// Now visit the arrow or dot expression if this is 
	// a method call.
	SgExpression *expression = functionCallExp->get_function();
	ROSE_ASSERT(expression != NULL);

	if ( !isSgFunctionRefExp(expression) ) {
	  // Zero out the rhs flags.
	  unsigned childFlags = 0;
	  
	  findAllMemRefsAndMemRefExprs(expression, memRefs, childFlags);
	}
      }

      break;
    }

  case V_SgConditionalExp:
    {
      // i.e., ( a cond b ? c : d )
      SgConditionalExp *conditionalExp = isSgConditionalExp(astNode);
      ROSE_ASSERT(conditionalExp != NULL);

      // Do not propagate a SgConditionalExp's flags to its
      // conditional child.  This child should be treated
      // independently, so simply zero out its flags.
      // Notice that we don't need to consume expectArrayBase, expectDef,
      // and expectArrowDotRHS since we have zeroed out the flags.
      SgExpression *conditionalChild = conditionalExp->get_conditional_exp();
      ROSE_ASSERT(conditionalChild != NULL);

      // Recurse on the conditional.
      findAllMemRefsAndMemRefExprs(conditionalChild, memRefs, 0);

      SgExpression *trueExpression = conditionalExp->get_true_exp();
      ROSE_ASSERT(trueExpression != NULL);
      
      // Propagate this node's flags to its child.
      unsigned trueFlags = flags;
      
      // Don't propagate DEF down to the child.  If this
      // SgConditionalExp is used on the lhs, we will represent
      // the DEF access by the MemRefExpr we create for this
      // node, not by that of it's children.
      removeFlag(trueFlags, expectDef);

      // Do not consume expectArrayBase or expectArrowDotRHS
      // from either of the true or false expressions.
      // i.e., we might not store MemRefHandles/MemRefExprs
      // at their respective AST nodes, though we will pass
      // them back to the caller via findAllMemRefsAndMemRefExprs
      // return value.

      // Recurse on true expression.
      list<OA::OA_ptr<OA::MemRefExpr> > trueMemRefExprs;
      trueMemRefExprs = 
	findAllMemRefsAndMemRefExprs(trueExpression, memRefs, trueFlags);

      SgExpression *falseExpression = conditionalExp->get_false_exp();
      ROSE_ASSERT(falseExpression != NULL);
      
      // Propagate this node's flags to its child.
      unsigned falseFlags = flags;
      
      // Don't propagate DEF down to the child.
      removeFlag(falseFlags, expectDef);

      // Recurse on false expression.
      list<OA::OA_ptr<OA::MemRefExpr> > falseMemRefExprs;
      falseMemRefExprs = 
	findAllMemRefsAndMemRefExprs(falseExpression, memRefs, falseFlags);

      // i.e., ( a cond b ? c : d )

      // The result of evaluating a conditional expression is either
      // its true or its false expression.  Therefore, return any memory 
      // reference expressions from the true/false expressions.
      // These will needed, e.g., if we dereference the result of 
      // a conditional expression:  
      // int *a; int *b;  *(a < b ? a : b)
      curMemRefExprs.splice(curMemRefExprs.end(), trueMemRefExprs);
      curMemRefExprs.splice(curMemRefExprs.end(), falseMemRefExprs);

      break;
    }

  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(astNode);
      ROSE_ASSERT(initName != NULL);

      SgInitializer *initializer = initName->get_initializer();
      if ( initializer != NULL ) {
	
	// This is a definition, not just a declaration.
	OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::DEF;

	// Create an OpenAnalysis handle for this node.
	symHandle = mIR->getNodeNumber(initName);

	// The address of the memory location is not taken
	// during a declaration.
	addressTaken = false;

	// The declaration accurately represents the memory expression.
	fullAccuracy = true;

	// Create a NamedRef to represent this memory reference.
	isNamed = true;
	
	OA::OA_ptr<OA::MemRefExpr> memRefExp;
	memRefExp = new OA::NamedRef(addressTaken, fullAccuracy,
				     memRefType, symHandle);
	ROSE_ASSERT(!memRefExp.ptrEqual(0));

	isMemRefExpr = true;

	curMemRefExprs.push_back(memRefExp);

      }

      break;
    }

  case V_SgVarRefExp:
    {

      SgVarRefExp *varRefExp = isSgVarRefExp(astNode);
      ROSE_ASSERT(varRefExp != NULL);

      // Get the memory reference type (DEF, USE, etc.) for 
      // this node.
      memRefType = flagsToMemRefType(flags);
      
      // Is the address taken in this memory reference?
      addressTaken = 
	( flags & expectAddrOf );
      
      // Access to a named variable is a fully accurate
      // representation of that access, unless it occurs
      // in the context of an array reference expression,
      // arrow/dot expression, etc.  In those case, the
      // inaccuracy of the representation will be handled
      // at those nodes.
      fullAccuracy = true;
	  
      // Create a named memory reference for this access.
      SgVariableSymbol *symbol = varRefExp->get_symbol(); 
      ROSE_ASSERT(symbol != NULL); 

      SgInitializedName *initName = symbol->get_declaration(); 
      ROSE_ASSERT(initName != NULL); 

      // Create an OpenAnalysis handle for this node.
      symHandle = mIR->getNodeNumber(initName);

      // Create a named memory reference expression.
      OA::OA_ptr<OA::MemRefExpr> memRefExp;
      memRefExp = new OA::NamedRef(addressTaken, fullAccuracy,
				   memRefType, symHandle);
      ROSE_ASSERT(!memRefExp.ptrEqual(0));

      isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      break;
    }

  case V_SgFunctionRefExp:
    {
      // A FunctionRefExp can appear on the right-hand side of
      // an assignment to a function pointer.  We could check
      // that it is the right-hand side of an assignment or that
      // its parent is not a SgFunctionCallExp.  However,
      // we instead assume that we have not recursed to this
      // node unless it is used in the context of an access.
      SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(astNode);
      ROSE_ASSERT(functionRefExp != NULL);

      // We should only be using this SgFunctionRefExp as a
      // DEF/USE if it is on the rhs of an assignment.
      // Note there could be casts, etc., so we wouldn't be
      // guaranteed in such cases that our direct parent is
      // an assignment.  Let's assume that if we aren't
      // ignoring this mem ref expression, it is used on the
      // rhs.  In those cases, we implicitly take the address
      // of the function.
      addressTaken = true;

      // Access to a named function is a fully
      // accurate representation of that access.
      fullAccuracy = true;
	  
      // Get the memory reference type (DEF, USE, etc.) for 
      // this node.
      memRefType = flagsToMemRefType(flags);

      // This had better be a USE ...
      ROSE_ASSERT(memRefType == OA::MemRefExpr::USE);
      
      // Get the declaration of the function.
      SgFunctionSymbol *functionSymbol = functionRefExp->get_symbol();
      ROSE_ASSERT(functionSymbol != NULL);
      
      SgFunctionDeclaration *functionDeclaration = 
	functionSymbol->get_declaration();
      ROSE_ASSERT(functionDeclaration != NULL);

      // Create an OpenAnalysis handle for this node.
      symHandle = mIR->getNodeNumber(functionDeclaration);

      // Create a named memory reference expression.
      OA::OA_ptr<OA::MemRefExpr> memRefExp;
      memRefExp = new OA::NamedRef(addressTaken, fullAccuracy,
				   memRefType, symHandle);
      ROSE_ASSERT(!memRefExp.ptrEqual(0));

      isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      break;
    }


  case V_SgMemberFunctionRefExp:
    {
      // A MemberFunctionRefExp appears on the right-hand side
      // of an arrow or dot expression during a method invocation.
      // We consider this a USE.

      SgMemberFunctionRefExp *memberFunctionRefExp = 
	isSgMemberFunctionRefExp(astNode);
      ROSE_ASSERT(memberFunctionRefExp != NULL);

      // This is a USE (of an entry in the virtual function table).
      // We are not taking any addresses.
      addressTaken = false;

      // Access to a named function is a fully
      // accurate representation of that access.
      fullAccuracy = true;
	  
      // Get the memory reference type (DEF, USE, etc.) for 
      // this node.  
      memRefType = flagsToMemRefType(flags);

      // This had better be a USE ...
      ROSE_ASSERT(memRefType == OA::MemRefExpr::USE);
      
      // Get the declaration of the function.
      SgFunctionSymbol *functionSymbol = memberFunctionRefExp->get_symbol();
      ROSE_ASSERT(functionSymbol != NULL);
      
      SgFunctionDeclaration *functionDeclaration = 
	functionSymbol->get_declaration();
      ROSE_ASSERT(functionDeclaration != NULL);

      // Create an OpenAnalysis handle for this node.
      symHandle = mIR->getNodeNumber(functionDeclaration);

      // Create a named memory reference expression.
      OA::OA_ptr<OA::MemRefExpr> memRefExp;
      memRefExp = new OA::NamedRef(addressTaken, fullAccuracy,
				   memRefType, symHandle);
      ROSE_ASSERT(!memRefExp.ptrEqual(0));

      isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      break;
    }

  case V_SgNewExp:
    {
      SgNewExp *newExp = isSgNewExp(astNode);
      ROSE_ASSERT(newExp != NULL);

      // Invocations of new are represented by unnamed memory references,
      // just as calls to malloc (above).

      memRefType = OA::MemRefExpr::USE;

      // Create an OpenAnalysis handle for this node.
      stmtHandle = mIR->getNodeNumber(newExp);

      // new computes the address of a memory location,
      // so consider it an addressOf operation.
      addressTaken = true;

      // The new call does _not_ accurately represent the memory 
      // expression, as this would require the precise calling context.
      fullAccuracy = false;

      // Create an unnamed memory reference expression.
      OA::OA_ptr<OA::MemRefExpr> memRefExp;
      memRefExp = new OA::UnnamedRef(addressTaken, fullAccuracy,
				     memRefType, stmtHandle);
      ROSE_ASSERT(!memRefExp.ptrEqual(0));
      
      isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      break;
    }

  case V_SgPointerDerefExp:
    {
      SgPointerDerefExp *pointerDerefExp = 
	isSgPointerDerefExp(astNode);
      ROSE_ASSERT(pointerDerefExp != NULL);

      SgExpression *operand = pointerDerefExp->get_operand_i();
      ROSE_ASSERT(operand != NULL);

      SgNode *parent = pointerDerefExp->get_parent();
      ROSE_ASSERT(parent != NULL);

      // Recurse on the operand.
      unsigned childFlags = flags;

      // Consume expectArrayBase and expectArrowDotRHS.  If either
      // or set, we will account for it here by not storing
      // a MemRefHandle/MemRefExpr for this node (though we will
      // create it and pass it back to the caller for potential use
      // there).
      removeFlag(childFlags, expectArrayBase);
      removeFlag(childFlags, expectArrowDotRHS);

      // Consume expectDef so that the children don't become DEFs.
      // We will create a mem ref expression at this node to
      // represent the DEF.
      removeFlag(childFlags, expectDef);

      list<OA::OA_ptr<OA::MemRefExpr> > operandMemRefExprs;
      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(operand, memRefs, childFlags);

      // Iterate over all of the memory reference expressions.
      bool hasOneOrMoreChildMemRefExps = false;

      if ( isSgDotExp(parent) ) {

	// This dereference occurs in the context of (*a).b
	// This will be treated as a->b when we reach the dot
	// expression.  For now, don't do anything except
	// return the operand's mem ref expressions.
	// Since we are only propagating the MemRefExprs, set
	// the flag to indicate that we should not store any
	// with this node.

	for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = operandMemRefExprs.begin();
	    it != operandMemRefExprs.end(); ++it) {
	  
	  OA::OA_ptr<OA::MemRefExpr> memRefExp = *it;
	  ROSE_ASSERT(!memRefExp.ptrEqual(0));

	  curMemRefExprs.push_back(memRefExp);
	  
	  hasOneOrMoreChildMemRefExps = true;

	} // end for

      } else {
	
	// The parent is not a dot expression.  For each
	// memory reference expression at the child, create
	// a corresponding Deref expression as we did above 
	// for arrow.

	// Is the address taken in this memory reference?
	addressTaken = ( flags & expectAddrOf );

	// A dereference fully represents a memory reference expression.
	fullAccuracy = true;
      
	// Get the memory reference type (DEF, USE, etc.) for 
	// this node.
	memRefType = flagsToMemRefType(flags);

	for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = operandMemRefExprs.begin();
	    it != operandMemRefExprs.end(); ++it) {
	  
	  OA::OA_ptr<OA::MemRefExpr> memRefExp = *it;
	  ROSE_ASSERT(!memRefExp.ptrEqual(0));

	  int numDerefs = 1;
	  
	  OA::OA_ptr<OA::Deref> deref;
#ifndef BWHITE_VERSION
	  deref = new OA::Deref(memRefExp, numDerefs);
#else
	  deref = new OA::Deref(addressTaken,
				fullAccuracy,
				memRefType,
				memRefExp,
				numDerefs);
#endif
	  ROSE_ASSERT(!deref.ptrEqual(0));

#ifdef BWHITE_VERSION
	  deref->setAccuracy(fullAccuracy);
	  deref->setAddressTaken(addressTaken);
	  deref->setMemRefType(memRefType);
#endif
	  curMemRefExprs.push_back(deref);
	  
	  isMemRefExpr = true;

	  hasOneOrMoreChildMemRefExps = true;

	}

      }

      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);

      break;
    }

  default:
    {
      handleDefaultCase(astNode, memRefs, flags);

      break;
    }
  }

  // If this node is a LHS, we will store the MemRefExpr/MemRefHandle
  // at the assign node, not here.  Notice that if is a DefUse or a UseDef
  // we still store the MemRefExpr/MemRefHandle.  Thus, for a = b = c,
  // we store the USE of c at c, the DEFUSE of b at the top-most assign,
  // and the DEF of a at the bottom-most assign.
  bool isLHS = false;
#if 0
  if ( ( flags & expectDef ) 
#if 1
       ||
       ( flags & expectDefUse ) ||
       ( flags & expectUseDef ) 
#endif
       )
    isLHS = true;
#endif

  if ( ( flags & expectArrayBase ) || 
       ( flags & expectArrowDotRHS ) ||
       ( flags & expectAddrOf ) ||
       ( isLHS ) )
    isMemRefExpr = false;

  // If we created a MemRefExpr above and need to store it,
  // do so now.  Notice that under some circumstances we do
  // not store newly created MemRefExprs, but do return
  // them to the caller for further processing.
  if ( isMemRefExpr ) {

    // list of MemRefHandles we are building
    OA::MemRefHandle memRefHandle = mIR->getNodeNumber(astNode);
    memRefs.push_back(memRefHandle);

    for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = curMemRefExprs.begin();
	it != curMemRefExprs.end(); ++it) {

      OA::OA_ptr<OA::MemRefExpr> mre = *it;

      // mapping of MemRefHandle to MemRefExprs
      SageIRInterface::sMemref2mreSetMap[memRefHandle].insert(mre);

    }

  }

  return curMemRefExprs;
}

OA::OA_ptr<OA::MemRefHandleIterator> 
SageIRInterface::getMemRefIterator(OA::StmtHandle h)
{
  OA::OA_ptr<OA::MemRefHandleIterator> retval;
  retval = new SageIRMemRefIterator(h, this);
  return retval;
}


