#include "MemSage2OA.h"

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

// If node represents a malloc or a new, return true and the
// type of the object allocated.  
bool SageIRInterface::isAllocation(SgNode *astNode, SgType *&type)
{

  if ( astNode == NULL ) return false;

  SgNode *node = astNode;

  bool isAlloc = false;

  switch(astNode->variantT()) {

  case V_SgNewExp:
    {
      SgNewExp *newExp = isSgNewExp(node);
      ROSE_ASSERT(newExp != NULL);
      
      type = newExp->get_type();
      ROSE_ASSERT(type != NULL);

      isAlloc = true;

      break;
    }

  case V_SgCastExp:
    {
      SgCastExp *castExp = isSgCastExp(node);
      ROSE_ASSERT(castExp != NULL);

      SgNode *node = castExp->get_operand();
      
      // fall through!
    }

  case V_SgFunctionCallExp:
    {
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      if ( functionCallExp != NULL ) {

	if ( isMalloc(functionCallExp) ) {
	  
	  // This is an allocation site.  malloc requires a cast.
	  // Find the nearest enclosing cast to determine the type.
	  
	  SgNode *parent = functionCallExp->get_parent();
	  while ( ( parent != NULL ) && ( !isSgCastExp(parent) ) )
	    parent = parent->get_parent();
	  
	  ROSE_ASSERT(parent != NULL);
	  
	  SgCastExp *castExp = isSgCastExp(parent);
	  ROSE_ASSERT(castExp != NULL);
	  
	  type = castExp->get_type();
	  ROSE_ASSERT(type != NULL);
	  
	  
	}

      }
      
      break;
      
    }

  default:
    {
      break;
    }
  }

  return isAlloc;

}

bool SageIRInterface::isMalloc(SgFunctionCallExp *functionCallExp)
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

std::map<OA::IRHandle,std::set<OA::MemRefHandle> >
SageIRInterface::sStmt2allMemRefsMap;

std::map<OA::MemRefHandle,OA::IRHandle> SageIRInterface::sMemRef2StmtMap;

std::map<OA::MemRefHandle,set<OA::OA_ptr<OA::MemRefExpr> > >
SageIRInterface::sMemref2mreSetMap;

std::map<OA::OA_ptr<OA::MemRefExpr>,OA::MemRefHandle >
SageIRInterface::sMre2MemrefMap;

SageIRMemRefIterator::SageIRMemRefIterator(OA::IRHandle h, 
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
SageIRMemRefIterator::create(OA::IRHandle h)
{
  SgNode *node = mIR->getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  // if haven't already determined the set of memrefs for this stmt
  // then do so by finding all the top memory references and then
  // initializing the mapping of MemRefHandle's to a set of MemRefExprs,
  // and based off that map get all the MemRefHandle's
  if (SageIRInterface::sStmt2allMemRefsMap[h].empty() ) {

    // get all the top memory references
    list<SgNode *>* topMemRefs = mIR->findIndependentMemRefs(node);
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
	    SageIRInterface::sStmt2allMemRefsMap[h].insert(*mrIter);
	    SageIRInterface::sMemRef2StmtMap[*mrIter] = h;
	  }
      }
    delete topMemRefs;
  }

  // loop through MemRefHandle's for this statement and for now put them
  // into our own list
  std::set<OA::MemRefHandle>::iterator setIter;
  for (setIter=SageIRInterface::sStmt2allMemRefsMap[h].begin();
       setIter!=SageIRInterface::sStmt2allMemRefsMap[h].end(); setIter++)
    {
      mMemRefList.push_back(*setIter);
    }

}

// isMemRefNode returns true if there should be a
// MemRefHandle associated with astNode.  
bool SageIRInterface::isMemRefNode(SgNode *astNode)
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
  case V_SgThisExp:
    {
      retVal = true;
      break;
    }
  case V_SgVarRefExp:
  case V_SgNewExp:
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
  case V_SgCastExp:
    {
      SgCastExp *castExp = isSgCastExp(astNode);
      ROSE_ASSERT(castExp != NULL);

      // Return the unnamed memory reference from a malloc's cast.
      SgNode *node = castExp->get_operand();
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      retVal = ( ( functionCallExp != NULL ) && ( isMalloc(functionCallExp) ) );

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

#if 0
	// Represent malloc'ed memory by an unnamed memory reference.
	retVal = true;
#endif
	// No!  Return the unnamed memory reference from a malloc's cast.
	retVal = false;

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
#if 0
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
#endif
#if 0
  case V_SgCommaOpExp:
    {
      SgCommaOpExp *commaOp = isSgCommaOpExp(astNode);
      ROSE_ASSERT(commaOp != NULL);

      SgExpression *lhs = commaOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      SgExpression *rhs = commaOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      if ( isMemRefNode(lhs) || isMemRefNode(rhs) ) {
	retVal = true;
      }

      break;
    }
#endif
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
	
	if ( isSgPntrArrRefExp(lhs) )
	  retVal = true;

      }
	
      break;

    }
#ifdef VTABLE_OPT
  case V_SgClassDefinition:
    {
      // When using the vtable-optimization for virtual method
      // resolution (via FIAlias), we return implicit assignments
      // at a class definition.  For each method m of a class C, we get an 
      // implicit assignment of the form < C.m, C::m >, i.e.,
      // < FieldAccess(NamedRef(SymHandle(class C)),FieldHandle(m)), 
      //   NamedRef(SymHandle(C::m)) >
      retVal = true;
      break;
    }
