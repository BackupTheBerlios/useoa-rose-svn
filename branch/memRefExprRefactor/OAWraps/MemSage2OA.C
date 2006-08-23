#include "MemSage2OA.h"
#include "SageOACallGraph.h"
#include "common.h"

using namespace std;

SageIRMemRefIterator::SageIRMemRefIterator(OA::StmtHandle h, 
                       SageIRInterface& ir)
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

/*! this method calls initMemRefMaps, which sets up 
    mMemRef2StmtMap, mStmt2MemRefSet, mCall2MRE,and mMemRef2mreSetMap
    in the SageIRInterface.
    Is only way to get MemRefHandle's therefore no queries should
    be logically made on MemRefHandle's before one of these
    iterators has been requested.
*/
void
SageIRMemRefIterator::create(OA::StmtHandle h)
{
  SgNode *node = mIR.getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  // if haven't already determined the set of memrefs for the program
  // then call initMemRefMaps
  if (mIR.mStmt2allMemRefsMap.empty() ) {
      mIR.initMemRefAndPtrAssignMaps();
  }

  // loop through MemRefHandle's for this statement and for now put them
  // into our own list
  std::set<OA::MemRefHandle>::iterator setIter;
  for (setIter=mIR.mStmt2allMemRefsMap[h].begin();
       setIter!=mIR.mStmt2allMemRefsMap[h].end(); setIter++)
    {
      mMemRefList.push_back(*setIter);
    }

}

/*!
   Create the parameter bindings for a call handle and
   place them in mCallToParamPtrPairs.
*/
void
SageIRInterface::createParamBindPtrAssignPairs(SgNode *node)
{
    verifyCallHandleNodeType(node);
    OA::CallHandle call = getCallHandle(node);
  
    bool isCallADotExp           = false;
    bool isCallAMethodInvocation = false;

    switch(node->variantT()) {
    case V_SgFunctionCallExp:
        {      
            SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
            ROSE_ASSERT(functionCallExp != NULL);
      
            isCallAMethodInvocation = isMethodCall(functionCallExp,  
                                                   isCallADotExp);
            break;
        }
    case V_SgConstructorInitializer:
        {
            isCallAMethodInvocation = true;

            SgNode *parent = node->get_parent();
            ROSE_ASSERT(parent != NULL);

            if ( isSgInitializedName(parent) ) {
                // If the parent of the constructor initialize is a var, then
                // we are in the case:
                // A a;
                // rather than:
                // A a = new A;
                // Therefore, in the implicit param binding with 'this', we
                // need to treat this as if it were a dot expression, so
                // that we take the address of a:  this = &a.
                isCallADotExp = true;
            }
            break;
        }
    case V_SgDeleteExp:
        {
            isCallAMethodInvocation = true; 
            break;
        }
    default:
        {
	    std::cerr << "Call must be a SgFunctionCallExp,"
                      << " a SgConstructorInitializer, or a SgDeleteExp."
                      << std::endl
                      << "Instead got a " << node->sage_class_name()
                      << std::endl;
            ROSE_ABORT();
            break;
        }
    }

    // Get the list of formal types.
    // NB:  This does _not_ have the receiver formal type folded in.
    SgTypePtrList &typePtrList = getFormalTypes(node);

    // Get the list of actual argument MREs.
    // NB:  This _does_ have the receiver actual argument folded in.
    OA::OA_ptr<OA::IRCallsiteParamIterator> actualsIter = 
        getCallsiteParams(call);
    actualsIter->reset();

    int paramNum = 0;

    // Handle the implicit this parameter outside of the loop.
    // Note that the implicit actual is included in actualsIter,
    // though the formal is not represented in typePtrList.

    // If this is a method, constructor, or destructor invocation,
    // we need to fold the receiver in as the 1st actual argument.
    // This folding is handled by getCallsiteParams; we need
    // only _not_ consume a formal type.
    if ( isCallAMethodInvocation ) {

        ROSE_ASSERT(actualsIter->isValid());
        
        OA::ExprHandle actualExpr = actualsIter->current(); 

        // I need a MemRefHandle.  I could cast actualExpr
        // to a SgNode and then ask for its topMemRefHandle.
        // Fine.  I'll do that.
        SgNode *actualNode = getNodePtr(actualExpr);

        OA::MemRefHandle actualMemRefHandle 
            = findTopMemRefHandle(actualNode);
      
        OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
            = getMemRefExprIterator(actualMemRefHandle);
      
        // for each mem-ref-expr associated with this memref
        for (actualMreIterPtr->reset(); actualMreIterPtr->isValid(); 
             (*actualMreIterPtr)++) {

            OA::OA_ptr<OA::MemRefExpr> curr = actualMreIterPtr->current();
            OA::OA_ptr<OA::MemRefExpr> actualMre = curr;

            if ( isCallADotExp == true ) {
                // We are returning a MemRefExpr representing the object b
                // upon which a method is invoked in the expression b.foo()
                // (i.e., a dot expression).  This pair is intended to
                // represent the implicit binding between the 'this' pointer
                // and this object.  Since 'this' is a pointer, we should
                // bind it to the address of b.

                actualMre = curr->clone();
                actualMre->setAddressTaken(true);
            }
	  
            makeParamPtrPair(call, paramNum, actualMre);
        }	
    
        // Consume the actual corresponding to the implicit receiver
        // and update the parameter number.
        (*actualsIter)++; 
        paramNum++;
    }

    // We model parameters passed to a vararg/... as a single formal
    // parameter.  Thus:
    // 
    // void ellipsis_intptrs(int x, ...) 
    // { 
    // } 
    //
    // has only two formals:  x (0th formal) and ... (1st formal).
    //
    // Therefore, the param pairs for:
    //
    // ellipsis_intptrs(3, &x, &y, &z);
    //
    // are (just for the pointers):
    // 
    // < 1, NamedRef( USE, SymHandle("x"), T, full) >
    // < 1, NamedRef( USE, SymHandle("y"), T, full) >
    // < 1, NamedRef( USE, SymHandle("z"), T, full) >
    //
    // In particular, notice that the param num is 1 for each of these.
    bool incrementParamNum = true;

    // Simultaneously iterate over both formals and actuals.
    // If the formal is a reference parameter (i.e., 
    // a pointer or a reference) store it in the iterator's list.
    SgTypePtrList::iterator formalIt = typePtrList.begin();
    for ( ; actualsIter->isValid(); (*actualsIter)++ ) { 
        bool treatAsPointerParam = false;

        OA::ExprHandle actualExpr = actualsIter->current(); 
        SgNode *actualNode = getNodePtr(actualExpr);
        ROSE_ASSERT(actualNode != NULL);

        SgType *type = NULL;
        // In the presence of varargs, we may have fewer
        // formals than actuals.
        if ( formalIt != typePtrList.end() ) {
            type = *formalIt;
        }

        // In the presence of varags, get the type from the actual.
        if ( ( type == NULL ) || ( isSgTypeEllipse(type) ) ) {
            SgExpression *actual = isSgExpression(actualNode);
            ROSE_ASSERT(actual != NULL);
            type = actual->get_type();
            // Do not increment the parameter number since
            // the rest of the args are varargs.
            incrementParamNum = false;
        }
        ROSE_ASSERT(type != NULL);
    
        if ( isSgReferenceType(type) || isSgPointerType(type) ) {
            treatAsPointerParam = true;
        }

        if ( treatAsPointerParam ) {

            // As above, take the long way to convert an ExprHandle to
            // a MemRefHandle.       
            OA::MemRefHandle actualMemRefHandle 
                = findTopMemRefHandle(actualNode);

            // Notice that actualMemRefHandle may be NULL/0.
            // For example, consider that printf's first formal is
            // a const char *.  There is no MemRefHandle here (is there?)

            OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
                = getMemRefExprIterator(actualMemRefHandle);
      
            // for each mem-ref-expr associated with this memref
            for (actualMreIterPtr->reset(); actualMreIterPtr->isValid(); 
                 (*actualMreIterPtr)++) {

                OA::OA_ptr<OA::MemRefExpr> actualMre = 
                    actualMreIterPtr->current();

                makeParamPtrPair(call, paramNum, actualMre);
            }
        }

        // If we haven't run out of formal types because of varargs,
        // go to the next one.
        if (formalIt != typePtrList.end()) {
            ++formalIt;
        }
 
        // Similarly, if we haven't hit any varargs, increment
        // the param num.
        if ( incrementParamNum ) {
            paramNum++;
        }
    } 
}