#endif
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
// Also the constructor invoked during a new is independent of
// the SgNewExp, which represents the unnamed memory returned.
void
SageIRInterface::getChildrenWithMemRefs(SgNode *astNode,
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
#if 0
  case V_SgCommaOpExp:
    {
      SgCommaOpExp *commaOp = isSgCommaOpExp(astNode);
      ROSE_ASSERT(commaOp != NULL);

      SgExpression *lhs = commaOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      SgExpression *rhs = commaOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      independentChildren.push_back(lhs);
      // The rhs is not independent, since its value is held/returned
      // by the CommaOpExp node.
      //      independentChildren.push_back(rhs);

      break;
    }
#endif
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
	independentChildren.push_back(expression);
	
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

  case V_SgInitializedName:
    {
      // The initializer is independent of the variable declaration.

      SgInitializedName *initName = isSgInitializedName(astNode);
      ROSE_ASSERT(initName != NULL);

      // If the initialized name has an initializer, it is
      // a definition (i.e., a DEF) and not simply a declration.
      SgInitializer *initializer = initName->get_initializer();
      if ( initializer != NULL ) {
	children.push_back(initializer);
	//	independentChildren.push_back(initializer);
      }

      break;
    }

  case V_SgNewExp:
    {
      SgNewExp *newExp = isSgNewExp(astNode);
      ROSE_ASSERT(newExp != NULL);

      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      children.push_back(ctorInitializer);
      independentChildren.push_back(ctorInitializer);

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
SageIRInterface::findTopMemRefs(SgNode *astNode)
{
  std::list<SgNode *> *topMemRefs = new std::list<SgNode *>;
  findIndependentMemRefs(astNode, *topMemRefs, true);
  return topMemRefs;
}

// findIndependentMemRefs: Given a SgNode representing a statement, 
// recursively find the independent memory references in the 
// statement/expression.  An independent memory reference is in the 
// same basic block as astNode and has no memory references
// proceeding it in its expression.
// The basic difference between this method and findTopMemRefs
// is in the handling of function calls and arrays:
// the args to calls and the index expressions to arrays are
// independent memory references (i.e., independent of
// the function call and array reference expression), but are
// not top expressions (i.e., not the first memory reference
// seen in the expression).
// Inspired by Open64IRMemRefIterator::findTopMemRefs.
std::list<SgNode *>* 
SageIRInterface::findIndependentMemRefs(SgNode *astNode)
{
  std::list<SgNode *> *topMemRefs = new std::list<SgNode *>;
  findIndependentMemRefs(astNode, *topMemRefs, false);
  return topMemRefs;
}

void 
SageIRInterface::findIndependentMemRefs(SgNode *astNode, 
					std::list<SgNode *>& topMemRefs,
					bool collectOnlyTopMemRefs)
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

    if ( collectOnlyTopMemRefs == true ) return;

    // Recurse only on independent children.  Currently,
    // the only independent children are the actual arguments
    // of a SgFunctionCallExp, the implicit actual arg of a
    // method invocation (the lhs of a dot or arrow), 
    // and the index expressions of a SgPntrArrRefExp.
    getChildrenWithMemRefs(astNode, independentChildren, children);

    for(vector<SgNode *>::iterator it = independentChildren.begin();
	it != independentChildren.end(); ++it) {

      SgNode *node = *it;
      ROSE_ASSERT(node != NULL);
      
      findIndependentMemRefs(node, topMemRefs, collectOnlyTopMemRefs);
      
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
    
    findIndependentMemRefs(node, topMemRefs, collectOnlyTopMemRefs);

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
  unsigned synthesizedFlags = 0;
  findAllMemRefsAndMemRefExprs(astNode, *memRefs, 0, synthesizedFlags);
  return memRefs;
}

// SageRefExprInheritedFlagType are flags that are passed down to 
// a SgNode during a traversal.  So, 'the node' below means 
// 'the node receiving the flag'.
enum SageRefExpInheritedFlagType { 
  expectUse         = 1,
  expectDef         = 2,
  expectUseDef      = 4,  // use and then define
  expectDefUse      = 8,  // define and then use
  expectAddrOf      = 16,
  ignore            = 32, // do not treat node as def/use
  partial           = 64,
  expectArrayBase   = 128,// the node is an array base
  expectArrowDotRHS = 256,// the node is the rhs of an arrow/dot expression
  expectRefRhs      = 512,// the node is the rhs of a reference assignment
  expectRhsComputesLValue      = 1024, // the node is a lhs with a rhs that
                                       // computes an lvalue
  expectRhsDoesntComputeLValue = 2048, // the node is a lhs with a rhs that
                                       // does not compute an lvalue
  expectDotLHS                 = 4096, // the node is the lhs of a dot expression
  expectDontApplyConversion    = 8192, // don't apply reference conversion rules
  takeAddrOfAndStoreMRE        = 16384, // take the address of the expression, but still store the MRE at the node (unlike expectAddrOf which does not).
  expectAddrReturningFunc      = 32768, // the node is a function which returns an address (pointer or reference)
}; 

// SageRefExpSynthesizedFlagType are flags that are passed up from
// a SgNode during a traversal.  So, 'the node' below means 'the
// node originating this flag'.
enum SageRefExpSynthesizedFlagType { 
  synthesizedComputesLValue         = 8192, // the node computes something 
                                            // whose address we could take
  synthesizedDoesntComputeLValue    = 16384, 
};

// addInheritedFlag appends flag to flags, which is intended to
// be an inherited value passed down to a node's children during
// an AST traversal.
static void addInheritedFlag(unsigned &flags, const SageRefExpInheritedFlagType &flag) {
  
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

  case takeAddrOfAndStoreMRE:
    {
      flags |= takeAddrOfAndStoreMRE;
      
      break;
    }
    
  case expectAddrReturningFunc:
    {
      flags |= expectAddrReturningFunc;

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

  case expectDotLHS:
    {
      flags |= expectDotLHS;

      break;
    }

  case expectRhsComputesLValue:
    {
      flags |= expectRhsComputesLValue;
      
      break;
    }

  case expectRhsDoesntComputeLValue:
    {
      flags |= expectRhsDoesntComputeLValue;
      
      break;
    }

  case expectRefRhs:
    {
      flags |= expectRefRhs;
      
      break;
    }

  case expectDontApplyConversion:
    {
      flags |= expectDontApplyConversion;

      break;
    }

  default:
    {
      break;
    }
    
  }
  
}

// addSynthesizedFlag appends flag to flags, which is intended to
// be a synthesized value passed up from a node's children to 
// the node during an AST traversal.
static void addSynthesizedFlag(unsigned &flags, const SageRefExpSynthesizedFlagType &flag) {
  
  switch(flag) {

  case synthesizedComputesLValue:
    {
      flags |= synthesizedComputesLValue;
      
      break;
    }

  case synthesizedDoesntComputeLValue:
    {
      flags |= synthesizedDoesntComputeLValue;
      
      break;
    }

  default:
    {
      break;
    }
    
  }
  
}

// removeInheritedFlag removes flag from flags, which is intended to
// be an inherited value passed down to a node's children during
// an AST traversal.
static void removeInheritedFlag(unsigned &flags, const SageRefExpInheritedFlagType &flag) {

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

  case takeAddrOfAndStoreMRE:
    {
      flags &= ~takeAddrOfAndStoreMRE;
      
      break;
    }

  case expectAddrReturningFunc:
    {
      flags &= ~expectAddrReturningFunc;

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

  case expectDotLHS:
    {
      flags &= ~expectDotLHS;

      break;
    }

  case expectRhsComputesLValue:
    {
      flags &= ~expectRhsComputesLValue;
      
      break;
    }

  case expectRhsDoesntComputeLValue:
    {
      flags &= ~expectRhsDoesntComputeLValue;
      
      break;
    }

  case expectRefRhs:
    {
      flags &= ~expectRefRhs;
      
      break;
    }

  case expectDontApplyConversion:
    {
      flags &= ~expectDontApplyConversion;

      break;
    }

  default:
    {
      break;
    }
    
  }
  
}

// removeSynthesizedFlag removes flag from flags, which is intended to
// be a synthesized value passed up from a node's children to 
// the node during an AST traversal.
static void removeSynthesizedFlag(unsigned &flags, const SageRefExpSynthesizedFlagType &flag) {
  
  switch(flag) {

  case synthesizedComputesLValue:
    {
      flags &= ~synthesizedComputesLValue;
      
      break;
    }

  case synthesizedDoesntComputeLValue:
    {
      flags &= ~synthesizedDoesntComputeLValue;
      
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
					unsigned flags,
					unsigned &synthesizedFlags)
{
  // Pass flags down to children.
  
  vector<SgNode *> containerList = 
    astNode->get_traversalSuccessorContainer();
  
  // Iterate over all children and recurse.
  for(vector<SgNode *>::iterator it = containerList.begin();
      it != containerList.end(); ++it) {

    SgNode *node = *it;
    if ( node != NULL ) {

      findAllMemRefsAndMemRefExprs(node, memRefs, flags, synthesizedFlags);
      
    }
 
  }
  
}

// dereferenceMre creates a Deref MemRefExpr that models a
// dereference of mre.  Note that address of and dereference are
// compensating actions.  Therefore, if mre has its address taken,
// dereferencing it simply sets this flag to false.
OA::OA_ptr<OA::MemRefExpr> 
SageIRInterface::dereferenceMre(OA::OA_ptr<OA::MemRefExpr> mre)
{
  OA::OA_ptr<OA::MemRefExpr> deref;
  
  bool addressTaken                     = false;
  bool fullAccuracy                     = true;
  int  numDerefs                        = 1;

  OA::MemRefExpr::MemRefType memRefType = getMemRefType(mre);

  if ( mre->hasAddressTaken() ) {

    // If the address of this MemRefExpr is taken,
    // simply set address taken to false since derefernce and address of
    // are compensating/inverse actions.  
    deref = mre->clone();
    deref->setAddressTaken(addressTaken);
    deref->setMemRefType(memRefType);

  } else {

    OA::OA_ptr<OA::MemRefExpr> baseMre = mre->clone();
    // The dereferences take the DEF/USE behavior of the mre.
    // As it is dereferenced, the mre becomes a USE.
    baseMre->setMemRefType(OA::MemRefExpr::USE);

    deref = new OA::Deref(addressTaken,
			  fullAccuracy,
			  memRefType,
			  baseMre,
			  numDerefs);
    if ( !baseMre->hasFullAccuracy() )
      deref->setAccuracy(false);
  }

  return deref;

}

// takeAddressOfMre creates a MemRefExpr that represents the
// address of mre.  Basically, this simply sets the addressTaken
// flag to true.  Note however that address of and dereference 
// are compensating actions.  Therefore if mre is a Deref, then
// taking its address simply yields the base MemRefExpr that was
// dereferenced.  This is _only_ true if the Deref models a
// fully accurate dereference (i.e., *a).  It is also currently
// used to model field accesses (i.e., a->b).  
OA::OA_ptr<OA::MemRefExpr> 
SageIRMemRefIterator::takeAddressOfMre(OA::OA_ptr<OA::MemRefExpr> mre)
{
  OA::OA_ptr<OA::MemRefExpr> addrOf;

  OA::MemRefExpr::MemRefType memRefType = mIR->getMemRefType(mre);

  // But in reality, it only makes sense to take the address of
  // something on the right-hand side.  So, ensure that the thing
  // we return will be a USE.
  ROSE_ASSERT(memRefType == OA::MemRefExpr::USE);

  if ( mre->isaRefOp() ) {
 
    OA::OA_ptr<OA::RefOp> refOp = mre.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    if ( refOp->isaDeref() && refOp->hasFullAccuracy() ) {
      
      OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = refOp->getMemRefExpr();

      addrOf = baseMemRefExpr->clone();
      ROSE_ASSERT(!addrOf.ptrEqual(0));

      addrOf->setMemRefType(memRefType);
      return addrOf;

    }

  }
      
  // The mre was not a Deref.  Simply set its addressTaken flag to true.
  addrOf = mre->clone();
  ROSE_ASSERT(!addrOf.ptrEqual(0));

  addrOf->setAddressTaken();

  return addrOf;

}

// Return true if mre computes an lvalue-- that is, we could take
// its address.
bool 
SageIRMemRefIterator::mreComputesLValue(OA::OA_ptr<OA::MemRefExpr> mre)
{
  if ( mre->hasAddressTaken() ) 
    return false;

  if ( mre->isaNamed() )
    return true;

  if ( mre->isaRefOp() ) {
 
    OA::OA_ptr<OA::RefOp> refOp = mre.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    // This is assumed to be either a fully accurate Deref
    // (i.e., representing *sym) or an inaccurate Deref
    // representing a field access (i.e., sym->mem).  
    // In either case we can take its address.
    if ( refOp->isaDeref() ) 
      return true;

  }

  return false;

}

// Apply the reference conversion rules:
// - For every initialization of a referenece:  t_l& lhs = rhs
//   (including the implicit assigment during parameter binding
//   and intialization in a constructor's initializer list)
//  1.  Convert lhs from a reference to a pointer type, t_l *lhs = ...
//  2.  If the rhs does not have an lvalue (e.g., 3+5, 5, &y, ...),
//      then replace lhs with *lhs.
//  3.  Whenever lhs is used on a right-hand side or in an
//      assignment (lhs = rhs), replace lhs with *lhs.  Since references
//      are only assigned once (during initialization), a reference's
//      appearance on the lhs of an assignment modifies the data to which
//      it refers, not to the reference itself.
// - For every rhs corresponding to the above initialization cases: t_l &lhs = rhs
//  4.  If the rhs has an lvalue, then replace rhs with &rhs.
// Returns converted and/or unconverted MemRefExprs in the out parameter
// convertedMemRefs.
// Returns in computesLValue whether mre computes an lvalue (independent
// of whether it actually is an lvalue).
// hasRhsThatComputesLValue indicates that mre appears on a lhs and
// its rhs computes an lvalue.
// hasRhsThatDoesntComputeLValue indicates that mre appears on a lhs and
// its rhs does not compute an lvalue.
// initializedRef indicates that the astNode is a reference which
// is initialized within astNode's enclosing statement.
// Note that one, both, or none of the hasRhs*LValue bools may be true.
// Both may be true if mre is on a lhs and the assignment might
// look like:
// a = ( cond ? d : e );
// where d computes an lvalue and e does not.
// Both are false if mre does not occur on the lhs.
void
SageIRMemRefIterator::applyReferenceConversionRules(OA::OA_ptr<OA::MemRefExpr> mre,
						    SgNode *astNode, 
						    bool appearsOnRhsOfRefInitialization,
						    bool hasRhsThatComputesLValue,
						    bool hasRhsThatDoesntComputeLValue,
						    bool initializedRef,
						    std::list<OA::OA_ptr<OA::MemRefExpr> > &convertedMemRefs)
{
  std::list<OA::OA_ptr<OA::MemRefExpr> > tmpMemRefs;
  bool isReference = false;

  OA::MemRefExpr::MemRefType memRefType = mIR->getMemRefType(mre);

  // Does mre represent a reference?
  SgType *type = NULL;
  SgExpression *expression = isSgExpression(astNode);
  if ( expression != NULL ) {

    type = expression->get_type();

  } else {

    SgInitializedName *initName = isSgInitializedName(astNode);
    if ( initName != NULL) { 
      type = initName->get_type();
    }

  }

  if ( isSgReferenceType( mIR->getBaseType(type) ) ) {
    isReference = true;
  }

  // The rules do not apply to MemRefExprs which are neither references
  // nor on the rhs of a reference assignment.
  if ( !isReference && !appearsOnRhsOfRefInitialization ) {
    convertedMemRefs.push_back(mre);  
    return;
  }

  // Apply conversions to USEs.
  OA::OA_ptr<OA::MemRefExpr> convertedMreUse;
  convertedMreUse = NULL;
  if ( mre->isUse() ) {

    // - For every assignment to a referenece:  t_l& lhs = rhs
    //  3.  Whenever lhs is used on a right-hand side, replace lhs with *lhs.
    if ( isReference ) {
      convertedMreUse = mIR->dereferenceMre(mre);
    } else {
      convertedMreUse = mre->clone();
    }

    // - For every rhs corresponding to a reference assignment: t_l &lhs = rhs
    //  4.  If the rhs has an lvalue, then replace rhs with &rhs.
    // Note that rhs does not itself have to be a reference.
    if ( appearsOnRhsOfRefInitialization ) {

      if ( mreComputesLValue(mre) ) {
//	computesLValue = true;
	convertedMreUse = takeAddressOfMre(convertedMreUse);
      } else {
	//	computesLValue = false;
      }

    }

    // This may have previously been a DEFUSE or USEDEF, but
    // it is strictly a USE now.  We'll create the DEFs below.
    // We need to separate them since the conversion rules
    // treat USEs and DEFs differently.
    convertedMreUse->setMemRefType(OA::MemRefExpr::USE);
  }

  // Apply conversions to DEFs.
  OA::OA_ptr<OA::MemRefExpr> derefedLhs;
  derefedLhs = NULL;

  OA::OA_ptr<OA::MemRefExpr> origLhs;
  origLhs = NULL;
  if ( mre->isDef() ) {
    
    // A DEF by definition computes an lvalue.
    //    computesLValue = true;

    // - For every assignment to a referenece:  t_l& lhs = rhs
    //  2.  If the rhs does not have an lvalue (e.g., 3+5, 5, &y, ...),
    //      then replace lhs with *lhs.
    if ( !isReference ) {

      origLhs = mre->clone();

    } else {

      // Implied corollary:  if the rhs does not compute an lvalue,
      // keep lhs as is.  If we have multiple rhs, we may need both
      // lhs and *lhs.
      if ( hasRhsThatDoesntComputeLValue || !initializedRef ) {
	derefedLhs = mIR->dereferenceMre(mre);
	// As we did above with USE, ensure this isn't a DEFUSE or USEDEF.
	derefedLhs->setMemRefType(OA::MemRefExpr::DEF);
      }
      
      if ( ( hasRhsThatComputesLValue || !hasRhsThatDoesntComputeLValue ) &&
	   initializedRef ) {
	origLhs = mre->clone();
	// As we did above with USE, ensure this isn't a DEFUSE or USEDEF.
	origLhs->setMemRefType(OA::MemRefExpr::DEF);
      }
    }
  }

  if ( ( memRefType == OA::MemRefExpr::DEFUSE ) || 
       ( memRefType == OA::MemRefExpr::USEDEF ) ) {

    // Above we may have split a DEFUSE/USEDEF into a USE and a DEF.
    // If we have created two MemRefExprs that differ only in their 
    // memRefType (i.e., USE or DEF), then merge them into a USEDEF
    // or DEFUSE.  Note that this makes sense since these two 
    // "different" MemRefExprs actually correspond to the same
    // expression (we only handle one expression in this method).
    
    // We know that convertedMreUse has type USE and the lhs have type
    // DEF.  Neither is a USEDEF or DEFUSE by construction (above).
    // Therefore, the easiest way to compare these is to set
    // the memRefType off convertedMreUse to DEF and then change it back.
    if ( !convertedMreUse.ptrEqual(0) && !derefedLhs.ptrEqual(0) ) {
      convertedMreUse->setMemRefType(OA::MemRefExpr::DEF);
      if ( convertedMreUse == derefedLhs ) {
	// Merge they in the derefedLhs with the memRefType of the
	// mre passed in to this method.
	derefedLhs->setMemRefType(memRefType);
	convertedMreUse = NULL;
      } else {
	convertedMreUse->setMemRefType(OA::MemRefExpr::USE);
      }
    }
    
    if ( !convertedMreUse.ptrEqual(0) && !origLhs.ptrEqual(0) ) {
      convertedMreUse->setMemRefType(OA::MemRefExpr::DEF);
      if ( convertedMreUse == origLhs ) {
	// Merge they in the origLhs with the memRefType of the
	// mre passed in to this method.
	origLhs->setMemRefType(memRefType);
	convertedMreUse = NULL;
      } else {
	convertedMreUse->setMemRefType(OA::MemRefExpr::USE);
      }      
    }

  }

  if ( !convertedMreUse.ptrEqual(0) ) {
    convertedMemRefs.push_back(convertedMreUse);
  }

  if ( !derefedLhs.ptrEqual(0) ) {
    convertedMemRefs.push_back(derefedLhs);
  }

  if ( !origLhs.ptrEqual(0) ) {
    convertedMemRefs.push_back(origLhs);
  }

}


// Return a memory reference expression for varRefExp.
// If varRefExp is an access to a class member m, then
// this method will explicitly model the dereference of this
// and create an MRE for this->m.
OA::OA_ptr<OA::MemRefExpr> 
SageIRInterface::createMRE(SgVarRefExp *varRefExp,
			   bool addressTaken,
			   bool fullAccuracy,
			   OA::MemRefExpr::MemRefType memRefType,
			   bool arrowOrDotModeledElsewhere)
{
  ROSE_ASSERT(varRefExp != NULL);

  // Create a named memory reference for this access.
  SgVariableSymbol *symbol = varRefExp->get_symbol(); 
  ROSE_ASSERT(symbol != NULL); 
  
  SgInitializedName *initName = symbol->get_declaration(); 
  ROSE_ASSERT(initName != NULL); 
  
  OA::OA_ptr<OA::MemRefExpr> memRefExp;
  memRefExp = createMRE(initName, 
			addressTaken, 
			fullAccuracy, 
			memRefType,
			arrowOrDotModeledElsewhere);

  return memRefExp;
}

// Return a memory reference expression for initName.
// If initName is an access to a class member m, then
// this method will explicitly model the dereference of this
// and create an MRE for this->m.
OA::OA_ptr<OA::MemRefExpr> 
SageIRInterface::createMRE(SgInitializedName *initName,
			   bool addressTaken,
			   bool fullAccuracy,
			   OA::MemRefExpr::MemRefType memRefType,
			   bool arrowOrDotModeledElsewhere)
{
  ROSE_ASSERT(initName != NULL);

  SgDeclarationStatement *declaration = initName->get_declaration();
  ROSE_ASSERT(declaration != NULL);

  bool memberVariable = false;

  // If the variable is defined in the parameter list, it is not
  // a member variable.  This isn't accurate.  It still could be
  // a member variable.  What I mean is that it is not have an
  // implicit 'this', but rather is accessed through a fully-quantified
  // name.
  if ( !isSgFunctionParameterList(declaration) ) {

    SgClassDefinition *classDefinition =
      getDefiningClass(declaration->get_scope());
    memberVariable = ( classDefinition != NULL );

  }

  OA::OA_ptr<OA::MemRefExpr> memRefExp;

  if ( arrowOrDotModeledElsewhere || !memberVariable ) {

    // Either we will ignore this MemRefExpr since we are being called 
    // downstream of an arrow or dot expression which will 
    // model this reference.  Therefore, just return something
    // simple.
    // Or, this is not a reference to a member variable, and
    // so requires no special treatment.
    
    OA::SymHandle symHandle = getNodeNumber(initName);

    memRefExp = new OA::NamedRef(addressTaken, fullAccuracy,
				 memRefType, symHandle);

  } else {

    SgFunctionDefinition *functionDefinition = 
      getEnclosingMethod(initName);
    ROSE_ASSERT(functionDefinition != NULL);
    
    SgFunctionDeclaration *functionDeclaration =
      functionDefinition->get_declaration();
    ROSE_ASSERT(functionDeclaration != NULL);

    SgMemberFunctionDeclaration *memberFunctionDeclaration =
      isSgMemberFunctionDeclaration(functionDeclaration);
    ROSE_ASSERT(memberFunctionDeclaration != NULL);

    // This is a reference to a member variable v which is
    // not part of an arrow or dot expression.  i.e., it
    // is an implicit reference to this->v.  
    // Because we do not accurately model field accesses,
    // model it as a partially accurate Deref of this.
    OA::SymHandle symHandle = getThisExpSymHandle(memberFunctionDeclaration);
 
    OA::OA_ptr<OA::MemRefExpr> baseMre;
    baseMre = new OA::NamedRef(false, true, OA::MemRefExpr::USE, symHandle);
    ROSE_ASSERT(!baseMre.ptrEqual(0));

    // Notice that baseMre may have addressTaken.  When we
    // Deref it, we do not view addressTaken and dereference as
    // inverse operations, since the Deref actually (inaccurately)
    // models a field access.
    memRefExp = new OA::Deref(addressTaken, false,
			      memRefType, baseMre, 1);

  }

  return memRefExp;
}


// findAllMemRefsAndMemRefExprs (manually) traverses the AST rooted at
// astNode.  Whenever it encounters a node that hase a memory reference
// (and hence should have an associated MemRefHandle), it creates a 
// MemRefHandle and puts it in the list memRefs.  
// findAllMemRefsAndMemRefExprs creates the one or more MemRefExprs
// associated with a MemRefHandle.  It creates an explicit association
// between a MemRefHandle and its MemRefExprs by placing the MemRefExprs
// in the set sMemref2mreSetMap[MemRefHandle] and by creating the
// reversal map from MemRefExpr to MemRefHandle in sMre2MemrefMap.
// In creating MemRefExprs, findAllMemRefsAndMemRefExprs is responsible
// for converting references to pointers (such that OpenAnalysis need not
// concern itself with references, but sees their effects modeled as 
// pointers).  To do so, it implements the 
// reference conversion rules.
// flags are the inheritedFlags being passed down from a parent node in
// the traversal.  synthesizedFlags is the out parameter synthesized flags
// used to communicate flags up to a parent.
list<OA::OA_ptr<OA::MemRefExpr> >
SageIRMemRefIterator::findAllMemRefsAndMemRefExprs(SgNode *astNode,
						   list<OA::MemRefHandle>& memRefs,
						   unsigned flags,
						   unsigned &synthesizedFlags)
{
  bool isMemRefExpr                     = false;
  OA::SymHandle symHandle               = 0;
  OA::StmtHandle stmtHandle             = 0;
  bool addressTaken                     = true;
  bool fullAccuracy                     = false;
  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
  bool isNamed                          = false;
  
  bool hasRhsThatComputesLValue         = flags & expectRhsComputesLValue;
  bool hasRhsThatDoesntComputeLValue    = flags & expectRhsDoesntComputeLValue;
  bool appearsOnRhsOfRefInitialization      = flags & expectRefRhs;
  bool initializedRef                   = false;
  bool dontApplyConversion              = flags & expectDontApplyConversion;

  bool takeAddrAndStoreMRESet           = flags & takeAddrOfAndStoreMRE;
  bool addrReturningFunc                = flags & expectAddrReturningFunc;
  bool expectAddrOfSet                  = flags & expectAddrOf;

  removeInheritedFlag(flags, expectDontApplyConversion);
  removeInheritedFlag(flags, takeAddrOfAndStoreMRE);
  removeInheritedFlag(flags, expectAddrReturningFunc);

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

#if 0
  list<OA::OA_ptr<OA::MemRefExpr> > implicitMemRefExprs;
  implicitMemRefExprs = 
    handleImplicitPtrAssigns(astNode, memRefs, flags, synthesizedFlags);
  curMemRefExprs.splice(curMemRefExprs.end(), implicitMemRefExprs);
#endif

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

      SgExpression *lhs = binaryOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      SgExpression *rhs = binaryOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      // Add USE to the rhs child's flags.
      unsigned rhsFlags = flags;

      // Consume expectArrayBase and 
      // expectArrowDotRHS.  Memory
      // reference expressions within an assignment need not
      // account for them.
      removeInheritedFlag(rhsFlags, expectArrayBase);
      removeInheritedFlag(rhsFlags, expectArrowDotRHS);
      removeInheritedFlag(rhsFlags, expectDotLHS);

      if ( rhsFlags & expectDef ) {

	// See note above.
	addInheritedFlag(rhsFlags, expectUseDef);

      } else {

	addInheritedFlag(rhsFlags, expectUse);

      }

      SgType *lhsType = lhs->get_type();
      ROSE_ASSERT(lhsType != NULL);

#if 0
      // No!  Do not treat assignments as initializations.
      if ( isSgReferenceType( mIR->getBaseType(lhsType) ) )
	addInheritedFlag(rhsFlags, expectRefRhs);
#endif

      // Recurse on rhs.
      findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags,
				   synthesizedFlags);

      // Add DEF to the lhs operand's flags.
      unsigned lhsFlags = flags;

      // If these we passed from above, they apply to the rhs only.
      removeInheritedFlag(lhsFlags, expectRhsComputesLValue);
      removeInheritedFlag(lhsFlags, expectRhsDoesntComputeLValue);
      removeInheritedFlag(lhsFlags, expectRefRhs);

      if ( synthesizedFlags & synthesizedComputesLValue )
	addInheritedFlag(lhsFlags, expectRhsComputesLValue);

      if ( synthesizedFlags & synthesizedDoesntComputeLValue )
	addInheritedFlag(lhsFlags, expectRhsDoesntComputeLValue);

      // We don't care if the rhs computes an lvalue, since the
      // result of this expression is the lhs.
      removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      // Consume expectArrayBase and expectArrowDotRHS.  Memory
      // reference expressions within an assignment need not
      // account for them.
      removeInheritedFlag(lhsFlags, expectArrayBase);
      removeInheritedFlag(lhsFlags, expectArrowDotRHS);
      removeInheritedFlag(lhsFlags, expectDotLHS);
      
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
	addInheritedFlag(lhsFlags, expectDefUse);

      } else {

	addInheritedFlag(lhsFlags, expectDef);

      }

      // Recurse on lhs.
      list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;
      lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags,
						    synthesizedFlags);

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
      removeInheritedFlag(childFlags, expectArrayBase);
      removeInheritedFlag(childFlags, expectArrowDotRHS);
      removeInheritedFlag(childFlags, expectDotLHS);

      if ( mode == SgUnaryOp::prefix ) {
	// Memory reference is defined first and then used.
	addInheritedFlag(childFlags, expectDefUse);
      } else {
	// Memory reference is used first and then defined.
	addInheritedFlag(childFlags, expectUseDef);
      }

      // Recurse on operand.
      list<OA::OA_ptr<OA::MemRefExpr> > operandMemRefExprs;
      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(operand, memRefs, childFlags,
				     synthesizedFlags);
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
	
	memRefExpr->setMemRefType(memRefType);
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
      removeInheritedFlag(childFlags, expectArrayBase);
      removeInheritedFlag(childFlags, expectArrowDotRHS);
      removeInheritedFlag(childFlags, expectDotLHS);

      // Consume flags.
      removeInheritedFlag(childFlags, expectRhsComputesLValue);
      removeInheritedFlag(childFlags, expectRhsDoesntComputeLValue);

      addInheritedFlag(childFlags, expectAddrOf);

      // Recurse on operand.
      list<OA::OA_ptr<OA::MemRefExpr> > operandMemRefExprs;
      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(operand, memRefs, childFlags,
				     synthesizedFlags);

      // Whether this expression computes an lvalue is determined
      // solely by this node and not by its children.  Therefore,
      // ignore their computesLValue flags.
      removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      // An address operator does not compute an lvalue.
      addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      // Store here any memory reference expressions created at our
      // children.
      curMemRefExprs.splice(curMemRefExprs.end(), operandMemRefExprs);

      isMemRefExpr = true;

      break;

    }

  case V_SgCommaOpExp:
    {
      // The result of evaluating a comma expression is the right-hand side.
      // So only return the rhs' MRE.
      SgCommaOpExp *commaOp = isSgCommaOpExp(astNode);
      ROSE_ASSERT(commaOp != NULL);

      SgExpression *lhs = commaOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      SgExpression *rhs = commaOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      unsigned childFlags = flags;

      // Consume expectArrayBase and expectArrowDotRHS.  
      removeInheritedFlag(childFlags, expectArrayBase);
      removeInheritedFlag(childFlags, expectArrowDotRHS);
      removeInheritedFlag(childFlags, expectDotLHS);

      // Consume flags.
      removeInheritedFlag(childFlags, expectRhsComputesLValue);
      removeInheritedFlag(childFlags, expectRhsDoesntComputeLValue);

      // Recurse on both lhs and rhs, but don't store lhs MRE here.
      list<OA::OA_ptr<OA::MemRefExpr> > operandMemRefExprs;
      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(lhs, memRefs, childFlags,
				     synthesizedFlags);

      // Whether this expression computes an lvalue is determined
      // by the rhs, so ignore the lhs results.
      removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(rhs, memRefs, childFlags,
				     synthesizedFlags);

      // Store here any memory reference expressions created at our rhs
      // child.
      curMemRefExprs.splice(curMemRefExprs.end(), operandMemRefExprs);

      //      isMemRefExpr = true;

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

      SgExpression *rhs = binaryOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      unsigned rhsFlags = flags;

      // Do _not_ consume expectArrayBase or expectArrowDotRHS
      // from the rhs of an arrow/dot expression since we
      // are going to ignore it (due to expectArrayBase).

      // Consume expectDef so that the children don't become DEFs.
      // We will create a mem ref expression at this node to
      // represent the DEF.
      removeInheritedFlag(rhsFlags, expectDef);

      // Consume flags.
      removeInheritedFlag(rhsFlags, expectRhsComputesLValue);
      removeInheritedFlag(rhsFlags, expectRhsDoesntComputeLValue);
      removeInheritedFlag(rhsFlags, expectRefRhs);

      addInheritedFlag(rhsFlags, partial);
      addInheritedFlag(rhsFlags, expectArrowDotRHS);
      //      addInheritedFlag(rhsFlags, expectDotLHS);

      // Recurse on rhs.
      list<OA::OA_ptr<OA::MemRefExpr> > rhsMemRefExprs;
      rhsMemRefExprs = findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags,
						    synthesizedFlags);

      // Notice that we
      // expect only one memory reference expression on the
      // rhs.  On the lhs, we allow expressions such
      // as ( a < b : c ? d ) which could yield multiple
      // mem ref expressions.  This does not make sense
      // on the right-hand side.
      ROSE_ASSERT(rhsMemRefExprs.size() == 1);

      bool rhsIsANamed = false;
      SgFunctionDeclaration *functionDeclaration = NULL;
      OA::OA_ptr<OA::MemRefExpr> rhsMre = *(rhsMemRefExprs.begin());
      ROSE_ASSERT(!rhsMre.ptrEqual(0));

      if ( rhsMre->isaNamed() ) {
	rhsIsANamed = true;
	OA::OA_ptr<OA::NamedRef> namedRef = rhsMre.convert<OA::NamedRef>();
	ROSE_ASSERT(!namedRef.ptrEqual(0));
	
	OA::SymHandle symHandle = namedRef->getSymHandle();
	SgNode *node = mIR->getNodePtr(symHandle);
	ROSE_ASSERT(node != NULL);
	
	functionDeclaration = isSgFunctionDeclaration(node);
      }

      // Represent the field access at this node, rather than
      // at the rhs.  We create a MemRefExpr representing
      // that field access for each potential lhs MemRefExpr,
      // so first determine that set of MemRefExprs.
      SgExpression *lhs = binaryOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      unsigned lhsFlags = flags;

      // Generally, we do not create an MRE for a because of a.b.
      // Instead, we create one for a.b, which we consider a 
      // "long name" for the expression.  Further for both a.foo
      // and a->foo, where foo is a method and I have intentionally
      // not written a.foo() or a->foo() which would represent the
      // return value from foo, we need not create an MRE for a or *a
      // so long as the compiler can statically determine the method
      // to be invoked.  In such cases (i.e., where foo is not virtual
      // or a is not a pointer or reference), the method is a property
      // of the class not directly of 'a'.  Nevertheless, we _do_
      // need to return an access of a since:
      // 1) it is an implicit parameter (as this) of a method and
      // 2) analysis should see it as a USE such that it doesn't think
      //    a is dead.
      // Therefore, if this is a dot expression that does not reference
      // a method, we do not create an MRE for the lhs.
      bool nodeIsArrowExp = mIR->isArrowExp(binaryOp);

      if ( !nodeIsArrowExp && ( functionDeclaration == NULL ) )
	addInheritedFlag(lhsFlags, expectDotLHS);
      
#if 0
      // If the lhs is a reference expression, don't apply the reference
      // conversion rules.  isArrowExp returns true if the lhs is
      // a reference although it is a dot expression.
      if ( isSgDotExp(binaryOp) && isSgReferenceType(lhs->get_type()) )
	addInheritedFlag(lhsFlags, expectDontApplyConversion);
#endif

#if 0
      // Even if we create an MRE for the lhs, let's associate with this
      // node.  When we recurse set expectDotLHS to true so that
      // we won't create an MRE at the child.  This is a bit of an
      // abuse since it may be an arrow expression; I'm using this
      // flag because I know it will prevent the storage of an MRE
      // at the child.  It isn't a great idea to associate the
      // MRE with the child:  if this a dot expression, we need to
      // take the addressOf the lhs.  However, doing so forces
      // isMemRefExpr to false and the MRE will not be associated with
      // the lhs.  In general, we associate the MRE for an addressOf MRE
      // with the node causing the addressOf (e.g., the addressOfOp).
      // For consistency, we do the same here.
      if ( functionDeclaration != NULL )
	addInheritedFlag(lhsFlags, expectDotLHS);
#else
      // But the above doesn't work because then getCallMemRefExprs will 
      // return not only the MRE for the call, but also the MRE for
      // this implicit arg.  Just associate the implicit actual with its
      // (child) node.  I will add a new flag, takeAddrOfAndStoreMRE.
#endif

      // Consume expectArrayBase and expectArrowDotRHS.  
      removeInheritedFlag(lhsFlags, expectArrayBase);
      removeInheritedFlag(lhsFlags, expectArrowDotRHS);

      // Consume flags.
      removeInheritedFlag(lhsFlags, expectRhsComputesLValue);
      removeInheritedFlag(lhsFlags, expectRhsDoesntComputeLValue);
      removeInheritedFlag(lhsFlags, expectRefRhs);

      // The addressOf flag only applies to the top-most
      // memory reference.  Therefore, remove it from the
      // lhs memory reference expression attribute.
      // If the address of an expression a->b or a.b is taken,
      // it applies to the expression as a whole (which is
      // represented by the SgArrowExp/SgDotExp).  
      // Therefore, remove from the lhs.  
      removeInheritedFlag(lhsFlags, expectAddrOf);

      // However, if this is a dot expression representing a method
      // invocation, we need to represent the implicit actual arg
      // corresponding to the 'this' implicit formal.  This is 
      // the lhs and is accomplished by visiting the lhs.  If this
      // is a dot expression, the lhs is an object and we need to
      // take its address (to bind it to the pointer this).  
      // If the lhs is a reference, we will convert it to a pointer
      // and the conversion will dereference it.  Therefore, we
      // still need to take the address to get rid of the Deref.
      // so we needn't take the address.
      // Pass takeAddrOfAndStoreMRE so that we don't ignore the
      // MRE as we usually do with expectAddrOf.
      if ( !nodeIsArrowExp && ( functionDeclaration != NULL ) )
	addInheritedFlag(lhsFlags, takeAddrOfAndStoreMRE);
	
      // Don't propagate DEF down to the lhs child
      // If this arrow/dot expression is used on the lhs,
      // it is the memRefExpr created at the SgArrowExp/SgDotExp
      // that will reflect this DEF.
      removeInheritedFlag(lhsFlags, expectDef);

      // Recurse on lhs.
      list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;

      // We don't care if the lhs computes an lvalue, since the
      // result of this field access expression is the rhs.
      unsigned tmpSynthesizedFlags;
      lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags,
						    tmpSynthesizedFlags);