void SageIRInterface::initMemRefAndPtrAssignMaps()
{
  // iterate over all procedures
  bool excludeInputFiles = false;  // FIXME: should this be true?
  OA::OA_ptr<OA::IRProcIterator> procIter;
  procIter = new SageIRProcIterator(wholeProject, *this, excludeInputFiles);
  for (procIter->reset() ; procIter->isValid(); ++(*procIter)) {
      OA::ProcHandle currProc = procIter->current();

      // Iterate over the statements of this procedure
      OA::OA_ptr<OA::IRStmtIterator> stmtIterPtr = getStmtIterator(currProc);
      for ( ; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
          OA::StmtHandle stmt = stmtIterPtr->current();

          // find all of the CallHandles and MemRefHandles within this Stmt
          findAllMemRefsAndPtrAssigns( getSgNode(stmt), stmt );
      }
  }
}

/** \brief  Apply the reference conversion rules to the actual
 *          arguments of a method/function invocation.
 *  \param  node  An AST node representing a 
 *                method/function/constructor/destructor invocation.
 *
 *  Iterates over the actuals passed to the function or method.
 *  If any of the corresponding formals are references, then take
 *  the address of the actual, in accordance with our modeling
 *  references as pointers.
 */
void SageIRInterface::convertReferenceActuals(SgNode *node)
{
    verifyCallHandleNodeType(node);

    SgExprListExp* exprListExp = NULL;

    switch(node->variantT()) {
    case V_SgFunctionCallExp:
        {      
            SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
            ROSE_ASSERT(functionCallExp != NULL);
      
            exprListExp = functionCallExp->get_args();
            break;
        }
    case V_SgConstructorInitializer:
        {
            SgConstructorInitializer *ctorInitializer =
                isSgConstructorInitializer(node);
            ROSE_ASSERT(ctorInitializer != NULL);

            exprListExp = ctorInitializer->get_args();
            break;
        }
    case V_SgDeleteExp:
        {
            // A destructor does not have an argument list.
            break;
        }
    default:
        {
	    std::cerr << "Call must be a SgFunctionCallExp,"
                      << " a SgConstructorInitializer, or a SgDeleteExp."
                      << std::endl
                      << "Instead got a " << node->sage_class_name()
                      << std::endl;
            ROSE_ABORT();
            break;
        }
    }

    if ( exprListExp == NULL ) {
        return;
    }

    SgExpressionPtrList & actualArgs =  
        exprListExp->get_expressions();  
      
    // Simultaneously iterate over the formals and the actuals.
    // NB:  neither of these iteration spaces have
    //      the implicit receiver folded in.
    SgTypePtrList &typePtrList = getFormalTypes(node);
    SgTypePtrList::iterator formalIt = typePtrList.begin();

    // Iterate over actuals, not formals, since the number
    // of actuals >= the number of formals (consider 
    // varargs).  Any actuals beyond the number of formals
    // can not be references.
    for(SgExpressionPtrList::iterator actualIt = actualArgs.begin(); 
        actualIt != actualArgs.end(); ++actualIt) { 

        SgExpression *actualArg = *actualIt;
        ROSE_ASSERT(actualArg != NULL);

        SgType *type = *formalIt;
        ROSE_ASSERT(type != NULL);

        // The formal is a reference type, so take the
        // address of the actual.
        if ( isSgReferenceType(type) ) {
            OA::MemRefHandle actual_memref 
                = findTopMemRefHandle(actualArg);
            OA::OA_ptr<OA::MemRefExprIterator> mIter 
                = getMemRefExprIterator(actual_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();

                // We can only take the address of an l-value.
                // i.e., if the actual already has its address
                // taken, we can not take it again.
                if ( !child_mre->hasAddressTaken() ) {
                    mMemref2mreSetMap[actual_memref].erase(child_mre);
                    child_mre = child_mre->setAddressTaken();
                    mMemref2mreSetMap[actual_memref].insert(child_mre);
                }
            }
        }
        ++formalIt;
    }
}


/*!
   Should originally be called on a statement so astNode and stmt will
   be the same.  This is a recursive procedure that traverses down
   the AST and then does work in postorder.
   As MemRefHandles are found and MREs created, mStmt2allMemRefsMap,
   mMemRef2StmtMap, mMemref2mreSetMap, and mCallToMRE map are
   updated.
   As pointer assignments are found, mStmtToPtrPairs and
   mCallToParamPtrPairs are updated.
*/
void SageIRInterface::findAllMemRefsAndPtrAssigns(SgNode *astNode, 
                                                  OA::StmtHandle stmt)
{
    ROSE_ASSERT(astNode != NULL);  bool retVal = false;
    switch(astNode->variantT()) {

    // ---------------------------------------- Expression cases
    case V_SgExprListExp:
        {
            SgExprListExp* exprListExp = isSgExprListExp(astNode);
            ROSE_ASSERT (exprListExp != NULL);  

            // recurse on all expressions
            SgExpressionPtrList & exprs = exprListExp->get_expressions();  
            for(SgExpressionPtrList::iterator exprIter = exprs.begin();
                exprIter != exprs.end(); ++exprIter) 
            {
                SgExpression *expr = *exprIter;
                ROSE_ASSERT(expr != NULL);
                findAllMemRefsAndPtrAssigns(expr, stmt);
            }

            break;
        }
    case V_SgVarRefExp:
        {
            SgVarRefExp *varRefExp = isSgVarRefExp(astNode);
            ROSE_ASSERT(varRefExp!=NULL);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            //======= create a NamedRef
            bool addressTaken = false;
            bool accuracy = true;
            // default MemRefType, ancestors will change this if necessary
            OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
            // get the symbol for the variable
            SgVariableSymbol *symbol = varRefExp->get_symbol();
            ROSE_ASSERT(symbol != NULL);
            SgInitializedName *initName = symbol->get_declaration();
            ROSE_ASSERT(initName != NULL);
            OA::SymHandle sym = getNodeNumber(initName);
            // construct the NamedRef
            OA::OA_ptr<OA::MemRefExpr> mre;
            mre = new OA::NamedRef(addressTaken,accuracy, mrType, sym);
            
            // if is a reference type then 
            SgType *type = varRefExp->get_type();
            if (isSgReferenceType(type)) {
                // wrap the NamedRef in a Deref
                int numderefs = 1;
                mre = new OA::Deref(addressTaken,accuracy,mrType,mre,numderefs);
            }

            mMemref2mreSetMap[memref].insert(mre);
            break;
        }
    case V_SgClassNameRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionRefExp:
        {
            // occurs when calling a function directly, not thru a func ptr
            SgFunctionRefExp *funcRefExp = isSgFunctionRefExp(astNode);
            ROSE_ASSERT(funcRefExp!=NULL);
            
            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);
            
            //======= create a NamedRef for the function
            bool addressTaken = false;
            bool accuracy = true;
            OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
            // Get the declaration of the function.
            SgFunctionSymbol *functionSymbol = funcRefExp->get_symbol();
            ROSE_ASSERT(functionSymbol != NULL);
            SgFunctionDeclaration *functionDeclaration =
                functionSymbol->get_declaration();
            ROSE_ASSERT(functionDeclaration != NULL);
            OA::SymHandle sym = getProcSymHandle(functionDeclaration);
            // construct the NamedRef
            OA::OA_ptr<OA::MemRefExpr> mre;
            mre = new OA::NamedRef(addressTaken,accuracy, mrType, sym);
            
            mMemref2mreSetMap[memref].insert(mre);
            break;
        }
    case V_SgMemberFunctionRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionCallExp:
        {
            SgFunctionCallExp *funcCallExp = isSgFunctionCallExp(astNode);
            ROSE_ASSERT(funcCallExp!=NULL);

            // recurse on children
            findAllMemRefsAndPtrAssigns(funcCallExp->get_function(),stmt);
            findAllMemRefsAndPtrAssigns(funcCallExp->get_args(),stmt);

            // is a CallHandle
            // FIXME: should I go ahead and assign this call to this stmt here?
            OA::CallHandle call = getCallHandle(astNode);
            OA::MemRefHandle funcMemRef 
                = findTopMemRefHandle(funcCallExp->get_function());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(funcMemRef);
            mCallToMRE[call] = mIter->current();
            
            // FIXME: still needs implemented
            // if the function is malloc then
                // is a MemRefHandle
                // create UnnamedRef
            // else if the function returns a pointer or reference?
                // is a MemRefHandle
                // clone MRE for function field
            // else
                // is not a MemRefHandle
            
            // Look at the actuals passed to the 
            // function/method/constructor/destructor invocation.
            // If its corresponding formal type is a reference,
            // then we need to take the address of the actual
            // (as part of modeling references as pointers).
            // NB:  Make certain we do this before creating
            //      the param bindings, which will look at the
            //      stored MREs.
            convertReferenceActuals(funcCallExp);

            // Create parameter binding params arising from
            // the actual arguments and any potential receiver,
            // for non-static method invocations, 
            // of the function/method invocation.
            createParamBindPtrAssignPairs(funcCallExp);

            break;
        }
    case V_SgSizeOfOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgConditionalExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgNewExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgDeleteExp:
        {
            SgDeleteExp *deleteExp = isSgDeleteExp(astNode);
            ROSE_ASSERT(deleteExp != NULL);

            // Create parameter binding params arising from
            // the receiver used to invoke the destructor.
            createParamBindPtrAssignPairs(deleteExp);

            ROSE_ASSERT(0);
            break;
        }
    case V_SgThisExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarArgStartOp:
    case V_SgVarArgCopyOp:
    case V_SgVarArgStartOneOperandOp:
        {
            // don't think we are seeing these now, 8/9/06 Dan email
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarArgOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarArgEndOp:
        {
            ROSE_ASSERT(0);
            break;
        }


    // ---------------------------------------- Initializer cases
    case V_SgInitializedName:
        {
            SgInitializedName *initName = isSgInitializedName(astNode);
            ROSE_ASSERT(initName != NULL);

            // if initptr is not null then             
            if (initName->get_initptr()!=NULL) {
                // recurse on child
                findAllMemRefsAndPtrAssigns(initName->get_initptr(),stmt);

                // is a MemRefHandle
                OA::MemRefHandle memref = getMemRefHandle(astNode);
                mStmt2allMemRefsMap[stmt].insert(memref);

                //======= create a DEF NamedRef
                // make a NamedRef for the variable being initialized
                bool addressTaken = false;
                bool accuracy = true;
                OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::DEF;
                // get the symbol for the variable
                OA::SymHandle sym = getNodeNumber(initName);
                // construct the NamedRef
                OA::OA_ptr<OA::MemRefExpr> mre;
                mre = new OA::NamedRef(addressTaken,accuracy, mrType, sym);

                mMemref2mreSetMap[memref].insert(mre);

                OA::MemRefHandle child_memref 
                    = findTopMemRefHandle(initName->get_initptr());
                // if is a reference then set the addressOf flag for the
                // MREs for the node under the SgInitializer node
                if (isSgReferenceType(getBaseType(initName->get_type()))) {
                    OA::OA_ptr<OA::MemRefExprIterator> mIter 
                        = getMemRefExprIterator(child_memref);
                    for ( ; mIter->isValid(); ++(*mIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
                        mMemref2mreSetMap[child_memref].erase(child_mre);
                        // an address taken will cancel out a deref and
                        // return the result
                        child_mre = child_mre->setAddressTaken();
                        mMemref2mreSetMap[child_memref].insert(child_mre);
                    }
                }

                //----------- Ptr Assigns
                if (isSgReferenceType(initName->get_type())
                    || isSgPointerType(initName->get_type())
                    || isSgArrayType(initName->get_type()) ) 
                {
                    makePtrAssignPair(stmt, memref, child_memref);
                }
            }
            break;
        }
    case V_SgAggregateInitializer:
        {
            SgAggregateInitializer *aggInit = isSgAggregateInitializer(astNode);
            ROSE_ASSERT(aggInit!=NULL);

            // recurse on this just in case there are uses in the SgExprList
            findAllMemRefsAndPtrAssigns(aggInit->get_initializers(),stmt);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // create an UnnamedRef
            OA::OA_ptr<OA::MemRefExpr> mre;
            bool addressTaken = true;
            bool fullAccuracy = false;
            mre = new OA::UnnamedRef(addressTaken, fullAccuracy,
                                     OA::MemRefExpr::USE, stmt);

            mMemref2mreSetMap[memref].insert(mre);
            break;
        }
    case V_SgConstructorInitializer:
        {
            SgConstructorInitializer *ctorInit =
                isSgConstructorInitializer(astNode);
            ROSE_ASSERT(ctorInit != NULL);

            // Look at the actuals passed to the 
            // function/method/constructor/destructor invocation.
            // If its corresponding formal type is a reference,
            // then we need to take the address of the actual
            // (as part of modeling references as pointers).
            // NB:  Make certain we do this before creating
            //      the param bindings, which will look at the
            //      stored MREs.
            convertReferenceActuals(ctorInit);

            // Create parameter binding params arising from
            // the actual arguments and receiver (objected created)
            // of the constructor invocation.
            createParamBindPtrAssignPairs(ctorInit);

            ROSE_ASSERT(0);
            break;
        }
    case V_SgAssignInitializer:
        {
            SgAssignInitializer *assignInit = isSgAssignInitializer(astNode);
            ROSE_ASSERT(assignInit != NULL);

            // recurse on child
            findAllMemRefsAndPtrAssigns(assignInit->get_operand_i(),stmt);
            break;
        }

    // ---------------------------------------- Binary Op cases
    case V_SgArrowExp:
        {
            SgArrowExp *arrowExp = isSgArrowExp(astNode);
            ROSE_ASSERT(arrowExp != NULL);
            
            // recurse on lhs and rhs
            findAllMemRefsAndPtrAssigns(arrowExp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(arrowExp->get_rhs_operand(),stmt);
            
            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // clones MREs for lhs and wraps them in a Deref and a FieldAccess
            OA::MemRefHandle lhs_memref 
                = findTopMemRefHandle(arrowExp->get_lhs_operand());
            OA::MemRefHandle rhs_memref 
                = findTopMemRefHandle(arrowExp->get_rhs_operand());
            std::string field_name = findFieldName(rhs_memref);
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(lhs_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();
                OA::OA_ptr<OA::Deref> deref_mre;
                bool addressTaken = false;
                int numDerefs = 1;
                OA::OA_ptr<OA::MemRefExpr> nullMRE;
                deref_mre = new OA::Deref( addressTaken, 
                                           lhs_mre->hasFullAccuracy(),
                                           OA::MemRefExpr::USE,
                                           nullMRE,
                                           numDerefs);
                OA::OA_ptr<OA::MemRefExpr> fieldAccess;
                addressTaken = false;
                fieldAccess = new OA::FieldAccess(false, 
                                              deref_mre->hasFullAccuracy(),
                                              OA::MemRefExpr::USE,
                                              deref_mre->composeWith(lhs_mre),
                                              field_name);
                mMemref2mreSetMap[memref].insert(fieldAccess);
            }
            // make rhs not a MemRefHandle
            mMemref2mreSetMap[rhs_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(rhs_memref);

            break;
        }
    case V_SgDotExp:
        {
            SgDotExp *dotExp = isSgDotExp(astNode);
            ROSE_ASSERT(dotExp != NULL);
            
            // recurse on lhs and rhs
            findAllMemRefsAndPtrAssigns(dotExp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(dotExp->get_rhs_operand(),stmt);
            
            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // takes MREs from lhs and wraps them in a FieldAccess
            OA::MemRefHandle lhs_memref 
                = findTopMemRefHandle(dotExp->get_lhs_operand());
            OA::MemRefHandle rhs_memref 
                = findTopMemRefHandle(dotExp->get_rhs_operand());
            std::string field_name = findFieldName(rhs_memref);
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(lhs_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();
                OA::OA_ptr<OA::MemRefExpr> fieldAccess;
                bool addressTaken = false;
                fieldAccess = new OA::FieldAccess(false, 
                                                  lhs_mre->hasFullAccuracy(),
                                                  OA::MemRefExpr::USE,
                                                  lhs_mre,
                                                  field_name);
                mMemref2mreSetMap[memref].insert(fieldAccess);
            }

            // make lhs and rhs not a MemRefHandle
            mMemref2mreSetMap[lhs_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(lhs_memref);
            mMemref2mreSetMap[rhs_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(rhs_memref);
            break;
        }
    case V_SgDotStarOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgArrowStarOp:
        {
            ROSE_ASSERT(0);
            break;
        }

    case V_SgAddOp:
    case V_SgSubtractOp:
        {
            // these two could be used in ptr arithmetic
            // if they are then take the memory reference handle
            // away from the child
            SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
            ROSE_ASSERT(binaryOp != NULL);
            // recurse on lhs and rhs
            findAllMemRefsAndPtrAssigns(binaryOp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(binaryOp->get_rhs_operand(),stmt);
            
            // if either of the children are array types 
            // then this node should be a MemRefHandle
            // because a[2] is represented as a+2 in Sage
            // if either of the children are pointer types
            // then this node should be a MemRefHandle due to 
            // ptr arithmetic
            createMemRefExprsForPtrArith(binaryOp, 
                                         binaryOp->get_lhs_operand(), stmt);
            createMemRefExprsForPtrArith(binaryOp, 
                                         binaryOp->get_rhs_operand(), stmt);
            break;
        }
    case V_SgEqualityOp:
    case V_SgLessThanOp:
    case V_SgGreaterThanOp:
    case V_SgNotEqualOp: 
    case V_SgLessOrEqualOp:
    case V_SgGreaterOrEqualOp:
    case V_SgMultiplyOp:
    case V_SgDivideOp:
    case V_SgIntegerDivideOp:
    case V_SgModOp: 
    case V_SgAndOp:
    case V_SgOrOp:
    case V_SgBitXorOp: 
    case V_SgBitAndOp:
    case V_SgBitOrOp:
    case V_SgCommaOpExp:
    case V_SgLshiftOp:
    case V_SgRshiftOp:
        {
            SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
            ROSE_ASSERT(binaryOp != NULL);

            // recurse on lhs and rhs
            findAllMemRefsAndPtrAssigns(binaryOp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(binaryOp->get_rhs_operand(),stmt);
            break;
        }
    case V_SgPntrArrRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAssignOp:
        {
            SgBinaryOp *assignOp = isSgBinaryOp(astNode);
            ROSE_ASSERT(assignOp != NULL);

            // recurse on children
            findAllMemRefsAndPtrAssigns(assignOp->get_lhs_operand(), stmt); 
            findAllMemRefsAndPtrAssigns(assignOp->get_rhs_operand(), stmt); 

            // astNode is a MemRefHandle because it results in a use 
            // of the location indicated by the lhs            
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // 1) the MREs for this are the MREs for left child set to USE
            // 2) change MRType for top memory reference on the lhs to DEF 
            OA::MemRefHandle lhs_memref 
                = findTopMemRefHandle(assignOp->get_lhs_operand());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(lhs_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();
                // 1) take the lhs mres, clone them, set to USE
                OA::OA_ptr<OA::MemRefExpr> mre = lhs_mre->clone();
                mre->setMemRefType(OA::MemRefExpr::USE);
                mMemref2mreSetMap[memref].insert(mre);

                // 2) change to a define
                lhs_mre->setMemRefType(OA::MemRefExpr::DEF);
            }

            // if the rhs is a SgFunctionRefExpr then we are assigning
            // a function to a function ptr and we need to set
            // the address of for the rhs MREs
            OA::MemRefHandle rhs_memref 
                = findTopMemRefHandle(assignOp->get_rhs_operand());
            if (isSgFunctionRefExp(getSgNode(rhs_memref))) {
                mIter = getMemRefExprIterator(rhs_memref);
                for ( ; mIter->isValid(); ++(*mIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> rhs_mre = mIter->current();
                    rhs_mre->setAddressTaken(true);
                }
            }

            //----------- Ptr Assigns
            // FIXME: some of this logic might be useful for initializations
            // rhs top memref can only have an lval or pointer val if it is
            // NOT MemRefHandle(0)
            if (rhs_memref!=OA::MemRefHandle(0))  {
                SgExpression* lhs_expr 
                    = isSgExpression(assignOp->get_lhs_operand());
                SgType *type = lhs_expr->get_type();
                
                bool ptr_assign_found = false;
                // if the top mem ref for the lhs_operand is a pointer type
                if (isSgPointerType(type)) {
                    ptr_assign_found = true;

                // if top mem ref for lhs is a reference type and rhs has lval
                } else if (isSgReferenceType(type) && is_lval(rhs_memref) ) {
                    ptr_assign_found = true;
                    // remove the deref from the lhs, 
                    // by default references have deref put on assuming a use
                    mIter = getMemRefExprIterator(lhs_memref);
                    for ( ; mIter->isValid(); ++(*mIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();
                        mMemref2mreSetMap[lhs_memref].erase(lhs_mre);
                        // an address taken will cancel out a deref and
                        // return the result
                        lhs_mre = lhs_mre->setAddressTaken();
                        mMemref2mreSetMap[lhs_memref].insert(lhs_mre);
                    }
                    // set the addressOf for rhs as well}
                    mIter = getMemRefExprIterator(rhs_memref);
                    for ( ; mIter->isValid(); ++(*mIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> rhs_mre = mIter->current();
                        mMemref2mreSetMap[rhs_memref].erase(rhs_mre);
                        rhs_mre = rhs_mre->setAddressTaken();
                        mMemref2mreSetMap[rhs_memref].insert(rhs_mre);
                    }
                }
                // if a ptr assignment was found then make pairs
                if (ptr_assign_found) {
                    makePtrAssignPair(stmt, lhs_memref, rhs_memref);
                }
            }
            break;
        }
    case V_SgPlusAssignOp:
    case V_SgMinusAssignOp:
        // FIXME: do we need to worry about pointer arithmetic involving the
        // above two?
    case V_SgAndAssignOp:
    case V_SgIorAssignOp:
    case V_SgMultAssignOp:
    case V_SgDivAssignOp:
    case V_SgModAssignOp:
    case V_SgXorAssignOp:
    case V_SgLshiftAssignOp:
    case V_SgRshiftAssignOp:
        {
            SgBinaryOp* binaryOp = isSgBinaryOp(astNode);
            ROSE_ASSERT(binaryOp);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // recurse on children
            findAllMemRefsAndPtrAssigns(binaryOp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(binaryOp->get_rhs_operand(),stmt);

            // its child's MREs should be made USEDEF, 
            // and it should have a similar set of MREs with USE
            OA::MemRefHandle lhs_memref 
                = findTopMemRefHandle(binaryOp->get_lhs_operand());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(lhs_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();
                OA::OA_ptr<OA::MemRefExpr> mre = lhs_mre->clone();
                mre->setMemRefType(OA::MemRefExpr::USE);
                mMemref2mreSetMap[memref].insert(mre);
                lhs_mre->setMemRefType(OA::MemRefExpr::USEDEF);
            }
            break; 
        }
    // ---------------------------------------- Unary Op cases
    case V_SgExpressionRoot:
    case V_SgMinusOp:
    case V_SgUnaryAddOp:
    case V_SgNotOp:
    case V_SgBitComplementOp:
    case V_SgCastExp:
    case V_SgThrowOp:
        {
            findAllMemRefsAndPtrAssigns(((SgUnaryOp*)astNode)->get_operand(), stmt);
            break;
        }
    case V_SgPointerDerefExp:
        {
            SgPointerDerefExp *ptrDerefOp = isSgPointerDerefExp(astNode);
            ROSE_ASSERT(ptrDerefOp != NULL);

            // recurse on child
            findAllMemRefsAndPtrAssigns(ptrDerefOp->get_operand_i(),stmt);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // get the MemRefHandle for the child
            OA::MemRefHandle child_memref 
                = findTopMemRefHandle(ptrDerefOp->get_operand_i());

            // create a Deref to wrap each MRE of child
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(child_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
                OA::OA_ptr<OA::Deref> deref_mre;
                bool addressTaken = false;
                int numDerefs = 1;
                OA::OA_ptr<OA::MemRefExpr> nullMRE;
                deref_mre = new OA::Deref( addressTaken, 
                                           child_mre->hasFullAccuracy(),
                                           OA::MemRefExpr::USE,
                                           nullMRE,
                                           numDerefs);
                // use composeWith so that any necessary canonicalization occurs
                mMemref2mreSetMap[memref].insert(
                        deref_mre->composeWith(child_mre));
            }

            break;
        }
    case V_SgAddressOfOp:
        {
            SgAddressOfOp *addressOfOp = isSgAddressOfOp(astNode);
            ROSE_ASSERT(addressOfOp != NULL);

            // recurse on child
            findAllMemRefsAndPtrAssigns(addressOfOp->get_operand_i(),stmt);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // MREs are all MREs for child node with addressOf=true
            OA::MemRefHandle child_memref 
                = findTopMemRefHandle(addressOfOp->get_operand_i());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(child_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
                child_mre = child_mre->setAddressTaken();
                mMemref2mreSetMap[memref].insert(child_mre);
            }

            // child is no longer a MemRefHandle
            mMemref2mreSetMap[child_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(child_memref);
            break;
        }
    case V_SgMinusMinusOp:
    case V_SgPlusPlusOp:
        {
            SgUnaryOp* unaryOp = isSgUnaryOp(astNode);
            ROSE_ASSERT(unaryOp);

            // recurse on child
            findAllMemRefsAndPtrAssigns(unaryOp->get_operand(),stmt);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            SgUnaryOp::Sgop_mode mode = unaryOp->get_mode();

            // its child's MREs should be made USEDEF, 
            // and it should have a similar set of MREs with USE
            OA::MemRefHandle child_memref 
                = findTopMemRefHandle(unaryOp->get_operand());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(child_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
                OA::OA_ptr<OA::MemRefExpr> mre = child_mre->clone();
                mre->setMemRefType(OA::MemRefExpr::USE);
                mMemref2mreSetMap[memref].insert(mre);
                // the memory reference
                // to a in ++a and a++, a is always used before it is 
                // defined.  Whether the update occurs before or after 
                // the use of ++a or a++ is something that
                // we will specify when there is someway to indicate 
                // ordering between MemRefHandles.
                // Memory reference is used first and then defined.
                child_mre->setMemRefType(OA::MemRefExpr::USEDEF);
            }
            break;
        }

    // ---------------------------------------- Statement cases
    case V_SgExprStatement:
        {
            SgExprStatement *exprStatement = isSgExprStatement(astNode);
            ROSE_ASSERT(exprStatement!=NULL);
            findAllMemRefsAndPtrAssigns(exprStatement->get_expression_root(),stmt);
            break;
        }
    //case V_SgCaseOptionStatement: // NOT in enum? spelled differently?
    //    {
    //        ROSE_ASSERT(0);
    //        break;
    //    }
    //case V_SgTryStatement:
    //    {
    //        ROSE_ASSERT(0);
    //        break;
    //    }
    case V_SgReturnStmt:
        {
            SgReturnStmt *returnStmt = isSgReturnStmt(astNode);
            ROSE_ASSERT(returnStmt != NULL);
            findAllMemRefsAndPtrAssigns(returnStmt->get_return_expr(), stmt);

            //----------- Ptr Assigns
            // if the method or function we are in has a ptr or 
            // reference return type
            // Make a pair with a NamedRef for the function being assigned
            // the memory references for the top MemRefHandle of the return exp
            SgFunctionDefinition *enclosingFunction =
                getEnclosingFunction(returnStmt);
            ROSE_ASSERT(enclosingFunction != NULL);
            SgFunctionDeclaration *functionDeclaration =
                enclosingFunction->get_declaration();
            ROSE_ASSERT(functionDeclaration != NULL);
        
            SgType *return_type = functionDeclaration->get_orig_return_type();
            if (isSgReferenceType(return_type) || isSgPointerType(return_type)){
  
                // Create the lhs to represent the method declaration.
                bool addressTaken = false;
                bool fullAccuracy = true;
                OA::SymHandle symHandle = getProcSymHandle(functionDeclaration);
    
                OA::OA_ptr<OA::MemRefExpr> function;
                function = new OA::NamedRef(addressTaken, 
                                            fullAccuracy,
                                            OA::MemRefExpr::DEF,
                                            symHandle);

                // get the memory reference handle for the return exp
                OA::MemRefHandle child_memref 
                    = findTopMemRefHandle(returnStmt->get_return_expr());
                SgExpression *child_node=NULL;
                SgType *child_type;
                if (child_memref!=OA::MemRefHandle(0)) {
                    child_node = isSgExpression(getSgNode(child_memref));
                    ROSE_ASSERT(child_node);
                    child_type = child_node->get_type();
                }

                // if returning a reference then need to modify the MREs
                // for the return expression
                // FIXME: very similar to some code in SgAssignOp case
                if (isSgReferenceType(child_type)) {
                    // set the addressOf for child 
                    OA::OA_ptr<OA::MemRefExprIterator> mIter;
                    mIter = getMemRefExprIterator(child_memref);
                    for ( ; mIter->isValid(); ++(*mIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
                        mMemref2mreSetMap[child_memref].erase(child_mre);
                        child_mre = child_mre->setAddressTaken();
                        mMemref2mreSetMap[child_memref].insert(child_mre);
                    }
                }

                makePtrAssignPair(stmt, function, child_memref);
            }
 
            break;
        }
    case V_SgSpawnStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVariableDeclaration:
        {
            // recurse on variables
            SgVariableDeclaration *varDecl = isSgVariableDeclaration(astNode);
            ROSE_ASSERT(varDecl != NULL);
            SgInitializedNamePtrList &variables = varDecl->get_variables();
            SgInitializedNamePtrList::iterator varIter;
            for (varIter=variables.begin(); varIter!=variables.end(); ++varIter)
            {
                SgNode *lhs = *varIter;
                ROSE_ASSERT(lhs != NULL);
                findAllMemRefsAndPtrAssigns(lhs,stmt);
            }
            break;
        }
    case V_SgVariableDefinition:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAsmStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionParameterList:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgCtorInitializerList:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgIfStmt:
        {
            SgIfStmt *ifStmt = isSgIfStmt(astNode);
            ROSE_ASSERT(ifStmt != NULL);
          
            SgStatement *condition = ifStmt->get_conditional();
            if ( condition != NULL ) {
                findAllMemRefsAndPtrAssigns( condition, stmt );
            }
            break;
        }
    case V_SgSwitchStatement:
        {
            SgSwitchStatement *switchStmt = isSgSwitchStatement(astNode);
            ROSE_ASSERT(switchStmt != NULL);
            SgStatement *selector = switchStmt->get_item_selector();
            if ( selector != NULL) {
                findAllMemRefsAndPtrAssigns( selector, stmt );
            }
            break;
        }
    case V_SgForStatement:
        {
            SgForStatement *forStatement = isSgForStatement(astNode);        
            ROSE_ASSERT(forStatement != NULL);
        
            SgForInitStatement *forInitStmt = forStatement->get_for_init_stmt();
            if (forInitStmt != NULL) {
                findAllMemRefsAndPtrAssigns( forInitStmt, stmt );
            }
            SgExpression *testExpr = forStatement->get_test_expr();
            if (testExpr != NULL) {
                findAllMemRefsAndPtrAssigns( testExpr, stmt );
            }

            // I'm pretty suspicious of putting this here.  bwhite.
            // FIXME: MMS, yeah I agree, I think this needs to be
            // associated with the increment statement somehow?
            SgExpression *incrExpr = forStatement->get_increment_expr();
            if (incrExpr != NULL) {
                findAllMemRefsAndPtrAssigns( incrExpr, stmt );
            }
            break;
        }
    case V_SgWhileStmt:
        {
            SgWhileStmt *whileStmt = isSgWhileStmt(astNode);
            ROSE_ASSERT(whileStmt != NULL);

            SgStatement *condition = whileStmt->get_condition();
            if ( condition != NULL ) {
                findAllMemRefsAndPtrAssigns( condition, stmt );
            }
            break;
        }
    case V_SgDoWhileStmt:
        {
            // recurse on condition
            SgDoWhileStmt *doWhileStmt = isSgDoWhileStmt(astNode);
            ROSE_ASSERT(doWhileStmt != NULL);

            SgStatement *condition = doWhileStmt->get_condition();
            if ( condition != NULL ) {
                findAllMemRefsAndPtrAssigns( condition, stmt );
            }
            break;
        }
    case V_SgCatchOptionStmt:
        {
            // recurse on condition
            SgCatchOptionStmt *catchOptionStmt = isSgCatchOptionStmt(astNode);
            ROSE_ASSERT(catchOptionStmt != NULL);
        
            SgVariableDeclaration *condition = catchOptionStmt->get_condition();
            if ( condition != NULL ) {
                findAllMemRefsAndPtrAssigns( condition, stmt );
            }
            break;
        }


    // The statement cases that we should not see
    case V_SgNamespaceDefinitionStatement:
    case V_SgClassDefinition:
        ROSE_ASSERT(0);
        break;

    case V_SgStringVal:
        {
            // BW 7/7/06
            // Added to handle bug reported by Michelle:
            // the following statement:
            //    char a[] = "testing";
            // gets only one MEMREFEXPR:
            //    MEMREFEXPRS = { StmtHandle("char a[] = "testing";") =>
            //        [
            //            MemRefHandle("a") =>
            //                NamedRef( DEF, SymHandle("a"), F, full)
            //        ] }
            // There needs to be an UnnamedRef for the "testing" string 
            // and a PtrAssign otherwise the alias analysis will 
            // have *a and anything that aliases it access
            // UnknownLoc.

            SgStringVal *stringVal = isSgStringVal(astNode);
            ROSE_ASSERT(stringVal != NULL);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            //======= create a NamedRef
            // Set addressTaken and fullAccuracy like new/malloc.
            // string value computes the address of a memory location,
            // so consider it an addressOf operation.
            bool addressTaken = true;

            // This does _not_ accurately represent the memory 
            // expression, as this would require the precise calling context.
            bool accuracy = false;
            // default MemRefType, ancestors will change this if necessary
            OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
            // get the symbol for the string
            OA::StmtHandle stmtHandle;
            stmtHandle = getNodeNumber(stringVal);

            OA::OA_ptr<OA::MemRefExpr> mre;
            mre = new OA::UnnamedRef(addressTaken, accuracy, 
                                     mrType, stmtHandle);

            mMemref2mreSetMap[memref].insert(mre);

            break;
        }


    // The "do nothing" statement cases
    case V_SgFunctionDefinition:
    case V_SgLabelStatement:
    case V_SgDefaultOptionStmt:
    case V_SgBreakStmt:
    case V_SgContinueStmt:
    case V_SgGotoStatement:
    case V_SgForInitStatement: //FIXME: not sure about this one ???
    //case V_SgCatcheStatementSeq:  // not in enum?
    case V_SgClinkageStartStatement:
    case V_SgEnumDeclaration:
    case V_SgTemplateDeclaration:
    case V_SgNamespaceDeclarationStatement:
    case V_SgNamespaceAliasDeclarationStatement:
    case V_SgUsingDirectiveStatement:
    case V_SgUsingDeclarationStatement:
    case V_SgPragmaDeclaration:
    case V_SgGlobal:
    case V_SgBasicBlock:
    case V_SgBoolValExp:
    case V_SgCharVal:
    case V_SgDoubleVal:
    case V_SgEnumVal:
    case V_SgFloatVal: 
    case V_SgIntVal:
    case V_SgLongDoubleVal:
    case V_SgLongIntVal:
    case V_SgLongLongIntVal:
    case V_SgShortVal:
    case V_SgUnsignedCharVal:
    case V_SgUnsignedIntVal:
    case V_SgUnsignedLongLongIntVal:
    case V_SgUnsignedLongVal:
    case V_SgUnsignedShortVal:
    case V_SgWcharVal:
        break;

    default:
        {
            // do nothing, there are nodes not listed above such as SgIntVal

            // I think we should ROSE_ABORT().  If you think we _really_
            // should not be handling a case, add it to the no-op case
            // directly above.  BW 8/22/06
            std::cerr << "Do not know how to handle a " 
                      << astNode->sage_class_name()
                      << std::endl;
            ROSE_ABORT();
            break;
        }

    } // end of switch
}

//! finds the topmost MemRefHandle in the subtree rooted at the given node
//! if the tree represents only an rvalue, then MemRefHandle(0) is returned
//! unless the rvalue is an addressOf, which is assigned a MemRefHandle
OA::MemRefHandle SageIRInterface::findTopMemRefHandle(SgNode *astNode)
{
    if (astNode==NULL) {
        return OA::MemRefHandle(0);
    }

    OA::MemRefHandle memref = getMemRefHandle(astNode);
    // if there are MREs assocated with this node, then return node    
    if (!mMemref2mreSetMap[memref].empty()) {
        return getMemRefHandle(astNode);

    } else {
        // determine the number of children the node has
        std::vector< SgNode * > kids 
            = astNode->get_traversalSuccessorContainer();

        // if only one child the return topMemRefHandle on that
        if (kids.size()==1) {
            return findTopMemRefHandle(kids[0]);

        // else if two or more children then return MemRefHandle(0) 
        } else {
            return getMemRefHandle(0);
        }
    }
}

//! For a given MemRefHandle determines if the associated MREs 
//! are lvals.  MREs that have addressOf set are not lvals since
//! the indicate the computation of an address instead of describing
//! a location that has an address.
bool SageIRInterface::is_lval(OA::MemRefHandle memref) 
{
    bool retval = false;
    if (memref!=OA::MemRefHandle(0)) {
        retval = true;
        OA::OA_ptr<OA::MemRefExprIterator> mIter 
            = getMemRefExprIterator(memref);
        for ( ; mIter->isValid(); ++(*mIter) ) {
            OA::OA_ptr<OA::MemRefExpr> mre = mIter->current();
            if (mre->hasAddressTaken()) {
                retval = false;
            }
        }
    }
    return retval;
}
void 
SageIRInterface::makePtrAssignPair(OA::StmtHandle stmt,
                                   OA::MemRefHandle lhs_memref, 
                                   OA::MemRefHandle rhs_memref)
{
    OA::OA_ptr<OA::MemRefExprIterator> lhsIter;
    lhsIter = getMemRefExprIterator(lhs_memref);
    OA::OA_ptr<OA::MemRefExprIterator> rhsIter;
    rhsIter = getMemRefExprIterator(rhs_memref);
    for ( ; lhsIter->isValid(); ++(*lhsIter) ) {
      OA::OA_ptr<OA::MemRefExpr> lhs_mre = lhsIter->current();
        for (rhsIter->reset(); rhsIter->isValid(); 
             ++(*rhsIter) ) 
        {
          OA::OA_ptr<OA::MemRefExpr> rhs_mre=rhsIter->current();
          mStmtToPtrPairs[stmt].insert(
            pair<OA::OA_ptr<OA::MemRefExpr>, 
                 OA::OA_ptr<OA::MemRefExpr> >(lhs_mre,rhs_mre));
        } 
    }
}

void 
SageIRInterface::makePtrAssignPair(OA::StmtHandle stmt,
                                   OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                                   OA::MemRefHandle rhs_memref)
{
    OA::OA_ptr<OA::MemRefExprIterator> rhsIter;
    rhsIter = getMemRefExprIterator(rhs_memref);
    for (rhsIter->reset(); rhsIter->isValid(); ++(*rhsIter) ) {
      OA::OA_ptr<OA::MemRefExpr> rhs_mre=rhsIter->current();
      mStmtToPtrPairs[stmt].insert(
        pair<OA::OA_ptr<OA::MemRefExpr>, 
             OA::OA_ptr<OA::MemRefExpr> >(lhs_mre,rhs_mre));
    } 
}

void
SageIRInterface::makeParamPtrPair(OA::CallHandle call,
                                  int formal,
                                  OA::OA_ptr<OA::MemRefExpr> MRE)
{
    mCallToParamPtrPairs[call].insert(std::pair<int, 
                                      OA::OA_ptr<OA::MemRefExpr> >(formal, MRE));
}


std::string SageIRInterface::findFieldName(OA::MemRefHandle memref)
{
    std::string retval = "<none>";
    OA::OA_ptr<OA::MemRefExprIterator> mIter
        = getMemRefExprIterator(memref);
    for ( ; mIter->isValid(); ++(*mIter) ) {
        OA::OA_ptr<OA::MemRefExpr> mre = mIter->current();
        if (mre->isaNamed()) {
            OA::OA_ptr<OA::NamedRef> named_mre = mre.convert<OA::NamedRef>();
            retval = toString(named_mre->getSymHandle());
        } else {
            ROSE_ASSERT(0);
        }
    }
    return retval;
}

/*!
   Takes the node where pointer arithmetic might be occuring
   and determines if it is based on the given child.
 */
void SageIRInterface::createMemRefExprsForPtrArith(SgExpression* node, 
                                  SgExpression* child, OA::StmtHandle stmt)
{
    OA::MemRefHandle child_memref  = findTopMemRefHandle(child);
    SgExpression *child_node=NULL;
    SgType *child_type;
    if (child_memref!=OA::MemRefHandle(0)) {
        child_node = isSgExpression(getSgNode(child_memref));
        ROSE_ASSERT(child_node);
        child_type = child_node->get_type();
    }
    if (child_node && 
        (isSgArrayType(child_type)||isSgPointerType(child_type)))
    {
        // this node is a MemRefHandle
        OA::MemRefHandle memref = getMemRefHandle(node);
        mStmt2allMemRefsMap[stmt].insert(memref);

        OA::OA_ptr<OA::MemRefExprIterator> mIter 
            = getMemRefExprIterator(child_memref);
        for ( ; mIter->isValid(); ++(*mIter) ) {
            OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
            if (isSgArrayType(child_type)) {
                // set the addressTaken for the var because could
                // be computing the address of the array
                child_mre->setAddressTaken(true);
                // conservatively assume that we are computing address 
                // somewhere into the array
                child_mre->setAccuracy(false);
                // take the MRE away from the lhs and make it
                // belong to this node
                mMemref2mreSetMap[child_memref].erase(child_mre);
                mStmt2allMemRefsMap[stmt].erase(child_memref);
                mMemref2mreSetMap[memref].insert(child_mre);
            }
            if (isSgPointerType(child_type)) {
                // get a clone of the lhs
                OA::OA_ptr<OA::MemRefExpr> mre = child_mre->clone();
                // set the clones accuracy to partial
                mre->setAccuracy(false);
                mMemref2mreSetMap[memref].insert(mre);
            }
        }
    }
}