#if 0
      // If this a method invocation, the lhs MREs represent the
      // implicit 'this' actual.  Store them at this node.
      // A second option is to pass them up and store them at
      // the SgFunctionCallExp along with the explicit actuals.
      // Conceptually, this would be more clear, but would complicate
      // this code.  We can't set isMemRefExpr to false which is
      // what we generally do when we've created MRE that we wish
      // to pass up to the parent without storing at the current node.
      // This would fail because we _do_ create MREs that we wish to
      // store here.  We would need to create another flag and array
      // of MREs.  Not difficult, but cumbersome ...
      if ( functionDeclaration != NULL ) {
	
	for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = lhsMemRefExprs.begin();
	    it != lhsMemRefExprs.end(); ++it) {
	  
	  OA::OA_ptr<OA::MemRefExpr> lhsMemRefExp = *it;

	  curMemRefExprs.push_back(lhsMemRefExp);

	  isMemRefExpr = true;

	}

      }
#else
      // See above comment.  I decided to store them at the child node.
#endif

      // For each MemRefExpr on the lhs, create a new MemRefExpr
      // to represent the field access.

      // Returns true if this is an arrow exp or a dot expression
      // of either a dereference or a reference (which will be
      // converted to a pointer).
      bool isArrowExp = mIR->isArrowExp(binaryOp);

      bool isReferenceExp = isSgReferenceType(binaryOp->get_lhs_operand_i()->get_type());

      // Is the address taken in this memory reference?
      addressTaken = ( expectAddrOfSet | takeAddrAndStoreMRESet );
      
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
	
	// If RHS MRE is a NamedRef with a SgFunctionDeclaration SymHandle
	//    If this is an arrow expression and the SgFunctionDeclaration
	//                                      is virtual
	//        Create a FieldAccess
	//    Else
	//        Create/copy the NamedRef here, it unambiguously
	//               determines the method invoked
	// Else
	//    // Not a method call, but a member field access
	//    Create a partially accurate Deref for arrow or
	//           a partially accureate MRE for dot.

	// NB:  We no longer check that the function is virtual.  
	//      Even if it is not virtual, there may be some ambiguity
	//      in which (class') method is invoked.  For example:
	//      B *b;
	//      if ( cond )
	//         b = new B;
	//      else
	//         b = new C;
	//      b->foo();
	//      where both B and C implement foo.

	// One more time.  Yes we do.  What was I thinking!?

	if ( ( rhsIsANamed == true ) && ( functionDeclaration != NULL ) ) {

	  if ( ( isArrowExp || isReferenceExp ) 
#if 1
	       && ( mIR->isVirtual(functionDeclaration) ) 
#endif
	       ) {

	    // Create a FieldAccess
	    bool fieldAccessFullAccuracy = true;
	    OA::MemRefExpr::MemRefType fieldAccessMemRefType = 
	      memRefType;
	    string mangledMethodName = 
	      functionDeclaration->get_mangled_name().str();
	  
	    OA::OA_ptr<OA::MemRefExpr> baseMre = lhsMemRefExp;
	    if ( isArrowExp ) {

	      baseMre = mIR->dereferenceMre(lhsMemRefExp);
	      if ( !lhsMemRefExp->hasFullAccuracy() )
		baseMre->setAccuracy(false);

	    }

#ifdef VTABLE_OPT
	    // If we are using the vtable optimization, we don't
	    // return (*a).method for a method invocation, but
	    // (*(*a).FieldHandle(OA_VTABLE_STR)).method.

	    OA::OA_ptr<OA::FieldAccess> fieldAccess;
	    fieldAccess = new OA::FieldAccess(false, 
					      true,
					      OA::MemRefExpr::USE;
					      baseMRE,
					      OA_VTABLE_STR);
	    if ( !baseMre->hasFullAccuracy() )
	      fieldAccess->setAccuracy(false);

	    baseMre = new OA::Deref(false,
				    true,
				    OA::MemRefExpr::USE;
				    fieldAccess,
				    1);
	    if ( !fieldAccess->hasFullAccuracy() )
	      baseMre->setAccuracy(false);
#endif

	    // HERE
	    arrowOrDotMemRefExpr = 
	      new OA::FieldAccess(addressTaken, 
				  fieldAccessFullAccuracy,
				  fieldAccessMemRefType,
				  baseMre,
				  mangledMethodName);

	    if ( !baseMre->hasFullAccuracy() )
	      arrowOrDotMemRefExpr->setAccuracy(false);

	  } else {

	    // Create/copy the NamedRef here, it unambiguously
	    // determines the method invoked.

	    arrowOrDotMemRefExpr = 
	      copyBaseMemRefExpr(rhsMre);
	    
	    // Notice that we do not set/change the accuracy.
	    // Since it unambiguously represents a (non-virtual) method
	    // invocation, it should not have partial accuracy.
	    // However, we assume it was set appropriately when
	    // it was created.
	    arrowOrDotMemRefExpr->setAddressTaken(addressTaken);
	    arrowOrDotMemRefExpr->setMemRefType(memRefType);
	    
	  }

	} else {

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
	    
	    arrowOrDotMemRefExpr = new OA::Deref(addressTaken,
						 fullAccuracy,
						 memRefType,
						 lhsMemRefExp,
						 numDerefs);
	    if ( !lhsMemRefExp->hasFullAccuracy() )
	      arrowOrDotMemRefExpr->setAccuracy(false);	    

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
	  
	  arrowOrDotMemRefExpr->setAccuracy(fullAccuracy);
	  arrowOrDotMemRefExpr->setAddressTaken(addressTaken);
	  arrowOrDotMemRefExpr->setMemRefType(memRefType);

	} 

	// Normalize the source program to convert references to pointers.  
	std::list<OA::OA_ptr<OA::MemRefExpr> > convertedMemRefs;
	if ( dontApplyConversion ) {
	  convertedMemRefs.push_back(arrowOrDotMemRefExpr);
	} else {
	  applyReferenceConversionRules(arrowOrDotMemRefExpr,
					astNode,
					appearsOnRhsOfRefInitialization,
					hasRhsThatComputesLValue,
					hasRhsThatDoesntComputeLValue,
					initializedRef,
					convertedMemRefs);
	}

	for (std::list<OA::OA_ptr<OA::MemRefExpr> >::iterator memRefIt = 
	       convertedMemRefs.begin();
	     memRefIt != convertedMemRefs.end(); ++memRefIt) {

	  OA::OA_ptr<OA::MemRefExpr> mre = *memRefIt;
	  curMemRefExprs.push_back(mre);

	}


	isMemRefExpr = true;

	hasOneOrMoreChildMemRefExps = true;


      } // end iteration over lhs MemRefExprs

      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);

      break;
    }

  case V_SgAddOp:
  case V_SgSubtractOp:
    {
      bool isArrayOperation = false;
      bool takeAddress = ( expectAddrOfSet | takeAddrAndStoreMRESet );

      SgAddOp *addOp = isSgAddOp(astNode);
      if ( addOp != NULL ) {

	// If the lhs is a SgPntrArrRefExp, then consider
	// this an address of operation.  e.g., I have 
	// seen &(sta[3][4]) translated to (SgPntrArrRefExp + 4).

	SgExpression *lhs = addOp->get_lhs_operand();
	ROSE_ASSERT(lhs != NULL);

	isArrayOperation = isSgPntrArrRefExp(lhs);
	
	if ( isArrayOperation ) {
	
	  takeAddress = true;

	} else {

	  // If the lhs is a SgVarRefExp with pointer type and
	  // the rhs has integral type, treat this as an 
	  // array access.  i.e., &sta[3] == (sta + 3).
	
	  SgVarRefExp *varRefExp = isSgVarRefExp(lhs);
	  if ( varRefExp ) {
	    if ( isSgPointerType(varRefExp->get_type()) ) {

	      SgExpression *rhs = addOp->get_rhs_operand();
	      if ( isSgTypeInt(rhs->get_type()) ) {
		isArrayOperation = true;
		takeAddress = true;
	      }

	    }
	  }

	}

	if ( isArrayOperation ) {
	  

	  
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
	  addInheritedFlag(lhsFlags, expectArrayBase);
	  
	  // Consume expectDef so that the children don't become DEFs.
	  // We will create a mem ref expression at this node to
	  // represent the DEF.
	  removeInheritedFlag(lhsFlags, expectDef);

	  // Consume flags.
	  removeInheritedFlag(lhsFlags, expectRhsComputesLValue);
	  removeInheritedFlag(lhsFlags, expectRhsDoesntComputeLValue);
	  removeInheritedFlag(lhsFlags, expectRefRhs);
	  removeInheritedFlag(lhsFlags, expectDotLHS);

	  // Recurse on the lhs.
	  list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;
	  lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags,
							synthesizedFlags);

	  // We don't care if the children compute an lvalue, since the
	  // result of this expression is generated by this node.
	  removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
	  removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

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

	  findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags,
				       synthesizedFlags);

	  // We don't care if the children compute an lvalue, since the
	  // result of this expression is generated by this node.
	  removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
	  removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);
	  
	  // Pointer arithmetic does not compute an lvalue.
	  addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

	  // Add the implicit addrOf operator to this node.
	  addressTaken = takeAddress;
	  
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

	    arrMemRefExpr->setAccuracy(fullAccuracy);
	    arrMemRefExpr->setAddressTaken(addressTaken);
	    arrMemRefExpr->setMemRefType(memRefType);

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

	  // Pointer arithmetic does not compute an lvalue.
	  addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

	}

	// Whether or not this is pointer arithmetic, we
	// need to visit the children.  Notice that we do 
	// not reach here when treating a SgAddOp as an
	// array access.  In that case, we explicitly visit the
	// children.
	handleDefaultCase(astNode, memRefs, flags, synthesizedFlags);

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
      // This is true only if the lhs is actually an array type
      // and not a pointer.  Consider:
      // int a[20];
      // int *b = a;
      // Michelle says:
      // a[2] just accesses the third location in the a location.  b[2]  
      // accesses the b location and the dereferences it to find the a  
      // location and then the third location in the a location.
      // So, ignore the lhs if it is an array type, do not ignore it
      // if it is a pointer.  Note, we will have to create a Deref
      // below if it is a pointer.
      bool pointerMasqueradingAsArray = false;

      if ( isSgPointerType(lhs->get_type()) )
	pointerMasqueradingAsArray = true;

      if ( !pointerMasqueradingAsArray )
	addInheritedFlag(lhsFlags, expectArrayBase);

      // Consume expectDef so that the children don't become DEFs.
      // We will create a mem ref expression at this node to
      // represent the DEF.
      removeInheritedFlag(lhsFlags, expectDef);

      // Consume flags.
      removeInheritedFlag(lhsFlags, expectRhsComputesLValue);
      removeInheritedFlag(lhsFlags, expectRhsDoesntComputeLValue);
      removeInheritedFlag(lhsFlags, expectRefRhs);
      removeInheritedFlag(lhsFlags, expectDotLHS);

      // Recurse on the lhs.
      list<OA::OA_ptr<OA::MemRefExpr> > lhsMemRefExprs;
      lhsMemRefExprs = findAllMemRefsAndMemRefExprs(lhs, memRefs, lhsFlags,
						    synthesizedFlags);

      // We don't care if the children compute an lvalue, since the
      // result of this array reference expression is generated by this node.
      removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

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

      findAllMemRefsAndMemRefExprs(rhs, memRefs, rhsFlags,
				   synthesizedFlags);

      // We don't care if the children compute an lvalue, since the
      // result of this array reference expression is generated by this node.
      removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      // This computes an lvalue.
      addSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);

      // Is the address taken in this memory reference?
      addressTaken = ( expectAddrOfSet | takeAddrAndStoreMRESet );
      
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
	
	OA::OA_ptr<OA::MemRefExpr> baseAccess = lhsMemRefExp;
	if ( pointerMasqueradingAsArray ) {

	  // As stated above, if this is a pointer being accessed like
	  // an array, we need to dereference the pointer and then
	  // perform the inaccurate "array access".
	  arrMemRefExpr = mIR->dereferenceMre(baseAccess);

	} else {
	  
	  arrMemRefExpr = 
	    copyBaseMemRefExpr(baseAccess);
	}

	ROSE_ASSERT(!arrMemRefExpr.ptrEqual(0));

	arrMemRefExpr->setAccuracy(fullAccuracy);
	arrMemRefExpr->setAddressTaken(addressTaken);
	arrMemRefExpr->setMemRefType(memRefType);

	curMemRefExprs.push_back(arrMemRefExpr);
	
	isMemRefExpr = true;

	hasOneOrMoreChildMemRefExps = true;
	
      } // end iteration over lhs' memory references
      
      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);

      break;

    }

  case V_SgCastExp:
    {
      SgCastExp *castExp = isSgCastExp(astNode);
      ROSE_ASSERT(castExp != NULL);

      // Return the unnamed memory reference from a malloc's cast.
      SgNode *node = castExp->get_operand();
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);

      // This cast is attached to a malloc.
      if ( ( functionCallExp != NULL ) && ( mIR->isMalloc(functionCallExp) ) ) {

	// Represent malloc'ed memory by an unnamed memory reference.

	memRefType = OA::MemRefExpr::USE;

	// Create an OpenAnalysis handle for this node.
	stmtHandle = mIR->getNodeNumber(castExp);

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

	// This isn't the cast to malloc case, so visit the children.
	handleDefaultCase(astNode, memRefs, flags, synthesizedFlags);

      }

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

      bool returnsReference = false;

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

	findAllMemRefsAndMemRefExprs(actualArg, memRefs, 0,
				     synthesizedFlags);
      }

      // We don't care if the children compute an lvalue, since the
      // result of this expression is generated by this node.
      removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);
      
      // Create a MemRefExpr for this function call if required.
      if ( mIR->isMalloc(functionCallExp) ) {

#if 1	
	// FIXME:  Eventually, consider making cast MRE, since we
	//         care about the type for malloc.
	// Done!  Moved this to SgCastExp case.
#else

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
#endif
      } else {

	SgType *type = functionCallExp->get_type();
	ROSE_ASSERT(type != NULL);

	bool returnsAddress = false;
#if 1
	/* 
	 * We no longer return an unknown MRE from a function
	 * which returns a pointer or a reference.  Instead
	 * we return something that represents the function/method.
	 * This allows us to create pointer assign pairs of
	 * the form, lhs = func for a function func() returning
	 * a pointer or reference.  For any return statements
	 * in func, we create a companion pointer assign pair
	 * of the form func = ret, where ret is the return expression
	 * of a return statement.  This allows FIAlias (or some
	 * other alias analysis) to create the union *lhs,*ret,
	 * via the intermediary *func.
	 
	 * The following table shows how we represent the function/method:
	 
	 proc type   ret type   lhs = proc
	 ---------   --------   ----------
	 func        address    lhs = func
	 non-addr       --
	 (*funcPtr)  address    lhs = *funcPtr
	 non-addr       --
	 method      address    lhs = method
	 non-addr       --
	 virtual     address    lhs = *(obj->"fooA")
	 non-addr       --
	 
	 Notice that the proc MREs are the same as those
	 returned by getCallMemRefExpr() and as created
	 by recursively visiting the function (from get_function()),
	 though in the latter case we need to dereference
	 the obj->"fooA" case MRE.

	*/

	if ( isSgReferenceType(type) || isSgPointerType(type) ) {
	  returnsAddress = true;
	}
	
#else /* 1 */
	// Create an unknown reference for this function if it
	// has a pointer or reference return type.
	// Also, if it is a method call, we need to visit the
	// arrow or dot expression.  This will be held in the
	// function child.

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

	  returnsReference = true;

	  if ( isSgExpression(parent) ) {

	    // Get the memory reference type (DEF, USE, etc.) for 
	    // this function call.
	    memRefType = flagsToMemRefType(flags);

	    // The address of the memory location is not taken
	    // during a function call.
	    addressTaken = false;
	    
	    // Create an unknown memory reference expression.
	    OA::OA_ptr<OA::MemRefExpr> memRefExp;

	    // The function call does _not_ accurately represent the memory 
	    // expression, as this would require the precise calling context.
	    fullAccuracy = false;
	    
	    // Create an OpenAnalysis handle for this node.
	    stmtHandle = mIR->getNodeNumber(functionCallExp);
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

#endif /* 1 */

	SgExpression *expression = functionCallExp->get_function();
	ROSE_ASSERT(expression != NULL);

	if ( returnsAddress || !isSgFunctionRefExp(expression) ) {
	  // Zero out the rhs flags.
	  unsigned childFlags = 0;
	  
	  // Generally we would not create an MRE for a function
	  // since its execution (unlike that of a method) does
	  // not represent use of program state.  However, if
	  // it is a function which returns an address (pointer or
	  // reference), we use the MRE generated from the SgFunctionRefExp
	  // to represent the return value.
	  if ( isSgFunctionRefExp(expression) )
	    addInheritedFlag(childFlags, expectAddrReturningFunc);

	  list<OA::OA_ptr<OA::MemRefExpr> > funcMemRefExprs;
	  funcMemRefExprs = findAllMemRefsAndMemRefExprs(expression, memRefs, 
							 childFlags,
							 synthesizedFlags);

	  if ( returnsAddress ) {

	    // If the MRE representing the function is a FieldAccess,
	    // it represents a virtual method call.  In this case, the 
	    // method field acts as a function pointer.  Therefore, 
	    // dereference the FieldAccess/function pointer:
	    // this pointer dereference represents the method invoked.

	    for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = funcMemRefExprs.begin();
		it != funcMemRefExprs.end(); ++it) {
	  
	      OA::OA_ptr<OA::MemRefExpr> mre = *it;
	      ROSE_ASSERT(!mre.ptrEqual(0));

	      if ( mIR->isFieldAccess(mre) ) {
		mre = mIR->dereferenceMre(mre);
	      }
	      
	      isMemRefExpr = true;

	      curMemRefExprs.push_back(mre);

	    }

	  }

	  if ( !returnsAddress ) {
	    // We don't care if the children compute an lvalue, since the
	    // result of this expression is generated by this node.
	    removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
	    removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);
	  }
#if 0
	  // Create a pseudo-MemRefExpr for the dispatching object,
	  // i.e., b in b->foo() or b.foo().  This is needed, for
	  // example, by FIAlias when we do implicit param binding.
	  // We have to handle two cases:  
	  //     b->foo()   -> this = b -> *this = *b  (in FIAlias)
	  //     b.foo()    -> this = &b -> *this = b  (in FIAlias)
	  // i.e., if expression is an arrow expression (including (*b).foo())
	  // we should simply create a MemRefExpr for the lhs of the
	  // arrow expression.  If expression is a dot expression,
	  // we should create a MemRefExpr which is the lhs of
	  // the dot expression with its address taken.
	  // This is an implicit/pseudo-MemRefExpr that we only want
	  // visible in results returned from getCallsiteParams,
	  // which will create a MemRefHandle to represent the
	  // dispatching object.  Therefore, we do not want to
	  // return this MemRefHandle via getAllMemRefs, so do
	  // not put it in the memRefs list.  However, do put
	  // the corresponding MemRefExpr in sMemref2mreSetMap so that 
	  // getMemRefExprIterator(memRefHandle) will find it.
	  // Notice the user will first have to get the relevant
	  // memRefHandle, which they may only do through the
	  // getCallsiteParams method.

	  OA::MemRefHandle memRefHandle =
	    mIR->getDispatchingObjectMemRefHandle(functionCallExp);
	    
	  bool isArrowExp = false;
	  SgExpression *lhs = NULL;

	  if ( isSgArrowExp(expression) ) {

	    isArrowExp = true;

	  } else {
	    
	    SgDotExp *dotExp = isSgDotExp(expression);
	    ROSE_ASSERT(dotExp != NULL);

	    lhs = dotExp->get_lhs_operand();
	    ROSE_ASSERT(lhs != NULL);

	    SgPointerDerefExp *pointerDerfeExp =
	      isSgPointerDerefExp(lhs);

	    if ( lhs != NULL ) {
	      
	      // This is (*b).foo() == b->foo();
	      isArrowExp = true;
	      lhs = pointerDerefExp->get_operand_i();

	    }

	  }

#endif

	}
      }

      // This node only computes an lvalue if the function returns
      // a reference.
      if ( returnsReference ) 
	addSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      else
	addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

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
      findAllMemRefsAndMemRefExprs(conditionalChild, memRefs, 0,
				   synthesizedFlags);

      // We don't care if the conditional computes an lvalue, since the
      // result of this conditional expression is generated by the true
      // and false expressions.
      removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      SgExpression *trueExpression = conditionalExp->get_true_exp();
      ROSE_ASSERT(trueExpression != NULL);
      
      // Propagate this node's flags to its child.
      unsigned trueFlags = flags;
      
      // Don't propagate DEF down to the child.  If this
      // SgConditionalExp is used on the lhs, we will represent
      // the DEF access by the MemRefExpr we create for this
      // node, not by that of it's children.
      removeInheritedFlag(trueFlags, expectDef);

      // Consume flags.
      removeInheritedFlag(trueFlags, expectRhsComputesLValue);
      removeInheritedFlag(trueFlags, expectRhsDoesntComputeLValue);
      removeInheritedFlag(trueFlags, expectRefRhs);
      removeInheritedFlag(trueFlags, expectDotLHS);

      // Do not consume expectArrayBase or expectArrowDotRHS
      // from either of the true or false expressions.
      // i.e., we might not store MemRefHandles/MemRefExprs
      // at their respective AST nodes, though we will pass
      // them back to the caller via findAllMemRefsAndMemRefExprs
      // return value.

      // Recurse on true expression.
      list<OA::OA_ptr<OA::MemRefExpr> > trueMemRefExprs;
      trueMemRefExprs = 
	findAllMemRefsAndMemRefExprs(trueExpression, memRefs, trueFlags,
				     synthesizedFlags);

      SgExpression *falseExpression = conditionalExp->get_false_exp();
      ROSE_ASSERT(falseExpression != NULL);
      
      // Propagate this node's flags to its child.
      unsigned falseFlags = flags;
      
      // Don't propagate DEF down to the child.
      removeInheritedFlag(falseFlags, expectDef);

      // Consume flags.
      removeInheritedFlag(falseFlags, expectRhsComputesLValue);
      removeInheritedFlag(falseFlags, expectRhsDoesntComputeLValue);
      removeInheritedFlag(falseFlags, expectRefRhs);
      removeInheritedFlag(falseFlags, expectDotLHS);

      // Recurse on false expression.
      list<OA::OA_ptr<OA::MemRefExpr> > falseMemRefExprs;
      falseMemRefExprs = 
	findAllMemRefsAndMemRefExprs(falseExpression, memRefs, falseFlags,
				     synthesizedFlags);

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
	
	// Add USE to the rhs child's flags.
	unsigned rhsFlags = flags;
	
	// Consume expectArrayBase and 
	// expectArrowDotRHS.  Memory
	// reference expressions within an assignment need not
	// account for them.
	removeInheritedFlag(rhsFlags, expectArrayBase);
	removeInheritedFlag(rhsFlags, expectArrowDotRHS);
	removeInheritedFlag(rhsFlags, expectDotLHS);
	
	// Consume flags.
	removeInheritedFlag(rhsFlags, expectRhsComputesLValue);
	removeInheritedFlag(rhsFlags, expectRhsDoesntComputeLValue);
	removeInheritedFlag(rhsFlags, expectRefRhs);

	addInheritedFlag(rhsFlags, expectUse);
	
	SgType *lhsType = initName->get_type();
	ROSE_ASSERT(lhsType != NULL);

	if ( isSgReferenceType( mIR->getBaseType(lhsType) ) )
	  addInheritedFlag(rhsFlags, expectRefRhs);
	// HERE--with initname
#if 1
	// Recurse on rhs.
	findAllMemRefsAndMemRefExprs(initializer, memRefs, rhsFlags,
				     synthesizedFlags);
#endif
	hasRhsThatComputesLValue = 
	  synthesizedFlags & synthesizedComputesLValue;
	hasRhsThatDoesntComputeLValue = 
	  synthesizedFlags & synthesizedDoesntComputeLValue;
	
	// We don't care if the rhs computes an lvalue, since the
	// result of this expression is generated by the lhs.
	removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
	removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

	// This is a definition, not just a declaration.
	OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::DEF;

	// The address of the memory location is not taken
	// during a declaration.
	addressTaken = false;

	// The declaration accurately represents the memory expression.
	fullAccuracy = true;

	// Create a named memory reference expression.
	OA::OA_ptr<OA::MemRefExpr> memRefExp;
	memRefExp = mIR->createMRE(initName, addressTaken,
				   fullAccuracy, memRefType,
				   ( flags & expectArrowDotRHS ) );
	ROSE_ASSERT(!memRefExp.ptrEqual(0));

	isNamed = memRefExp->isaNamed();


	isMemRefExpr = true;

	// Normalize the source program to convert references to pointers.  
	std::list<OA::OA_ptr<OA::MemRefExpr> > convertedMemRefs;
	if ( dontApplyConversion ) {
	  convertedMemRefs.push_back(memRefExp);
	} else {
	  applyReferenceConversionRules(memRefExp,
					astNode,
					appearsOnRhsOfRefInitialization,
					hasRhsThatComputesLValue,
					hasRhsThatDoesntComputeLValue,
					initializedRef = true,
					convertedMemRefs);
	}

	for (std::list<OA::OA_ptr<OA::MemRefExpr> >::iterator memRefIt = 
	       convertedMemRefs.begin();
	     memRefIt != convertedMemRefs.end(); ++memRefIt) {

	  OA::OA_ptr<OA::MemRefExpr> mre = *memRefIt;
	  curMemRefExprs.push_back(mre);

	}

      }

      // An initialized name is an lvalue
      addSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);

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
      addressTaken = ( expectAddrOfSet | takeAddrAndStoreMRESet );
      
      // Access to a named variable is a fully accurate
      // representation of that access, unless it occurs
      // in the context of an array reference expression,
      // arrow/dot expression, etc.  In those case, the
      // inaccuracy of the representation will be handled
      // at those nodes.
      fullAccuracy = true;
	  
      // Create a named memory reference expression.
      OA::OA_ptr<OA::MemRefExpr> memRefExp;
      memRefExp = mIR->createMRE(varRefExp, addressTaken,
				 fullAccuracy, memRefType,
				 ( flags & expectArrowDotRHS ) );
      ROSE_ASSERT(!memRefExp.ptrEqual(0));

      isMemRefExpr = true;

      // Normalize the source program to convert references to pointers.  
      std::list<OA::OA_ptr<OA::MemRefExpr> > convertedMemRefs;
      if ( dontApplyConversion ) {
	convertedMemRefs.push_back(memRefExp);
      } else {
	applyReferenceConversionRules(memRefExp,
				      astNode,
				      appearsOnRhsOfRefInitialization,
				      hasRhsThatComputesLValue,
				      hasRhsThatDoesntComputeLValue,
				      initializedRef,
				      convertedMemRefs);
      }

      for (std::list<OA::OA_ptr<OA::MemRefExpr> >::iterator memRefIt = 
	     convertedMemRefs.begin();
	   memRefIt != convertedMemRefs.end(); ++memRefIt) {
	
	OA::OA_ptr<OA::MemRefExpr> mre = *memRefIt;
	curMemRefExprs.push_back(mre);
	
      }

      // A var ref exp is an lvalue
      addSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);

      break;
    }

  case V_SgThisExp:
    {
      SgThisExp *thisExp = isSgThisExp(astNode);
      ROSE_ASSERT(thisExp != NULL);

      // Get the memory reference type (DEF, USE, etc.) for 
      // this node.
      memRefType = flagsToMemRefType(flags);
      
      // Is the address taken in this memory reference?
      addressTaken = ( expectAddrOfSet | takeAddrAndStoreMRESet );
      
      // Access to a named variable is a fully accurate
      // representation of that access, unless it occurs
      // in the context of an array reference expression,
      // arrow/dot expression, etc.  In those case, the
      // inaccuracy of the representation will be handled
      // at those nodes.
      fullAccuracy = true;
	  
      // Create an OpenAnalysis handle for this node.
      //      symHandle = mIR->getNodeNumber(thisExp);
      // NB:  Please see getFormalParamIterator and getFormalForActual.
      //      These methods do not have access to a SgThisExp, so
      //      they instead return a SgClassSymbol to represent it.
      //      So, we do the same here.
      symHandle = mIR->getThisExpSymHandle(thisExp);

      // Create a named memory reference expression.
      OA::OA_ptr<OA::MemRefExpr> memRefExp;
      memRefExp = new OA::NamedRef(addressTaken, fullAccuracy,
				   memRefType, symHandle);
      ROSE_ASSERT(!memRefExp.ptrEqual(0));

      // We don't have to convert 'this', though it is named.
      // It is a pointer, not a reference.

      isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      // this does not compute an lvalue.
      addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      break;
    }

  case V_SgFunctionRefExp:
    {
      // A FunctionRefExp can appear on the right-hand side of
      // an assignment to a function pointer.  We also need
      // to associate an MRE with a SgFunctionRefExp if it is
      // used to represent the return value of an address-returning
      // function.

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

      // However, we might also use this SgFunctionRefExp to
      // represent the return value from a SgFunctionCallExp.
      // In that case, we do not take the address.
      if ( addrReturningFunc )
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

      // We do not have to apply the conversion rules since this
      // is a function pointer, not a reference.

      // If we are creating this MRE to represent the return value
      // from an addressing-return function, hold the MRE at its
      // parent, not here.  We do this because some MRE representing
      // addressing-returning functions, namely virtual methods,
      // are derefer'ed at the parent and held there.
      if ( !addrReturningFunc )
	isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      // We can take the address of a function to assign it to 
      // a function pointer.  This computes an lvalue.
      addSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);

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

      // We do not have to apply the conversion rules: this
      // is not a reference.

      isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      // This does not compute an lvalue.
      addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      break;
    }

  case V_SgNewExp:
    {
      SgNewExp *newExp = isSgNewExp(astNode);
      ROSE_ASSERT(newExp != NULL);

      // Invocations of new are represented by unnamed memory references,
      // just as calls to malloc (above).

      // Notice that we also need to represent the access to the 
      // invoked constructor/method.  Do this at the SgConstructorInitializer
      // accessible from newExp as newExp->get_constructor_args();

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

      // This does not compute an lvalue.
      addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

      // The constructor initializer is treated
      // independently of whatever precedes it in the
      // AST.  Therefore, do not propagate the parent
      // node's flags to them, but rather zero out their flags.
      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      // Notice that we don't need to consume expectArrayBase, expectDef,
      // and expectArrowDotRHS since we have zeroed out the flags.
      
      unsigned int dummySynthesizedFlags;
      findAllMemRefsAndMemRefExprs(ctorInitializer, memRefs, 0,
				   dummySynthesizedFlags);

      break;
    }

  case V_SgConstructorInitializer:
    {
      // A SgConstructorInitializer appears as a child of a SgNewExp.
      // We need to extract the constructor invoked and treat it
      // analogously to a method invocation above.
      // We consider this a USE.

      SgConstructorInitializer *ctorInitializer =
	isSgConstructorInitializer(astNode);
      ROSE_ASSERT(ctorInitializer != NULL);

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
      SgFunctionDeclaration *functionDeclaration = 
	ctorInitializer->get_declaration();
      ROSE_ASSERT(functionDeclaration != NULL);

      // Create an OpenAnalysis handle for this node.
      symHandle = mIR->getNodeNumber(functionDeclaration);

      // Create a named memory reference expression.
      OA::OA_ptr<OA::MemRefExpr> memRefExp;
      memRefExp = new OA::NamedRef(addressTaken, fullAccuracy,
				   memRefType, symHandle);
      ROSE_ASSERT(!memRefExp.ptrEqual(0));

      // We do not have to apply the conversion rules: this
      // is not a reference.

      isMemRefExpr = true;

      curMemRefExprs.push_back(memRefExp);

      // This does not compute an lvalue.
      addSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

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
      removeInheritedFlag(childFlags, expectArrayBase);
      removeInheritedFlag(childFlags, expectArrowDotRHS);
      removeInheritedFlag(childFlags, expectDotLHS);

      // Consume expectDef so that the children don't become DEFs.
      // We will create a mem ref expression at this node to
      // represent the DEF.
      removeInheritedFlag(childFlags, expectDef);

      // Consume flags.
      removeInheritedFlag(childFlags, expectRhsComputesLValue);
      removeInheritedFlag(childFlags, expectRhsDoesntComputeLValue);
      removeInheritedFlag(childFlags, expectRefRhs);

      // If this dereference is on the rhs and we are going to 
      // treat it like a reference, we will end up creating an MRE
      // that takes the address of the Deref, i.e., it will
      // be the same MRE created at the operand. So, ignore that.
      if ( flags & expectRefRhs ) 
	addInheritedFlag(childFlags, ignore);

      list<OA::OA_ptr<OA::MemRefExpr> > operandMemRefExprs;
      operandMemRefExprs = 
	findAllMemRefsAndMemRefExprs(operand, memRefs, childFlags,
				     synthesizedFlags);

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
	addressTaken = ( expectAddrOfSet | takeAddrAndStoreMRESet );
      
	// A dereference fully represents a memory reference expression.
	fullAccuracy = true;
      
	// Get the memory reference type (DEF, USE, etc.) for 
	// this node.
	memRefType = flagsToMemRefType(flags);

	// We don't care if the operand computes an lvalue, since the
	// result of this expression is generated by this node.
	removeSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
	removeSynthesizedFlag(synthesizedFlags, synthesizedDoesntComputeLValue);

	for(list<OA::OA_ptr<OA::MemRefExpr> >::iterator it = operandMemRefExprs.begin();
	    it != operandMemRefExprs.end(); ++it) {
	  
	  OA::OA_ptr<OA::MemRefExpr> memRefExp = *it;
	  ROSE_ASSERT(!memRefExp.ptrEqual(0));

	  int numDerefs = 1;
	  OA::OA_ptr<OA::Deref> deref;
	  deref = new OA::Deref(addressTaken,
				fullAccuracy,
				memRefType,
				memRefExp,
				numDerefs);
	  ROSE_ASSERT(!deref.ptrEqual(0));

	  deref->setAccuracy(fullAccuracy);
	  deref->setAddressTaken(addressTaken);
	  deref->setMemRefType(memRefType);

	  if ( !memRefExp->hasFullAccuracy() )
	    deref->setAccuracy(false);

	  // Normalize the source program to convert references to pointers.  
	  // Though this is obviously not a reference (since we 
	  // are dereferencing it), we may need to take its address
	  // if it is on the rhs.
	  std::list<OA::OA_ptr<OA::MemRefExpr> > convertedMemRefs;
	  if ( dontApplyConversion ) {
	    convertedMemRefs.push_back(deref);
	  } else {
	    applyReferenceConversionRules(deref,
					  astNode,
					  appearsOnRhsOfRefInitialization,
					  hasRhsThatComputesLValue,
					  hasRhsThatDoesntComputeLValue,
					  initializedRef,
					  convertedMemRefs);
	  }

	  for (std::list<OA::OA_ptr<OA::MemRefExpr> >::iterator memRefIt = 
		 convertedMemRefs.begin();
	       memRefIt != convertedMemRefs.end(); ++memRefIt) {
	    
	    OA::OA_ptr<OA::MemRefExpr> mre = *memRefIt;
	    curMemRefExprs.push_back(mre);
	    
	  }

	  isMemRefExpr = true;

	  hasOneOrMoreChildMemRefExps = true;

	}

	// This computes an lvalue
	addSynthesizedFlag(synthesizedFlags, synthesizedComputesLValue);
      }

      ROSE_ASSERT(hasOneOrMoreChildMemRefExps == true);

      break;
    }

  default:
    {
      handleDefaultCase(astNode, memRefs, flags, synthesizedFlags);

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
       ( flags & expectDotLHS ) ||
       ( flags & expectAddrOf ) ||
       ( flags & ignore ) ||
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

      // mapping of MemRefHandle to MemRefExprs.
      SageIRInterface::sMemref2mreSetMap[memRefHandle].insert(mre);

      // reverse map of MemRefExpr to MemRefHandle (a 1 to many map).
      SageIRInterface::sMre2MemrefMap[mre] = memRefHandle;

    }

  }

  return curMemRefExprs;
}

OA::OA_ptr<OA::MemRefHandleIterator> 
SageIRInterface::getMemRefIterator(OA::IRHandle h)
{
  OA::OA_ptr<OA::MemRefHandleIterator> retval;
  retval = new SageIRMemRefIterator(h, this);
  return retval;
}

// Return the MemRefType of a MemRefExpr.
OA::MemRefExpr::MemRefType
SageIRInterface::getMemRefType(OA::OA_ptr<OA::MemRefExpr> mre)
{
  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;

  if ( mre->isUse() && mre->isDef() ) {
    if ( mre->isUseDef() ) 
      memRefType = OA::MemRefExpr::USEDEF;
    else 
      memRefType = OA::MemRefExpr::DEFUSE;
  } else if ( mre->isDef() ) {
    memRefType = OA::MemRefExpr::DEF;
  }

  return memRefType;

}
