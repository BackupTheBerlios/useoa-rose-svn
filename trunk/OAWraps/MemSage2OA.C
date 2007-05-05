#include "MemSage2OA.h"
#include "SageOACallGraph.h"
#include "common.h"

#define ROSE_0_8_9a      

using namespace std;
using namespace UseOA;

bool is_lval(OA::OA_ptr<OA::MemRefExpr> mre);

bool isReferenceTypeRequiringModeling(SgType *type)
{
    return isSgReferenceType(type);
}

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

bool isFieldAccess(OA::OA_ptr<OA::MemRefExpr> mre)
{
    if ( !mre->isaRefOp() ) {
        return false;
    }

    OA::OA_ptr<OA::RefOp> refOp = mre.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    if ( !refOp->isaSubSetRef() ) {
        return false;
    }
      
    OA::OA_ptr<OA::SubSetRef> subSetRef = refOp.convert<OA::SubSetRef>();
    ROSE_ASSERT(!subSetRef.ptrEqual(0));

    return subSetRef->isaFieldAccess();
}

bool isDeref(OA::OA_ptr<OA::MemRefExpr> mre)
{
    if ( !mre->isaRefOp() ) {
        return false;
    }

    OA::OA_ptr<OA::RefOp> refOp = mre.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    if ( refOp->isaDeref() ) {
        return true;
    }

    return false;
}

/*!
   Create the parameter bindings for a call handle and
   place them in mCallToParamPtrPairs.  
   Also, apply reference conversion rules 2 and 4 to the actuals, 
   as required.
*/
void
SageIRInterface::createParamBindPtrAssignPairs(OA::StmtHandle stmt, SgNode *node)
{
    verifyCallHandleNodeType(node);
    OA::CallHandle call = getCallHandle(node);
  
    bool isCallADotExp           = false;
    bool isCallANonStaticMethodInvocation = false;

    switch(node->variantT()) {
    case V_SgFunctionCallExp:
        {      
            SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
            ROSE_ASSERT(functionCallExp != NULL);
      
            isCallANonStaticMethodInvocation = 
                isNonStaticMethodCall(functionCallExp,  
                                      isCallADotExp);
            break;
        }
    case V_SgConstructorInitializer:
        {
            isCallANonStaticMethodInvocation = true;

            SgConstructorInitializer *ctorInit =
                isSgConstructorInitializer(node);
            ROSE_ASSERT(ctorInit != NULL);

            if ( isObjectInitialization(ctorInit) ) {
                // If the parent of the constructor initialize is a var, then
                // we are in the case:
                // A a;
                // rather than:
                // A a = new A;
                // Therefore, in the implicit param binding with 'this', we
                // need to treat this as if it were a dot expression, so
                // that we take the address of a:  this = &a.

                // Note, it is _not_ sufficient that the parent be a 
                // SgInitializedName (as was checked previously), as this
                // might also indicate invocation of a base class.
                isCallADotExp = true;
            }
            break;
        }
    case V_SgDeleteExp:
        {
            isCallANonStaticMethodInvocation = true; 
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
    if ( isCallANonStaticMethodInvocation ) {

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

            /* MMS, took out because the appropriate MRE has already been
             * calculated
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
            */
	  
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

        SgType *formal_type = NULL;

        SgExpression *actual = NULL;
        SgType *actual_type = NULL;

        bool usingVarArg = false;

        OA::ExprHandle actualExpr = actualsIter->current(); 
        SgNode *actualNode = getNodePtr(actualExpr);
        if ( actualNode == NULL ) {
            // The OA::Expr/MemRefHandle representing the actual
            // may legitimately be NULL, which causes actualNode
            // to be NULL.  This occurs when the actual is not
            // a memory reference, such as 5, sizeof(int), etc.
            // Obviously, if there is no memory reference, there
            // is no pointer assignment and we have no work to do.
            goto nxt_iter;
        } 

        // In the presence of varargs, we may have fewer
        // formals than actuals.
        if ( formalIt != typePtrList.end() ) {
            formal_type = getBaseType(*formalIt);
        }

        actual = isSgExpression(actualNode);
        ROSE_ASSERT(actual != NULL);
        actual_type = getBaseType(actual->get_type()); 
        ROSE_ASSERT(actual_type != NULL);

        // In the presence of varags, get the type from the actual.
        if ( ( formal_type == NULL ) || ( isSgTypeEllipse(formal_type) ) ) {
            formal_type = actual_type;
            // Do not increment the parameter number since
            // the rest of the args are varargs.
            incrementParamNum = false;
            usingVarArg = true;
        }
        ROSE_ASSERT(formal_type != NULL);
    
        if ( isSgReferenceType(formal_type) || isSgPointerType(formal_type)
             || isSgArrayType(formal_type) ) {
            treatAsPointerParam = true;
        }

        if ( treatAsPointerParam ) {

            // As above, take the long way to convert an ExprHandle to
            // a MemRefHandle.       
            OA::MemRefHandle actual_memref 
                = findTopMemRefHandle(actualNode);

            // Notice that actual_memref may be NULL/0.
            // For example, consider that printf's first formal is
            // a const char *.  There is no MemRefHandle here (is there?)

            OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
                = getMemRefExprIterator(actual_memref);
      
            // for each mem-ref-expr associated with this memref
            for (actualMreIterPtr->reset(); actualMreIterPtr->isValid(); 
                 (*actualMreIterPtr)++) {

                OA::OA_ptr<OA::MemRefExpr> actual_mre = 
                    actualMreIterPtr->current();

                // If the parameter is a reference type, we need to apply
                // reference conversion rules 2 and 4.  Rule 3 has already
                // been applied when the mre was created.
                if ( isReferenceTypeRequiringModeling(formal_type) && 
                     !usingVarArg ) {
             
                    // Very similar to SgInitializedName and SgReturnStmt
                    // cases, except we do not create a ptr assign
                    // pair, but a param ptr pair. 
                    if ( is_lval(actual_mre) ) {
                        // Apply reference conversion rule 4-- 
                        // take the address of the rhs.  
                        
                        // an address taken will cancel out a deref and
                        // return the result
                        mMemref2mreSetMap[actual_memref].erase(actual_mre);

                        OA::OA_ptr<OA::AddressOf> address_mre;
                        OA::OA_ptr<OA::MemRefExpr> nullMRE;

                        address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

                        actual_mre = address_mre->composeWith(actual_mre);

                        mMemref2mreSetMap[actual_memref].insert(actual_mre);

                        // We also need to be careful to remove the
                        // original reference base, lest we get two of
                        // them.  This was inserted as the Sg_File_Info
                        // of the deref'ed MRE.  XXX:  Ugly.  Encapsulate.
			//                        SgNode *node = (SgNode*)(actual_memref.hval());
                        SgNode *node = getNodePtr(actual_memref);
                        ROSE_ASSERT(node != NULL);
                 
                        Sg_File_Info *fileInfo = node->get_file_info();
                        ROSE_ASSERT(fileInfo != NULL);

                        OA::MemRefHandle hiddenMemref = getMemRefHandle(fileInfo);

                        // Need to erase the mre w/o the deref.
                        mMemref2mreSetMap[hiddenMemref].erase(actual_mre);

                    } else {
                        // Apply reference conversion rules 2 and 4.
                        OA::OA_ptr<OA::MemRefExpr> addr_of_lhs_tmp_mre;
                        addr_of_lhs_tmp_mre =
                            applyReferenceConversionRules2And4(stmt,
                                                               formal_type,
                                                               NULL,
                                                               actualNode,
                                                               actual_memref,
                                                               actual_mre);

                        actual_mre = addr_of_lhs_tmp_mre;
                    }

                // If the actual is an array type, take its address.
                } else if ( isSgArrayType(actual_type) ) {
                    mMemref2mreSetMap[actual_memref].erase(actual_mre);

                    OA::OA_ptr<OA::AddressOf> address_mre;
                    OA::OA_ptr<OA::MemRefExpr> nullMRE;

                    address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);
                                              
                    actual_mre = address_mre->composeWith(actual_mre);
                    
                    mMemref2mreSetMap[actual_memref].insert(actual_mre);
                }

                makeParamPtrPair(call, paramNum, actual_mre);
            }
        }
    nxt_iter:
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
  OA::OA_ptr<OA::IRProcIterator> procIter;
  procIter = new SageIRProcIterator(wholeProject, *this);
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

    
OA::OA_ptr<OA::MemRefExpr> 
SageIRInterface::
applyReferenceConversionRules2And4(OA::StmtHandle stmt,
                                   SgType *lhs_type,
                                   SgNode *lhs,
                                   SgNode *rhs,
                                   OA::MemRefHandle rhs_memref,
                                   OA::OA_ptr<OA::MemRefExpr> rhs_mre)
{

    ROSE_ASSERT(lhs_type != NULL);
    SgType *lhs_base_type = getBaseType(lhs_type);

    // This rule only applies when the rhs does not have an lvalue
    // (e.g., 3+5 and &y are not lvalues).  
    ROSE_ASSERT(!is_lval(rhs_mre));

    // Since the rhs does not have an lvalue, the initialization
    // stmt we are converting must be of the form:
    //    t_l const & lhs = rhs;         (1)
    // i.e., the lhs must be a reference to a const t_l.
    SgReferenceType *referenceType = isSgReferenceType(lhs_type);
    ROSE_ASSERT(referenceType != NULL);
    ROSE_ASSERT(isReferenceTypeRequiringModeling(referenceType));    

    SgType *referenceBaseType = referenceType->get_base_type();
    ROSE_ASSERT(referenceBaseType != NULL);
    ROSE_ASSERT(isConstType(referenceBaseType));
 
    // Convert (1) to 
    //    t_l lhsTmp = rhs;              (2a)
    //    t_l &lhs = lhsTmp;             (2b)
    // so that the reference initialization (2b) _does_ have a
    // rhs with an lvalue.
    
    // Handle (2a)
    // Create the MemRefExpr for lhsTmp.  
    // Use the Sg_File_Info of the rhs as the MemRefHandle
    // of the lhsTmp node in (2a).
    Sg_File_Info *fileInfo = rhs->get_file_info(); 
    ROSE_ASSERT(fileInfo != NULL);

    OA::MemRefHandle memref = getMemRefHandle(fileInfo);
    mStmt2allMemRefsMap[stmt].insert(memref);

    OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::DEF;
    OA::OA_ptr<OA::MemRefExpr> lhs_tmp_mre;

    OA::ExprHandle exprHandle = findTopExprHandle(rhs);
    if ( isSgExpression(rhs) ) {
         lhs_tmp_mre = new OA::UnnamedRef(mrType, exprHandle);
    } else {
         assert(0);
    }

    // Record the type of the MRE (reference or non-reference).
    mMre2TypeMap[lhs_tmp_mre] = other;

    mMemref2mreSetMap[memref].insert(lhs_tmp_mre);

    // If t_l is a pointer type, then we need to create 
    // a pointer assignment pair for lhsTmp = rhs;
    if ( isSgPointerType(getBaseType(referenceBaseType)) ) {
        makePtrAssignPair(stmt, lhs_tmp_mre, rhs_mre);
    }

    // Handle (2b)
		
    // By construction, lhsTmp is a non-reference with an lvalue.  
    // Therefore, we take is address according to reference 
    // conversion rule 4.

    // Use the Sg_File_Info of the lhs as the MemRefHandle
    // of the lhsTmp node in (2b).
    //    fileInfo = lhs->get_file_info(); 
    // The lhs can be NULL, so use the rhs's file info again.
    // Will this cause a problem?  This mem ref will have 2 mres.
    fileInfo = rhs->get_file_info(); 
    ROSE_ASSERT(fileInfo != NULL);

    memref = getMemRefHandle(fileInfo);
    mStmt2allMemRefsMap[stmt].insert(memref);

    OA::OA_ptr<OA::MemRefExpr> addr_of_lhs_tmp_mre = lhs_tmp_mre->clone();

    OA::OA_ptr<OA::AddressOf> address_mre;
    OA::OA_ptr<OA::MemRefExpr> nullMRE;
    
    address_mre = new OA::AddressOf(
                          OA::MemRefExpr::USE,
                          nullMRE);

    
    addr_of_lhs_tmp_mre = address_mre->composeWith(addr_of_lhs_tmp_mre);
    
    addr_of_lhs_tmp_mre->setMemRefType(OA::MemRefExpr::USE);

    // Record the type of the MRE (reference or non-reference).
    mMre2TypeMap[addr_of_lhs_tmp_mre] = other;

    mMemref2mreSetMap[memref].insert(addr_of_lhs_tmp_mre);

    return addr_of_lhs_tmp_mre;
}

static OA::OA_ptr<OA::MemRefExpr>
derefMre(OA::OA_ptr<OA::MemRefExpr> mre, OA::MemRefExpr::MemRefType mrType, int numDerefs)
{
    OA::OA_ptr<OA::Deref> deref_mre;
    ROSE_ASSERT(numDerefs == 1);
    OA::OA_ptr<OA::MemRefExpr> nullMRE;

    // create Deref mre
    OA::OA_ptr<OA::MemRefExpr> composed_mre;
    deref_mre = new OA::Deref (
                                mrType,
                                nullMRE,
                                numDerefs);

    OA::OA_ptr<OA::MemRefExpr> tmp_mre = mre->clone();
    composed_mre = deref_mre->composeWith(tmp_mre);

    /* MMS, 4/27/07, I don't think this is correct or necessary*/
    // change Deref mre to partial accuracy if 
    // original mre has partial accuracy.
    if(mre->isaRefOp()) {
        
       OA::OA_ptr<OA::RefOp> refOp;
       refOp  = mre.convert<OA::RefOp>();
       
       if(refOp->isaSubSetRef()) {
         
           OA::OA_ptr<OA::SubSetRef> subsetRef;
           
           subsetRef = refOp.convert<OA::SubSetRef>();

           if(!subsetRef->isaFieldAccess() &&
              !subsetRef->isaIdxAccess() ) {

             OA::OA_ptr<OA::SubSetRef> subset_mre;
             subset_mre = new OA::SubSetRef(
                                 OA::MemRefExpr::USE,
                                 nullMRE
                                );
             composed_mre 
                     = subset_mre->composeWith(composed_mre->clone());
           } 
       }
    }   
    /**/

    return composed_mre;
    
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

    if ( isSgExpression(astNode) ) {
        // We now keep track of all expressions (as well as mem ref handles).
        // We do this because the rhs of an assignment may be an 
        // expression, but not a mem ref handle (i.e., it may not have
        // an lval).  For example:
        //   int a,b;
        //   a = (a + b);
        // Here, (a + b) is an expression, but not a mem ref handle.
        // Therefore, while makePtrAssignPairs gets the top mem ref handles,
        // makeAssignPairs gets the top expression handles.
        ExprHandle exprHandle = getMemRefHandle(astNode);
        mStmt2allExprsMap[stmt].insert(exprHandle);
    }

    switch(astNode->variantT()) {

    // ---------------------------------------- Expression cases
    case V_SgExprListExp:
        {
            SgExprListExp* exprListExp = isSgExprListExp(astNode);
            ROSE_ASSERT (exprListExp != NULL);  

            // A SgExprListExp is used as a MemRefHandle
            // associated with an implicit this actual,
            // when one is not available in the AST.
            // e.g., for invocations of base class constructors
            // within a SgConstructorInitializerList, we
            // need to pass something as the actual for the
            // zeroth formal representing the receiver.

            // For constructors the logic to create the implicit this
            // actual now resides at SgConstructorInitializer.
            
            // Syntactically, one might suspect that we need
            // to introduce an implicit actual here as well:
            //
            // class Foo {
            //   void someMethod() { }
            //   void someOtherMethod() { someMethod(); }
            // };
            //
            // i.e., someMethod() is implicitly this->someMethod().
            // However, I believe that Sage normalizes this call.
            // Let's verify that ...

            SgNode *parent = exprListExp->get_parent();
            ROSE_ASSERT(parent != NULL);

            SgFunctionCallExp *functionCall = isSgFunctionCallExp(parent);
            if ( functionCall != NULL ) {
                SgBinaryOp *arrowOrDotExp =
                    isSgBinaryOp(functionCall->get_function());
                if ( arrowOrDotExp != NULL ) {
                    // Assert that a (non-static) method invocation 
                    // has a non-NULL receiver.
                    // Note that static method invocations have
                    // a get_function() which returns a SgMemberFunctionRef,
                    // not an arrow or dot expression.
                    ROSE_ASSERT(isSgArrowExp(arrowOrDotExp) || 
                                isSgDotExp(arrowOrDotExp));
                    ROSE_ASSERT(arrowOrDotExp->get_lhs_operand() != NULL);
                }
            } 

            /*
            // Given the above assertion, we should only need to 
            // introduce implicit 'this' actuals for invocations
            // of a baseclass contructor within a 
            // SgConstructorInitializerList.  
            SgConstructorInitializer *ctorInitializer = 
                isSgConstructorInitializer(parent);

            // If we would use this node to represent an implicit
            // this, then we need to create a MemRefHandle/MRE pair
            // for it.
            if ( ( ctorInitializer != NULL ) &&
                 ( getConstructorInitializerLhs(ctorInitializer) == exprListExp ) ) {
                // is a MemRefHandle
                OA::MemRefHandle memref = getMemRefHandle(astNode);
                mStmt2allMemRefsMap[stmt].insert(memref);

                //======= create a NamedRef
                bool addressTaken = false;
                bool accuracy = true;
                // default MemRefType, ancestors will change this if necessary
                OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
                // get the symbol for the variable

                SgFunctionDefinition *enclosingFunction =
                    getEnclosingFunction(astNode);
                ROSE_ASSERT(enclosingFunction != NULL);
                SgFunctionDeclaration *functionDeclaration =
                    enclosingFunction->get_declaration();
	        OA::SymHandle symHandle = 
                    getThisExpSymHandle(functionDeclaration);

                OA::OA_ptr<OA::MemRefExpr> mre;
                mre = new OA::NamedRef(addressTaken, 
                                       accuracy,
                                       mrType,
                                       symHandle);
                ROSE_ASSERT(!mre.ptrEqual(0));

                // Record the type of the MRE (reference or non-reference).
                mMre2TypeMap[mre] = other;
                mMemref2mreSetMap[memref].insert(mre);
            }
            */

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
           
            mre = new OA::NamedRef(mrType, sym);
            
            // Record the type of the MRE (reference or non-reference).
	        // mMre2TypeMap[mre] = ( isSgReferenceType(initName->get_type()) ? reference : other );
            mMre2TypeMap[mre] = other;

            // if is a reference type then 
            // Reference conversion rule 3.
            SgType *type = getBaseType(varRefExp->get_type());
            if (isReferenceTypeRequiringModeling(type)) {
                // wrap the NamedRef in a Deref, but before doing so also
                // create a MemRefHandle/MRE pair for the reference itself.
                // Use the Sg_File_Info as the MemRefHandle.  This should
                // effectively hide the MRE from anyone calling findTopMemRefHandle,
                // e.g., to create ptr assign pairs.  That's good, because the
                // deref should be the top memrefhandle.
                Sg_File_Info *fileInfo = astNode->get_file_info();
                ROSE_ASSERT(fileInfo != NULL);
	            OA::MemRefHandle hiddenMemref = getMemRefHandle(fileInfo);
                mStmt2allMemRefsMap[stmt].insert(hiddenMemref);
                mMemref2mreSetMap[hiddenMemref].insert(mre);

                int numderefs = 1;
                mre = derefMre(mre, mrType, numderefs);

         		// It is the deref that we will see as the top mem ref handle
                // and ask whether the access it reprsents is to a reference.
                mMre2TypeMap[mre] = reference;

            }

            // Record the type of the MRE (reference or non-reference).
	        // mMre2TypeMap[mre] = other;
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

            mre = new OA::NamedRef(mrType, sym);
            
            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[mre] = other;

            mMemref2mreSetMap[memref].insert(mre);
            break;
        }
    case V_SgMemberFunctionRefExp:
        {
            // FIXME: shares enough code with SgFunctionRefExp
            // that we probably want to factor the two
            SgMemberFunctionRefExp *memberFunctionRefExp 
                = isSgMemberFunctionRefExp(astNode); 
            ROSE_ASSERT(memberFunctionRefExp!=NULL);

            // occurs on rhs of SgDotExp or SgArrowExp when calling member func
            // so that it is treated like other fields, will make a 
            // MemRefHandle here that will later be taken away at the SgDotExp
            // or SgArrowExp
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);
            
            //======= create a NamedRef
            OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
            // get the symbol for the member function
            SgFunctionSymbol *functionSymbol 
                = memberFunctionRefExp->get_symbol();
            ROSE_ASSERT(functionSymbol != NULL);
            SgFunctionDeclaration *functionDeclaration =
                functionSymbol->get_declaration();
            ROSE_ASSERT(functionDeclaration != NULL);
            OA::SymHandle sym = getProcSymHandle(functionDeclaration);
            // construct the NamedRef
            OA::OA_ptr<OA::MemRefExpr> mre;

            mre = new OA::NamedRef(mrType, sym);
 
            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[mre] = other;

            mMemref2mreSetMap[memref].insert(mre);
            break;
        }
    case V_SgFunctionCallExp:
        {
            
            SgFunctionCallExp *funcCallExp = isSgFunctionCallExp(astNode);
            ROSE_ASSERT(funcCallExp!=NULL);

            // recurse on children
            findAllMemRefsAndPtrAssigns(funcCallExp->get_function(),stmt);
            findAllMemRefsAndPtrAssigns(funcCallExp->get_args(),stmt);

            bool isaCallHandle = true;
            
            // if the function is malloc then
            if (isFunc(funcCallExp, "malloc")) {
                // is a MemRefHandle
                OA::MemRefHandle memref = getMemRefHandle(astNode);
                mStmt2allMemRefsMap[stmt].insert(memref);

                //======= create UnnamedRef
                OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
                OA::OA_ptr<OA::MemRefExpr> mre;

                OA::ExprHandle exprHandle = findTopExprHandle(funcCallExp);
                if ( isSgExpression(funcCallExp) ) {
                     mre = new OA::UnnamedRef(mrType, exprHandle);
                } else {
                     assert(0);
                }

                OA::OA_ptr<OA::SubSetRef> subset_mre;
                OA::OA_ptr<OA::MemRefExpr> nullMRE;
                OA::OA_ptr<OA::MemRefExpr> composed_mre;

                subset_mre = new OA::SubSetRef(
                                 OA::MemRefExpr::USE,
                                 nullMRE
                                );
                mre = subset_mre->composeWith(mre->clone());
                
                OA::OA_ptr<OA::AddressOf> address_mre;

                address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);
                
                mre = address_mre->composeWith(mre);
                // Record the type of the MRE (reference or non-reference).
                mMre2TypeMap[mre] = other;

                mMemref2mreSetMap[memref].insert(mre);
 
            // if the function is va_start then
            // hardcoding handling of varargs macros and functions
            } else if ( isVaStart(funcCallExp) ) {
                // not going to treat this guy like a normal function
                isaCallHandle = false;

                // is a MemRefHandle 
                OA::MemRefHandle memref = getMemRefHandle(astNode);
                mStmt2allMemRefsMap[stmt].insert(memref);

                // create a DEF and a USE to deref of a clone of the first 
                // actual parameter,
                // which will be of type va_list and we are assuming 
                // that is a ptr type of some kind
                SgExprListExp* exprListExp = funcCallExp->get_args();
                SgExpressionPtrList & actualArgs =
                            exprListExp->get_expressions();

                // get mem ref for the first parameter
                SgExpressionPtrList::iterator actualIt = actualArgs.begin();
                SgExpression *actualArg = *actualIt;
                ROSE_ASSERT(actualArg != NULL);
                OA::MemRefHandle actual_memref 
                    = findTopMemRefHandle(actualArg);

                createUseDefForVarArg(memref, actual_memref);

                // also remove the USE MRE for the va_start function
                OA::MemRefHandle function_memref 
                    = findTopMemRefHandle(funcCallExp->get_function());
                mMemref2mreSetMap[function_memref].clear();
                mStmt2allMemRefsMap[stmt].erase(function_memref);

            // else if the function returns a pointer or reference
            // the MRE for this node is the same as the CallMRE
            } 
            if (isaCallHandle) {
                // is a CallHandle
                // FIXME: should I go ahead and assign this call to this stmt here?
                OA::CallHandle call = getCallHandle(astNode);
                OA::MemRefHandle funcMemRef 
                    = findTopMemRefHandle(funcCallExp->get_function());
                OA::OA_ptr<OA::MemRefExprIterator> mIter
                    = getMemRefExprIterator(funcMemRef);

                OA::OA_ptr<OA::MemRefExpr> call_mre;
                call_mre = mIter->current();

                if ( isFieldAccess(call_mre ) ) {
                    // Need to deref the field access for a virtual
                    // method.
                    OA::OA_ptr<OA::Deref> deref_mre;
                    int numDerefs = 1;
                    OA::OA_ptr<OA::MemRefExpr> nullMRE;

                    // create composed_mre with full accuracy
                   
                    OA::OA_ptr<OA::MemRefExpr> composed_mre;

                    deref_mre = new OA::Deref(OA::MemRefExpr::USE,
                                             nullMRE,
                                             numDerefs);
                    composed_mre = deref_mre->composeWith(call_mre->clone());

                   // changed composed_mre to partial accuracy if
                   // original call_mre has partial accuracy
                   if(call_mre->isaRefOp()) {
                       
                       OA::OA_ptr<OA::RefOp> refOp;
                       refOp = call_mre.convert<OA::RefOp>();
                       
                       if(refOp->isaSubSetRef()) {
                          
                          OA::OA_ptr<OA::SubSetRef> subsetRef;
                          subsetRef = refOp.convert<OA::SubSetRef>();

                          if(!subsetRef->isaFieldAccess() &&
                             !subsetRef->isaIdxAccess() ) {
                             OA::OA_ptr<OA::SubSetRef> subset_mre;
                             subset_mre = new OA::SubSetRef(
                                            OA::MemRefExpr::USE,
                                            nullMRE
                                        );
                             composed_mre 
                               = subset_mre->composeWith(composed_mre->clone());
                          }
                       }
                       
                    }   

                    call_mre = composed_mre->clone();
                    
                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[call_mre] = other;
                }


#if 0
		//		std::cout << "Adding call handle for " << (int)(call.hval()) << std::endl;
		SgNode *node = getNodePtr(call);
		ROSE_ASSERT(node != NULL);
		std::cout << "call handle node (h: " << (int)(call.hval()) << " type: " << node->sage_class_name()
                  << ") " << node->unparseToString() 
                  << std::endl;
		node->get_file_info()->display("call handle: ");
#endif
                mCallToMRE[call] = call_mre;

                if (returnsAddress(funcCallExp) 
                    && !isFunc(funcCallExp, "malloc")) 
                {
                    // is a MemRefHandle
                    OA::MemRefHandle memref = getMemRefHandle(astNode);
                    mStmt2allMemRefsMap[stmt].insert(memref);
                    // clone MRE for function 
                    OA::OA_ptr<OA::MemRefExpr> mre;
                    mre = getCallMemRefExpr(getCallHandle(astNode));

  		            OA::OA_ptr<OA::MemRefExpr> cloned = mre->clone();
                    // Transfer type to cloned MRE
                    mMre2TypeMap[cloned] = mMre2TypeMap[mre];

		            OA::OA_ptr<OA::MemRefExpr> retSlot = cloned;
                    // If the return value has reference type,
                    // we need to apply reference conversion rule 3.
                    if ( returnsReference(funcCallExp) ) {
                        // Change the type of the field access to a 
                        // reference.
                        // No, see below.
		        //                        mMre2TypeMap[cloned] = reference;

                        // wrap the NamedRef in a Deref, but before doing so also
                        // create a MemRefHandle/MRE pair for the reference itself.
                        // Use the Sg_File_Info as the MemRefHandle.  This should
                        // effectively hide the MRE from anyone calling findTopMemRefHandle,
                        // e.g., to create ptr assign pairs.  That's good, because the
                        // deref should be the top memrefhandle.
                        Sg_File_Info *fileInfo = astNode->get_file_info();
                        ROSE_ASSERT(fileInfo != NULL);
  	                    OA::MemRefHandle hiddenMemref = getMemRefHandle(fileInfo);
                        mStmt2allMemRefsMap[stmt].insert(hiddenMemref);
                        mMemref2mreSetMap[hiddenMemref].insert(retSlot);
                        int numderefs = 1;

#if 1
                        retSlot = derefMre(cloned, OA::MemRefExpr::USE, numderefs);
#else
                        retSlot = 
                            new OA::Deref(cloned->hasFullAccuracy(),
                                          OA::MemRefExpr::USE,
                                          cloned,
                                          numderefs);
#endif

		        // It is the deref that we will see as the top mem ref handle
                        // and ask whether the access it reprsents is to a reference.
                        mMre2TypeMap[retSlot] = reference;
                    }
                    mMemref2mreSetMap[memref].insert(retSlot);
                }

                // Create parameter binding params arising from
                // the actual arguments and any potential receiver,
                // for non-static method invocations, 
                // of the function/method invocation.
                // This also applies reference conversion rules
                // 2 and 4, as required.
                createParamBindPtrAssignPairs(stmt, funcCallExp);
            }

            break;
        }
    case V_SgSizeOfOp:
        {
            SgSizeOfOp *sizeOp = isSgSizeOfOp(astNode);
            ROSE_ASSERT(sizeOp!=NULL);

            // recurse on child
            if (sizeOp->get_operand_expr() != NULL) {
                findAllMemRefsAndPtrAssigns(sizeOp->get_operand_expr(),stmt);
            }
            break;
        }
    case V_SgConditionalExp:
        {
            
            SgConditionalExp *condExp = isSgConditionalExp(astNode);
            ROSE_ASSERT(condExp!=NULL);

            // recurse on children
            findAllMemRefsAndPtrAssigns(condExp->get_conditional_exp(),stmt);
            findAllMemRefsAndPtrAssigns(condExp->get_true_exp(),stmt);
            findAllMemRefsAndPtrAssigns(condExp->get_false_exp(),stmt);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // take the MREs from the true and false branch and assign
            // them all to this MemRefHandle
            OA::MemRefHandle true_memref 
                = findTopMemRefHandle(condExp->get_true_exp());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(true_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> mre = mIter->current();
                mMemref2mreSetMap[memref].insert(mre);
            }
            mMemref2mreSetMap[true_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(true_memref);
            OA::MemRefHandle false_memref 
                = findTopMemRefHandle(condExp->get_false_exp());
            mIter = getMemRefExprIterator(false_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> mre = mIter->current();
                mMemref2mreSetMap[memref].insert(mre);
            }
            mMemref2mreSetMap[false_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(false_memref);
            break;
        }
    case V_SgNewExp:
        {
            SgNewExp *newExp = isSgNewExp(astNode);
            ROSE_ASSERT(newExp != NULL);

            // Invocations of new are represented by unnamed memory references,
            // just as calls to malloc.

            // Notice that we also need to represent the access to the 
            // invoked constructor/method.  Do this at the 
            // SgConstructorInitializer accessible from newExp as 
            // newExp->get_constructor_args();

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // create an UnnamedRef
            OA::OA_ptr<OA::MemRefExpr> mre;

            OA::ExprHandle exprHandle = findTopExprHandle(newExp);
            if ( isSgExpression(newExp) ) {
                 mre = new OA::UnnamedRef(OA::MemRefExpr::USE, exprHandle);
            } else {
                 assert(0);
            }


            OA::OA_ptr<OA::SubSetRef> subset_mre;
            OA::OA_ptr<OA::MemRefExpr> nullMRE;
            OA::OA_ptr<OA::MemRefExpr> composed_mre;

            subset_mre = new OA::SubSetRef(
                                 OA::MemRefExpr::USE,
                                 nullMRE
                                );
            mre = subset_mre->composeWith(mre->clone());

            
            OA::OA_ptr<OA::AddressOf> address_mre;

            address_mre = new OA::AddressOf(
                                  OA::MemRefExpr::USE,
                                  nullMRE);

            mre = address_mre->composeWith(mre);

            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[mre] = other;
            mMemref2mreSetMap[memref].insert(mre);

            SgConstructorInitializer *ctorInitializer =
                newExp->get_constructor_args();
            ROSE_ASSERT(ctorInitializer != NULL);

            findAllMemRefsAndPtrAssigns(ctorInitializer, stmt);

            break;
        }
    case V_SgDeleteExp:
        {
            SgDeleteExp *deleteExp = isSgDeleteExp(astNode);
            ROSE_ASSERT(deleteExp != NULL);

            SgExpression *receiver = deleteExp->get_variable();
            ROSE_ASSERT(receiver != NULL);

            // Visit the receiver of the delete expression,
            // which will be the implicit this.
            findAllMemRefsAndPtrAssigns(receiver, stmt);

            SgType *type = receiver->get_type(); 
            ROSE_ASSERT(type != NULL);

            // Need getBaseType to look through typedefs.
            SgPointerType *ptrType = isSgPointerType(getBaseType(type));

            ROSE_ASSERT(ptrType != NULL);
            type = getBaseType(ptrType->get_base_type());

            ROSE_ASSERT(type != NULL);

            // If this is a base type, then do nothing more--
            // i.e., not even a call handle.
            if ( isSgClassType(type) ) {

                SgClassDefinition *classDefn = getClassDefinition(type);
                ROSE_ASSERT(classDefn != NULL);
    
                SgMemberFunctionDeclaration *methodDecl = 
                    lookupDestructorInClass(classDefn); 
                ROSE_ASSERT(methodDecl != NULL); 
    
                // Create the MRE for the call handle.
                OA::OA_ptr<OA::MemRefExpr> method;
    
                // If the destructor is virtual, we need to create
                // an invocation through a field access.
                // However, according to section r.12.7 of 
                // Stroustrup 2nd edition, calls to virtual methods
                // within a constructor or destructor may be
                // statically resolved.
                SgFunctionDefinition *funcDefn =
                    getEnclosingFunction(astNode);
                SgMemberFunctionDeclaration *decl =
                    isSgMemberFunctionDeclaration(funcDefn->get_declaration());
                bool invokedFromCtorOrDtor = 
                    ( ( decl != NULL ) &&
                      ( isConstructor(decl) || isDestructor(decl) ) );

                if ( isVirtual(methodDecl) && !invokedFromCtorOrDtor ) {
    
                    // Notice that we first need to visit the receiver,
                    // so that we will have a MRE for the base of
                    // the field access.  
    
                    // clones MREs for lhs and wraps them in a Deref 
                    // and a FieldAccess
                    OA::MemRefHandle lhs_memref
                        = findTopMemRefHandle(receiver);
                    std::string field_name 
                        = toStringWithoutScope(methodDecl);
                    OA::OA_ptr<OA::MemRefExprIterator> mIter
                        = getMemRefExprIterator(lhs_memref);
                    int numMREs = 0;
                    for ( ; mIter->isValid(); ++(*mIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();
                        OA::OA_ptr<OA::Deref> deref_mre;
                        
                        int numDerefs = 1;
    
                        OA::OA_ptr<OA::MemRefExpr> nullMRE;

                        OA::OA_ptr<OA::MemRefExpr> composed_mre;

                        deref_mre = new OA::Deref(OA::MemRefExpr::USE,
                                                  nullMRE,
                                                  numDerefs);

                        OA::OA_ptr<OA::MemRefExpr> composedMre
                                    = deref_mre->composeWith(lhs_mre);


                        if(lhs_mre->isaRefOp()) {

                           OA::OA_ptr<OA::RefOp> refOp;
                           refOp = lhs_mre.convert<OA::RefOp>();

                           if(refOp->isaSubSetRef()) { 

                               OA::OA_ptr<OA::SubSetRef> subset_mre;
                               OA::OA_ptr<OA::MemRefExpr> nullMRE;
                               //OA::OA_ptr<OA::MemRefExpr> composed_mre;

                               subset_mre = new OA::SubSetRef(
                                            OA::MemRefExpr::USE,
                                            nullMRE
                                            );
                               composedMre
                                = subset_mre->composeWith(composedMre->clone());

                           }
                        }

                        // FIXME?: this throws away composedMre in above
                        // statement
                        composedMre = deref_mre->clone();   
 
                        OA::OA_ptr<OA::MemRefExpr> fieldAccess;
    
                        mMre2TypeMap[composedMre] = other;
                       
                        fieldAccess 
                             = new OA::FieldAccess(OA::MemRefExpr::USE,
                                                   composedMre,
                                                   OA_VTABLE_STR);

                        if(lhs_mre->isaRefOp()) {

                           OA::OA_ptr<OA::RefOp> refOp;
                           refOp = lhs_mre.convert<OA::RefOp>();

                           if(refOp->isaSubSetRef()) {
                            
                              OA::OA_ptr<OA::SubSetRef> subset_mre;
                              OA::OA_ptr<OA::MemRefExpr> nullMRE;
                              OA::OA_ptr<OA::MemRefExpr> composed_mre;

                              subset_mre = new OA::SubSetRef(
                                            OA::MemRefExpr::USE,
                                            nullMRE
                                            );
                              fieldAccess
                               = subset_mre->composeWith(fieldAccess->clone());

                            }
                       }

                        // Record the type of the MRE (reference or non-reference).
                        mMre2TypeMap[fieldAccess] = other;

         		        composedMre = derefMre(fieldAccess, OA::MemRefExpr::USE, 1);

                        mMre2TypeMap[composedMre] = other;

                        fieldAccess = new OA::FieldAccess(
                                                          OA::MemRefExpr::USE,
                                                          composedMre,
                                                          field_name);

                        if(lhs_mre->isaRefOp()) {

                           OA::OA_ptr<OA::RefOp> refOp;
                           refOp = lhs_mre.convert<OA::RefOp>();
                           
                           if(refOp->isaSubSetRef()) {
                              OA::OA_ptr<OA::SubSetRef> subset_mre;
                              OA::OA_ptr<OA::MemRefExpr> nullMRE;
                              OA::OA_ptr<OA::MemRefExpr> composed_mre;

                              subset_mre = new OA::SubSetRef(
                                            OA::MemRefExpr::USE,
                                            nullMRE
                                            );
                              fieldAccess
                               = subset_mre->composeWith(fieldAccess->clone());
                           }

                        }

                        
                        // Record the type of the MRE (reference or non-reference).
                        mMre2TypeMap[fieldAccess] = other;
    
                        // I do not think that there should be a general
                        // MRE here.  Just a call handle.
    
                        // mMemref2mreSetMap[memref].insert(fieldAccess);
                        method = fieldAccess;
    
                        ++numMREs;
                    }

                    // XXX:  Could we have multiple MREs here?  I don't
                    // think so.  Even if we could, mCallToMRE is
                    // not set up to handle it.  Abort if we see it.
                    ROSE_ASSERT( numMREs == 1 );
    
                    // Need to deref the field access for a virtual
                    // method.
                    OA::OA_ptr<OA::Deref> deref_mre;
                    int numDerefs = 1;
                    OA::OA_ptr<OA::MemRefExpr> nullMRE;

                    deref_mre = new OA::Deref( OA::MemRefExpr::USE,
                                               nullMRE,
                                               numDerefs);

                    OA::OA_ptr<OA::MemRefExpr> mre = method->clone();
                    method = deref_mre->composeWith(mre);

                    if(method->isaRefOp()) {
                       OA::OA_ptr<OA::RefOp> refOp;
                       refOp = method.convert<OA::RefOp>();
                       if(refOp->isaSubSetRef()) {

                           OA::OA_ptr<OA::SubSetRef> subset_mre;
                           OA::OA_ptr<OA::MemRefExpr> nullMRE;
                           OA::OA_ptr<OA::MemRefExpr> composed_mre;

                           subset_mre = new OA::SubSetRef(
                                            OA::MemRefExpr::USE,
                                            nullMRE
                                            );
                           method
                               = subset_mre->composeWith(method->clone());

                       }
                    }


                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[method] = other;

                } else {
    
                    // Create a call handle for the delete expression.
    
                   OA::SymHandle symHandle = getProcSymHandle(methodDecl);
      
                   method = new OA::NamedRef( OA::MemRefExpr::USE,
                                              symHandle);

    
                   // Record the type of the MRE (reference or non-reference).
                   mMre2TypeMap[method] = other;
    
                }
    	
                OA::CallHandle call = getCallHandle(astNode);
		//		std::cout << "Adding call handle for " << (int)(call.hval()) << std::endl;
#if 0
		SgNode *node = getNodePtr(call);
		ROSE_ASSERT(node != NULL);
		std::cout << "call handle node (h: " << (int)(call.hval()) << " type: " << node->sage_class_name()
                  << ") " << node->unparseToString() 
                  << std::endl;
		node->get_file_info()->display("call handle: ");
#endif
                mCallToMRE[call] = method;
    
                // We consider function/method invocations to be 
                // uses, so add the call as a general MRE.
                OA::MemRefHandle memref = getMemRefHandle(astNode);
                mStmt2allMemRefsMap[stmt].insert(memref);
                mMemref2mreSetMap[memref].insert(method);
    
                // Create parameter binding params arising from
                // the receiver used to invoke the destructor.
                createParamBindPtrAssignPairs(stmt, deleteExp);
            }
            break;
        }
    case V_SgThisExp:
        {
            SgThisExp *thisExp = isSgThisExp(astNode);
            ROSE_ASSERT(thisExp != NULL);

            OA::SymHandle symHandle = getThisExpSymHandle(thisExp);

            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            OA::OA_ptr<OA::MemRefExpr> mre;

            mre = new OA::NamedRef(OA::MemRefExpr::USE,
                                   symHandle);

                                   
            ROSE_ASSERT(!mre.ptrEqual(0));

            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[mre] = other;

            mMemref2mreSetMap[memref].insert(mre);
            break;
        }
    case V_SgVarArgStartOp:
    case V_SgVarArgCopyOp:
    case V_SgVarArgStartOneOperandOp:
        {
            // don't think these nodes are currently used in ROSE, 
            // 8/9/06 Dan email
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarArgOp:
        {
            SgVarArgOp *varArgOp = isSgVarArgOp(astNode);
            ROSE_ASSERT(varArgOp!=NULL);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // recurse on child
            findAllMemRefsAndPtrAssigns(varArgOp->get_operand_expr(), stmt);

            // Treat this node as a MemRefHandle that is accessing the
            // symbol for the ellipsis parameter
            OA::SymHandle formalSym;
            SgFunctionDefinition *functionDefinition 
                = getEnclosingFunction(getSgNode(stmt));
            SgFunctionDeclaration *functionDeclaration
                = functionDefinition->get_declaration();
            // Get the list of formal parameters for the function.
            SgFunctionParameterList *parameterList = 
                functionDeclaration->get_parameterList(); 
            if (parameterList != NULL) {
                // Iterate over the formal parameters as represented by
                // SgInitializedNames.  
                const SgInitializedNamePtrList &formalParams 
                    = parameterList->get_args(); 
                for(SgInitializedNamePtrList::const_iterator formalIt 
                        = formalParams.begin();
                    formalIt != formalParams.end(); ++formalIt) 
                {
                    SgInitializedName* formalParam = *formalIt;  
                    ROSE_ASSERT(formalParam != NULL); 

                    SgType* type = getBaseType(formalParam->get_type());
                    if (isSgTypeEllipse(type)) {
                        formalSym = getSymHandle(formalParam);
                        break;
                    }
                }
            }

            //======= create a NamedRef
            OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
            OA::OA_ptr<OA::MemRefExpr> mre;

            mre = new OA::NamedRef(mrType, formalSym);
  
            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[mre] = other;

            mMemref2mreSetMap[memref].insert(mre);

            // also create a use and def to the deref of the va_list
            createUseDefForVarArg(memref, 
                findTopMemRefHandle(varArgOp->get_operand_expr()));
            break;
        }
    case V_SgVarArgEndOp:
        {
            SgVarArgEndOp *varArgEndOp = isSgVarArgEndOp(astNode);
            ROSE_ASSERT(varArgEndOp!=NULL);

            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // recurse on child
            findAllMemRefsAndPtrAssigns(varArgEndOp->get_operand_expr(), stmt);

            // also create a use and def to the deref of the va_list
            createUseDefForVarArg(memref, 
                findTopMemRefHandle(varArgEndOp->get_operand_expr()));
            break;
        }


    // ---------------------------------------- Initializer cases
    case V_SgInitializedName:
        {
            SgInitializedName *initName = isSgInitializedName(astNode);
            ROSE_ASSERT(initName != NULL);

            bool found_assignment = false;

            // if initptr is not null then             
            if (initName->get_initptr()!=NULL) {
                // recurse on the child
                findAllMemRefsAndPtrAssigns(initName->get_initptr(),stmt);

                // If our immediate child is a SgConstructorInitializer
                // then this node is not a MemRefHandle and there
                // will be no pointer assignments modeled where this
                // node is the lhs.
                // Examples:
                //   class Foo : public Bar { Foo(Foo &f) : Bar(f) { } }
                //   Foo f();
                //   class Foo { Foo(Bar &b) : mBar(b) {}  Bar mBar; };
                // We do not need a DEF memory reference for the target
                // of the implicit "this" in the above cases.  When the
                // target of "this" is defined in the constructor procedure
                // that will show up in the side-effect results.
                SgConstructorInitializer *ctorInitializer =
                    isSgConstructorInitializer(initName->get_initptr());
                if (ctorInitializer) {
                    break;
                }

                // At this point, we know there is a RHS.  Therefore, we
                // need to create an assignment pair (and we may also need
                // to create a ptr assignment pair).
                found_assignment = true;

                // Since our immediate child is NOT a SgConstructorInitializer
                // we know that the will be a pointer assignment if 
                // this node represents a pointer or reference type.
                // Therefore, before we do anything like creating
                // a MemRefHandle for an implicit "this", going to get
                // the MemRefHandle for our child.
                OA::MemRefHandle child_memref 
                    = findTopMemRefHandle(initName->get_initptr());

                // If this is an initialization of a member variable
                // then we are going to need an implicit "this".
                // We will associate it with our immediate child
                // which should be a SgAssignInitializer or 
                // a SgAggregateInitializer.
                OA::MemRefHandle receiver_memref;
                bool requiresImplicitReceiver = false;
                SgNode *parent = initName->get_parent();
                ROSE_ASSERT(parent != NULL);
                if ( isSgCtorInitializerList(parent) ) {

                    // At this point, we know that initName is a
                    // variable initialization within a constructor
                    // intializer list.  That is, it is 
                    // a member variable initialization.
                    // These have an implicit receive in the Sage AST,
                    // that we need to make explicit here.
                    requiresImplicitReceiver = true;

                    // setting up the MemRefHandle for implicit "this"
                    SgInitializer *initer = initName->get_initptr();
                    ROSE_ASSERT(initer != NULL);

                    receiver_memref = getMemRefHandle(initer);
                    mStmt2allMemRefsMap[stmt].insert(receiver_memref);
                }

                // we are on the SgInitializedName node, it is the lhs
                // and it is a MemRefHandle
                OA::MemRefHandle memref = getMemRefHandle(astNode);
                mStmt2allMemRefsMap[stmt].insert(memref);

                // Create the MRE for this SgInitializedName node
                OA::OA_ptr<OA::MemRefExpr> mre;

                if ( !requiresImplicitReceiver ) {
                    //======= create a DEF NamedRef
                    // make a NamedRef for the variable being initialized
                    OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::DEF;
                    OA::SymHandle sym = getNodeNumber(initName);

                    mre = new OA::NamedRef(mrType, sym);
 
                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[mre] = ( isSgReferenceType(initName->get_type()) ? reference : other );

                } else {
                    //======= create a DEF FieldAccess
                    OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;

                    // get the symbol for the implicit formal this.
        	        OA::SymHandle symHandle = getThisFormalSymHandle(astNode);

                    OA::OA_ptr<OA::MemRefExpr> base;

                    base = new OA::NamedRef(mrType,
                                            symHandle);

                   
                    mMemref2mreSetMap[receiver_memref].insert(base);

                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[base] = other;


   		            OA::OA_ptr<OA::MemRefExpr> cloned = base->clone();
                    // Transfer type to cloned MRE
                    mMre2TypeMap[cloned] = mMre2TypeMap[base];

                    // Deref the MRE.
                    OA::OA_ptr<OA::MemRefExpr> deref_mre;
                    int numDerefs = 1;

#if 1
                    deref_mre = derefMre(cloned, OA::MemRefExpr::USE, numDerefs);
#else
                    deref_mre = new OA::Deref(accuracy,
                                              OA::MemRefExpr::USE,
                                              cloned,
                                              numDerefs);

#endif

                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[deref_mre] = other;

                    // Create the FieldAccess MRE.
                    OA::OA_ptr<OA::MemRefExpr> fieldAccess;
                    std::string field_name = toStringWithoutScope(initName);

                    fieldAccess = new OA::FieldAccess(OA::MemRefExpr::DEF,
                                                      deref_mre,
                                                      field_name);


                    mre = fieldAccess->clone();
                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[mre] = ( isSgReferenceType(initName->get_type()) ? reference : other );

                }

                mMemref2mreSetMap[memref].insert(mre);
	    
                // Iterate over the rhses.
                OA::OA_ptr<OA::MemRefExprIterator> mIter 
                    = getMemRefExprIterator(child_memref);
                for ( ; mIter->isValid(); ++(*mIter) ) {

                    OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();

                    SgType *type = getBaseType(initName->get_type());

                    // if is an array, then the init is an UnnamedRef
                    // like "my string" or {1,3,5}.  By default the UnnamedRef
                    // are set up with their addressTaken.  Since we are modeling
                    // array variables as actual arrays instead of constant
                    // ptrs to an array, we need to remove the addressTaken
                    // for the rhs MREs

                    
                    /* deprecated addressTaken 1/2/2007
                    if (isSgArrayType(type)) {
                        child_mre->setAddressTaken(false);

                    // If is a reference, then we need to apply reference
                    // conversion rules.  
                    } else if (isReferenceTypeRequiringModeling(getBaseType(type))) {
                    */
                   
                    
                    if(isSgArrayType(type)) {
                        if(child_mre->isaRefOp()) {
                            OA::OA_ptr<OA::RefOp> refOp = child_mre.convert<OA::RefOp>();
                            ROSE_ASSERT(!refOp.ptrEqual(0));
                            if(refOp->isaAddressOf()) {
                               mMemref2mreSetMap[child_memref].erase(child_mre);
                               child_mre = refOp->getMemRefExpr();
                               mMemref2mreSetMap[child_memref].insert(child_mre);
                            }
                        } else {
                            assert(0);
                        }
                        
                    } else if (isReferenceTypeRequiringModeling(getBaseType(type))) {

                        
                        // If the rhs has an lvalue, then set the 
                        // addressOf flag for the MREs for the node under 
                        // the SgInitializer node.  i.e., apply
                        // reference conversion rule 4 to the rhs.
                        // If the rhs does not have an lvalue, then
                        // apply reference conversion 3 to the _lhs_.
                        if ( is_lval(child_mre) ) {
                           
                            // Apply reference conversion rule 4-- 
                            // take the address of the rhs.  
                        
                            // an address taken will cancel out a deref and
                            // return the result
                            mMemref2mreSetMap[child_memref].erase(child_mre);


                            OA::OA_ptr<OA::AddressOf> address_mre;
                            OA::OA_ptr<OA::MemRefExpr> nullMRE;
                            
                            address_mre = new OA::AddressOf(
                                                  OA::MemRefExpr::USE,
                                                  nullMRE);

                            
                            child_mre = address_mre->composeWith(child_mre);

                            mMemref2mreSetMap[child_memref].insert(child_mre);

                            // We also need to be careful to remove the
                            // original reference base, lest we get two of
                            // them.  This was inserted as the Sg_File_Info
                            // of the deref'ed MRE.  XXX:  Ugly.  Encapsulate.
			    //                            SgNode *node = (SgNode*)(child_memref.hval());
                            SgNode *node = getNodePtr(child_memref);
                            ROSE_ASSERT(node != NULL);
                 
                            Sg_File_Info *fileInfo = node->get_file_info();
                            ROSE_ASSERT(fileInfo != NULL);

             			    OA::MemRefHandle hiddenMemref = getMemRefHandle(fileInfo);

                            // Need to erase the mre w/o the deref.
                            mMemref2mreSetMap[hiddenMemref].erase(child_mre);

                            // Model the reference initialization as
                            // a pointer assignment.

                            makePtrAssignPair(stmt, mre, child_mre);
                        } else {

                            // Apply reference conversion rules 2 and 4.
                            OA::OA_ptr<OA::MemRefExpr> addr_of_lhs_tmp_mre;
                            addr_of_lhs_tmp_mre =
                                applyReferenceConversionRules2And4(stmt,
                                                                   type,
                                                                   initName,
                                                                   initName->get_initptr(),
                                                                   child_memref,
                                                                   child_mre);

                            // Now model the reference initialization 
                            //    t_l &lhs = lhsTmp;             
                            // as the pointer assignment
                            //    t_l *lhs = &lhsTmp;
                            makePtrAssignPair(stmt, mre, addr_of_lhs_tmp_mre);
                        }
                    // Not an array or reference, if the lhs is 
                    // a pointer, we need to create a ptr assign.
                    } else if ( isSgPointerType(type) ) {
                        makePtrAssignPair(stmt, mre, child_mre);
                    }
                }
                
                if ( found_assignment == true ) {
                    makeAssignPair(stmt, memref, child_memref);
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

            OA::ExprHandle exprHandle = findTopExprHandle(astNode);
            if ( isSgExpression(astNode) ) {
                 mre = new OA::UnnamedRef(OA::MemRefExpr::USE, exprHandle);
            } else {
                 assert(0);
            }

            OA::OA_ptr<OA::SubSetRef> subset_mre;
            OA::OA_ptr<OA::MemRefExpr> nullMRE;
            OA::OA_ptr<OA::MemRefExpr> composed_mre;

            subset_mre = new OA::SubSetRef(
                                 OA::MemRefExpr::USE,
                                 nullMRE
                                );
            mre = subset_mre->composeWith(mre->clone());

            OA::OA_ptr<OA::AddressOf> address_mre;

            address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

            
            mre = address_mre->composeWith(mre);


            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[mre] = other ;

            mMemref2mreSetMap[memref].insert(mre);
            break;
        }
    case V_SgConstructorInitializer:
        { 
            SgConstructorInitializer *ctorInit =
                isSgConstructorInitializer(astNode);
            ROSE_ASSERT(ctorInit != NULL);

            // Get the declaration of the function.
            SgFunctionDeclaration *functionDeclaration = 
                ctorInit->get_declaration();

            // The declaration of the constructor initializer
            // may be NULL either if the constructor is
            // implicitly defined (and we have not done
            // AST normalization) or if this is a base type.
            // Tolerate NULL for the latter reason.
            if (functionDeclaration == NULL) {
                // If the class declaration associated with this
                // method is NULL, then it looks like a basic type.
                ROSE_ASSERT(ctorInit->get_class_decl() == NULL);
	    } else {

                OA::SymHandle symHandle = getProcSymHandle(functionDeclaration);
        
                OA::OA_ptr<OA::MemRefExpr> method;

                method = new OA::NamedRef(OA::MemRefExpr::USE,
                                          symHandle);

    
                // Record the type of the MRE (reference or non-reference).
                mMre2TypeMap[method] = other;
    
                OA::CallHandle call = getCallHandle(astNode);
#if 0
		//		std::cout << "Adding call handle for " << (int)(call.hval()) << std::endl;
		SgNode *node = getNodePtr(call);
		ROSE_ASSERT(node != NULL);
		std::cout << "call handle node (h: " << (int)(call.hval()) << " type: " << node->sage_class_name()
                  << ") " << node->unparseToString() 
                  << std::endl;
		node->get_file_info()->display("call handle: ");
#endif
                mCallToMRE[call] = method;
                OA::MemRefHandle memref = getMemRefHandle(astNode);
                mStmt2allMemRefsMap[stmt].insert(memref);
                mMemref2mreSetMap[memref].insert(method);

                // Create MREs for the actuals passed to the constructor.
    
                // Get the list of actual arguments from the constructor call.
                SgExprListExp* exprListExp = ctorInit->get_args();  
                ROSE_ASSERT (exprListExp != NULL);  
    
                // Notice that since we visit the SgExprListExp and since 
                // that case visits its sub-expressions, we do not 
                // need to explicitly visit the actual args here.
                findAllMemRefsAndPtrAssigns(exprListExp, stmt);
                
                // Note that we use the SgExprList of the constructor
                // to model the actual 'this' argument.  
                OA::MemRefHandle receiver_memref = getMemRefHandle(exprListExp);
                mStmt2allMemRefsMap[stmt].insert(receiver_memref);
                OA::OA_ptr<OA::MemRefExpr> receiver_mre
                    = createConstructorInitializerReceiverMRE(ctorInit);
                mMemref2mreSetMap[receiver_memref].insert(receiver_mre);
    
                // Create parameter binding params arising from
                // the actual arguments and receiver (objected created)
                // of the constructor invocation.
                // This also applies reference conversion rules
                // 2 and 4, as required.
                createParamBindPtrAssignPairs(stmt, ctorInit);
            }

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
    case V_SgDotExp:
        {
            SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
            ROSE_ASSERT(binaryOp != NULL);

            // recurse on lhs and rhs
            findAllMemRefsAndPtrAssigns(binaryOp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(binaryOp->get_rhs_operand(),stmt);
            
            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // clones MREs for lhs and wraps them in a Deref and a FieldAccess
            OA::MemRefHandle lhs_memref 
                = findTopMemRefHandle(binaryOp->get_lhs_operand());
            OA::MemRefHandle rhs_memref 
                = findTopMemRefHandle(binaryOp->get_rhs_operand());


	    //            std::string field_name = findFieldName(rhs_memref);
	    std::string field_name = toStringWithoutScope(binaryOp->get_rhs_operand());

            bool isVirtualInvocation = false;
            bool isMethodInvocation = false;
            SgMemberFunctionRefExp *memberFunctionRefExp =
	      isSgMemberFunctionRefExp(binaryOp->get_rhs_operand());
            if (memberFunctionRefExp != NULL) {
                SgMemberFunctionDeclaration *methodDecl =
                    getInvokedMethod(memberFunctionRefExp);
                ROSE_ASSERT(methodDecl != NULL);
		
                isVirtualInvocation = !memberFunctionRefExp->get_need_qualifier() && isVirtual(methodDecl);
		
                if ( isVirtualInvocation ) {
                    // According to section r.12.7 of 
                    // Stroustrup 2nd edition, calls to virtual methods
                    // within a constructor or destructor may be
                    // statically resolved.
                    SgFunctionDefinition *funcDefn =
                        getEnclosingFunction(astNode);
                    SgMemberFunctionDeclaration *decl =
                        isSgMemberFunctionDeclaration(funcDefn->get_declaration());
                
                    if ( ( decl != NULL ) &&
                         ( isConstructor(decl) || isDestructor(decl) ) ) {
                        isVirtualInvocation = false;
                    }
                }

		//		std::cout << "isVirtual: " << isVirtualInvocation << " qualify: " << memberFunctionRefExp->get_need_qualifier() << " get_virt: " << memberFunctionRefExp->get_virtual_call() << " " << binaryOp->unparseToString() << std::endl;
                // Even if we invoke a virtual method, it is 
                // statically resolvable if the receiver
                // is an object, rather than a reference.
                SgType *type = binaryOp->get_lhs_operand()->get_type();
                ROSE_ASSERT(type != NULL);
                if ( !isArrowExp(binaryOp) && !isSgReferenceType(type) ) {
                    isVirtualInvocation = false;
                }
                isMethodInvocation = true;
            }

            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(lhs_memref);
            OA::OA_ptr<OA::MemRefExprIterator> mRhsIter
                = getMemRefExprIterator(rhs_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();
                
                OA::OA_ptr<OA::MemRefExpr> memberAccess;
                if ( mUseVtableOpt && isVirtualInvocation ) {

                    // If we are using the vtable optimization, we don't
                    // return receiver->"method" for a virtual method 
                    // invocation, but
                    // (*(*receiver).FieldHandle(OA_VTABLE_STR))."method".

                    if ( isSgArrowExp(binaryOp) ) {
#if 1
		                lhs_mre = derefMre(lhs_mre, OA::MemRefExpr::USE, 1);
#else 
                        lhs_mre = new OA::Deref(false,
                                                lhs_mre->hasFullAccuracy(),
                                                OA::MemRefExpr::USE,
                                                lhs_mre,
                                                1);
#endif
                        // Record the type of the MRE (reference or non-reference).
                        mMre2TypeMap[lhs_mre] = other;
                    }


                    OA::OA_ptr<OA::MemRefExpr> fieldAccess;

                    fieldAccess = new OA::FieldAccess(OA::MemRefExpr::USE,
                                                      lhs_mre,
                                                      OA_VTABLE_STR);
                    
                    if(lhs_mre->isaRefOp()) {
               
                      OA::OA_ptr<OA::RefOp> refOp;
                      refOp = lhs_mre.convert<OA::RefOp>();

                      if(refOp->isaSubSetRef()) {

                         OA::OA_ptr<OA::SubSetRef> subset_mre;
                         OA::OA_ptr<OA::MemRefExpr> nullMRE;
                         OA::OA_ptr<OA::MemRefExpr> composed_mre;

                         subset_mre = new OA::SubSetRef(
                                             OA::MemRefExpr::USE,
                                             nullMRE
                                     );
                         
                         composed_mre 
                            = subset_mre->composeWith(fieldAccess->clone());

                         fieldAccess = composed_mre->clone();
                      }
                    } 

                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[fieldAccess] = other;

                    lhs_mre = fieldAccess;

                }

                if ( ( mUseVtableOpt && isVirtualInvocation ) ||
                     ( !isMethodInvocation && isSgArrowExp(binaryOp) ) ) {

#if 1
        		    lhs_mre = derefMre(lhs_mre, OA::MemRefExpr::USE, 1);
#else
                    lhs_mre = new OA::Deref(false,
                                            lhs_mre->hasFullAccuracy(),
                                            OA::MemRefExpr::USE,
                                            lhs_mre,
                                            1);
#endif
                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[lhs_mre] = other;

                }

                if ( isVirtualInvocation || !isMethodInvocation ) {

                    // Virtual method invocations need to be represented
                    // via a FieldAccess, as do member variable accesses.
                    OA::OA_ptr<OA::MemRefExpr> fieldAccess;

                    fieldAccess = new OA::FieldAccess(OA::MemRefExpr::USE,
                                                      lhs_mre,
                                                      field_name);

                    if(lhs_mre->isaRefOp()) {

                      OA::OA_ptr<OA::RefOp> refOp;
                      refOp = lhs_mre.convert<OA::RefOp>();

                      if(refOp->isaSubSetRef()) {

                         OA::OA_ptr<OA::SubSetRef> subset_mre;
                         OA::OA_ptr<OA::MemRefExpr> nullMRE;
                         OA::OA_ptr<OA::MemRefExpr> composed_mre;

                         subset_mre = new OA::SubSetRef(
                                             OA::MemRefExpr::USE,
                                             nullMRE
                                     );
                         fieldAccess
                            = subset_mre->composeWith(fieldAccess->clone());
                      }
                    }


                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[fieldAccess] = other;

                    memberAccess = fieldAccess;

                    // if the rhs is a variable with reference type then 
                    // Reference conversion rule 3.
                    if ( !isMethodInvocation ) {
                        int numRhses = 0;
                        OA::OA_ptr<OA::MemRefExpr> rhs_mre;
                        for (mRhsIter->reset(); mRhsIter->isValid();
                            ++(*mRhsIter) )
                        {
                            rhs_mre = mRhsIter->current();
                            ++numRhses;
                        }

                        ROSE_ASSERT(numRhses != 0);
                        ROSE_ASSERT(numRhses == 1);

                        if ( ( mMre2TypeMap.find(rhs_mre) != mMre2TypeMap.end() ) &&
                             ( mMre2TypeMap[rhs_mre] == reference ) ) {
                            // Change the type of the field access to a 
                            // reference.
                            // No, see below.
			    //                            mMre2TypeMap[fieldAccess] = reference;

                            // wrap the NamedRef in a Deref, but before doing so also
                            // create a MemRefHandle/MRE pair for the reference itself.
                            // Use the Sg_File_Info as the MemRefHandle.  This should
                            // effectively hide the MRE from anyone calling findTopMemRefHandle,
                            // e.g., to create ptr assign pairs.  That's good, because the
                            // deref should be the top memrefhandle.
                            Sg_File_Info *fileInfo = astNode->get_file_info();
                            ROSE_ASSERT(fileInfo != NULL);
	                    OA::MemRefHandle hiddenMemref = getMemRefHandle(fileInfo);
                            mStmt2allMemRefsMap[stmt].insert(hiddenMemref);
                            mMemref2mreSetMap[hiddenMemref].insert(fieldAccess);

                            // wrap the NamedRef in a Deref
                            int numderefs = 1;
#if 1
                            
                            memberAccess = derefMre(fieldAccess, OA::MemRefExpr::USE, numderefs);
#else
                            memberAccess = 
                                new OA::Deref(fieldAccess->hasFullAccuracy(),
                                              OA::MemRefExpr::USE,
                                              fieldAccess,
                                              numderefs);
#endif

 	  	            // It is the deref that we will see as the top mem ref handle
                            // and ask whether the access it reprsents is to a reference.
                            mMre2TypeMap[memberAccess] = reference;
                        }
                    }

                } else {

                    // Non-virtual method invocations are represented 
                    // by the NamedRef create for the rhs.  Clone it
                    // here, erase it below.  We only expect there to 
                    // be one rhs MRE.
                    int numRhses = 0;

                    for ( ; mRhsIter->isValid(); ++(*mRhsIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> rhs_mre = mRhsIter->current();
                        memberAccess = rhs_mre->clone();
                        // Transfer type to cloned MRE
                        mMre2TypeMap[memberAccess] = mMre2TypeMap[rhs_mre];
                        ++numRhses;
                    }
                    ROSE_ASSERT(numRhses != 0);
                    ROSE_ASSERT(numRhses == 1);

                }
                mMemref2mreSetMap[memref].insert(memberAccess);
            }

            if ( !isMethodInvocation && !isSgArrowExp(binaryOp) ) {
                mMemref2mreSetMap[lhs_memref].clear();
                mStmt2allMemRefsMap[stmt].erase(lhs_memref);
            } else if ( !isSgArrowExp(binaryOp) ) {
                // For a method invocation, we use the child/lhs receiver 
                // MRE as the implicit actual.  Therefore, not only do we
                // not remove it, but we need to take its address.
                OA::OA_ptr<OA::MemRefExprIterator> mLhsIter
                    = getMemRefExprIterator(lhs_memref);
                for ( ; mLhsIter->isValid(); ++(*mLhsIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> lhs_mre = mLhsIter->current();
                    mMemref2mreSetMap[lhs_memref].erase(lhs_mre);
                    
                    OA::OA_ptr<OA::AddressOf> address_mre;
                    OA::OA_ptr<OA::MemRefExpr> nullMRE;

                    address_mre = new OA::AddressOf(
                                          OA::MemRefExpr::USE,
                                          nullMRE);
                    
                    lhs_mre = address_mre->composeWith(lhs_mre);

                    mMemref2mreSetMap[lhs_memref].insert(lhs_mre);
		    //                    lhs_mre->setAddressTaken(true);
                }
            }

            // make rhs not a MemRefHandle
            // Note that lhs continues to be a MemRefHandle.

            mMemref2mreSetMap[rhs_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(rhs_memref);

            // We also need to be careful to remove the
            // original reference base at the rhs, if one exists.
            // If it does, it will be the Sg_File_Info of the rhs_memref.
	    //            SgNode *node = (SgNode*)(rhs_memref.hval());

            SgNode *node = getNodePtr(rhs_memref);
            ROSE_ASSERT(node != NULL);
                 
            Sg_File_Info *fileInfo = node->get_file_info();
            ROSE_ASSERT(fileInfo != NULL);

	    OA::MemRefHandle hiddenMemref = getMemRefHandle(fileInfo);
            mMemref2mreSetMap[hiddenMemref].clear();
            mStmt2allMemRefsMap[stmt].erase(hiddenMemref);
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
            
            // if either of the children are array types or ptr types
            // then this node should be a MemRefHandle
            // because a[2] is represented as a+2 in Sage
            // if either of the children are pointer types
            // then this node should be a MemRefHandle due to 
            // ptr arithmetic
            SgType *lhs_type, *rhs_type;
            lhs_type = getBaseType(binaryOp->get_lhs_operand()->get_type());
            rhs_type = getBaseType(binaryOp->get_rhs_operand()->get_type());
            bool rhs_ptr = false;  bool lhs_ptr = false;
            lhs_ptr = isSgArrayType(lhs_type)||isSgPointerType(lhs_type);
            rhs_ptr = isSgArrayType(rhs_type)||isSgPointerType(rhs_type);

            // Only one of the children should be of pointer or array type.
            // If both are then do not make this node a pointer
            // arithmetic MemRefHandle.
            if (lhs_ptr && !rhs_ptr) {
                createMemRefExprsForPtrArith(binaryOp, 
                                         binaryOp->get_lhs_operand(), stmt,
                                         true);
            } else if ( !lhs_ptr && rhs_ptr ) {
                createMemRefExprsForPtrArith(binaryOp, 
                                         binaryOp->get_rhs_operand(), stmt,
                                         true);
            }
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
    case V_SgCommaOpExp:
        {
            SgBinaryOp *binaryOp = isSgBinaryOp(astNode);
            ROSE_ASSERT(binaryOp != NULL);

            // recurse on lhs and rhs
            findAllMemRefsAndPtrAssigns(binaryOp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(binaryOp->get_rhs_operand(),stmt);
            
            // is a MemRefHandle
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            mStmt2allMemRefsMap[stmt].insert(memref);

            // takes MREs from rhs 
            OA::MemRefHandle rhs_memref 
                = findTopMemRefHandle(binaryOp->get_rhs_operand());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(rhs_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> rhs_mre = mIter->current();
                mMemref2mreSetMap[memref].insert(rhs_mre);
            }

            // make rhs not a MemRefHandle
            mMemref2mreSetMap[rhs_memref].clear();
            mStmt2allMemRefsMap[stmt].erase(rhs_memref);
            break;
        }
    case V_SgPntrArrRefExp:
        {
            // Note:  can not have arrays of references, otherwise
            // we would need to apply reference conversion rule 3 here.
            SgPntrArrRefExp *arrRefExp = isSgPntrArrRefExp(astNode);
            OA::MemRefHandle memref = getMemRefHandle(astNode);
            ROSE_ASSERT(arrRefExp != NULL);

            // recurse on children
            findAllMemRefsAndPtrAssigns(arrRefExp->get_lhs_operand(), stmt);
            findAllMemRefsAndPtrAssigns(arrRefExp->get_rhs_operand(), stmt); 

            // if through a constant pointer
            if(!isSgPointerType(getBaseType(
                            arrRefExp->get_lhs_operand()->get_type()))) 
            {
                // take the lhs MREs and make each of them have partial
                // accuracy and assign them to this MemRefHandle -make lhs not
                // a MemRefHandle
                OA::MemRefHandle lhs_memref 
                    = findTopMemRefHandle(arrRefExp->get_lhs_operand());

                OA::OA_ptr<OA::MemRefExpr> lhs_mre;
                OA::OA_ptr<OA::MemRefExprIterator> mIter
                    = getMemRefExprIterator(lhs_memref);
                for ( ; mIter->isValid(); ++(*mIter) ) {
                    lhs_mre = mIter->current();

                    OA::OA_ptr<OA::SubSetRef> subset_mre;
                    OA::OA_ptr<OA::MemRefExpr> nullMRE;
                    OA::OA_ptr<OA::MemRefExpr> composed_mre;

                    subset_mre = new OA::SubSetRef(
                                           OA::MemRefExpr::USE,
                                           nullMRE
                                     );
                    lhs_mre
                          = subset_mre->composeWith(lhs_mre->clone());


                    mMemref2mreSetMap[memref].insert(lhs_mre);
                }
                // child is no longer a MemRefHandle
                mMemref2mreSetMap[lhs_memref].clear();
                mStmt2allMemRefsMap[stmt].erase(lhs_memref);

                // take the rhs MRE and wrap it in an IdxExprAccess
                MemRefHandle mref =
                    ((OA::irhandle_t)arrRefExp->get_rhs_operand());
                OA_ptr<IdxExprAccess> rhs_mre;
                rhs_mre = new OA::IdxExprAccess(
                    MemRefExpr::USE, lhs_mre, mref);
                mStmt2allMemRefsMap[stmt].insert(mref);
                mMemref2mreSetMap[memref].insert(rhs_mre);
            }
            // else if through a variable pointer
            else {
                // clone the lhs MREs and make each clone have partial accuracy
                // and assign them to this MemRefHandle
                OA::MemRefHandle lhs_memref 
                    = findTopMemRefHandle(arrRefExp->get_lhs_operand());

                OA::OA_ptr<OA::MemRefExprIterator> mIter
                    = getMemRefExprIterator(lhs_memref);
                for ( ; mIter->isValid(); ++(*mIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();

                    OA::OA_ptr<OA::Deref> deref_mre;
                    int numDerefs = 1;
                    OA::OA_ptr<OA::MemRefExpr> nullMRE;

                    deref_mre = new OA::Deref( OA::MemRefExpr::USE,
                                               nullMRE,
                                               numDerefs);

                    OA::OA_ptr<OA::MemRefExpr> mre;
                    
                    mre = deref_mre->composeWith(lhs_mre->clone());

                    if(lhs_mre->isaRefOp()) {

                       OA::OA_ptr<OA::RefOp> refOp;
                       refOp = lhs_mre.convert<OA::RefOp>();

                       if(refOp->isaSubSetRef()) {
                          OA::OA_ptr<OA::SubSetRef> subset_mre;
                          OA::OA_ptr<OA::MemRefExpr> nullMRE;
                          OA::OA_ptr<OA::MemRefExpr> composed_mre;

                          subset_mre = new OA::SubSetRef(
                                             OA::MemRefExpr::USE,
                                             nullMRE
                                     );
                          mre
                            = subset_mre->composeWith(mre->clone());

                       }
                    }

                    OA::OA_ptr<OA::SubSetRef> subset_mre;
                    OA::OA_ptr<OA::MemRefExpr> composed_mre;

                    subset_mre = new OA::SubSetRef(
                                             OA::MemRefExpr::USE,
                                             nullMRE
                                     );
                    mre = subset_mre->composeWith(mre->clone());

                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[mre] = other;

                    mMemref2mreSetMap[memref].insert(mre);
                }
            }

            mStmt2allMemRefsMap[stmt].insert(memref);
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

                // Transfer type to cloned MRE
                mMre2TypeMap[mre] = mMre2TypeMap[lhs_mre];

                // FIXME
                // Needs to fix when AssignmentPair problem is solved.
                // Temporarily Fixed: Michelle Suggested to consider
                // Exposed Uses here. 
                // PLM 1/4/2007
 
                mre->setMemRefType(OA::MemRefExpr::USE);
                mMemref2mreSetMap[memref].insert(mre);

                // 2) change to a define
		lhs_mre->setMemRefType(OA::MemRefExpr::DEF);
            }

            OA::MemRefHandle rhs_memref 
                = findTopMemRefHandle(assignOp->get_rhs_operand());
            // if the rhs is a SgFunctionRefExpr then we are assigning
            // a function to a function ptr and we need to set
            // the address of for the rhs MREs
            if (isSgFunctionRefExp(getSgNode(rhs_memref))) {
                mIter = getMemRefExprIterator(rhs_memref);
                for ( ; mIter->isValid(); ++(*mIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> rhs_mre = mIter->current();
                    mMemref2mreSetMap[rhs_memref].erase(rhs_mre);
                    
                    OA::OA_ptr<OA::AddressOf> address_mre;
                    OA::OA_ptr<OA::MemRefExpr> nullMRE;

                    address_mre = new OA::AddressOf(
                                          OA::MemRefExpr::USE,
                                          nullMRE);
                    
                    rhs_mre = address_mre->composeWith(rhs_mre);

                    mMemref2mreSetMap[rhs_memref].insert(rhs_mre);
		    //                    rhs_mre->setAddressTaken(true);
                }
            }

            //----------- Ptr Assigns
            // FIXME: some of this logic might be useful for initializations
            // rhs top memref can only have an lval or pointer val if it is
            // NOT MemRefHandle(0)
            SgExpression* lhs_expr 
                = isSgExpression(assignOp->get_lhs_operand());
            SgType *lhs_type = getBaseType(lhs_expr->get_type());

            // Create an assignment pair.  We may also have to create
            // a pointer assignment pair (on a per-MRE basis).
            OA::ExprHandle rhs_expr = findTopExprHandle(assignOp->get_rhs_operand());
            makeAssignPair(stmt, lhs_memref, rhs_expr);

            if (rhs_memref!=OA::MemRefHandle(0))  {
                
                mIter = getMemRefExprIterator(lhs_memref);
                for ( ; mIter->isValid(); ++(*mIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> lhs_mre = mIter->current();

                    OA::OA_ptr<OA::MemRefExprIterator> mRhsIter;
                    mRhsIter = getMemRefExprIterator(rhs_memref);
                    for ( ; mRhsIter->isValid(); ++(*mRhsIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> rhs_mre = mRhsIter->current();
                        if (isSgPointerType(lhs_type)) {
                            makePtrAssignPair(stmt, lhs_mre, rhs_mre);
#if 1
                        } else if ( isSgReferenceType(lhs_type) ) {
                          // Remember if lhs is a reference, then
                          //    lhs = rhs;
                          // is effectively
                          //    *lhs = *rhs;
                          // i.e., this is only a pointer assignment if
                          // the base of the reference is a reference
                          // (couldn't be) or a pointer.
                          SgReferenceType *lhs_reference = 
                              isSgReferenceType(lhs_type);
                          ROSE_ASSERT(lhs_reference != NULL);
                          SgType *base = getBaseType(lhs_reference->get_base_type());
                          ROSE_ASSERT(base != NULL);
                          if ( isSgPointerType(base) || isSgReferenceType(base) ) {
                              makePtrAssignPair(stmt, lhs_mre, rhs_mre);
                          }
                        }
#else
                        // if top mem ref for lhs is a reference type and rhs has lval
                        } else if ( ( mMre2TypeMap[lhs_mre] == reference ) && is_lval(rhs_mre)) {
                            // remove the deref from the lhs, 
                            // by default references have deref put on assuming a use
                            mMemref2mreSetMap[lhs_memref].erase(lhs_mre);
                            // an address taken will cancel out a deref and
                            // return the result
                            lhs_mre = lhs_mre->setAddressTaken();
			    lhs_mre->setMemRefType(OA::MemRefExpr::DEF);
                            mMemref2mreSetMap[lhs_memref].insert(lhs_mre);
                            makePtrAssignPair(stmt, lhs_mre, rhs_mre);
                        }
#endif
                    }
                }
#if 0
                bool ptr_assign_found = false;
                // if the top mem ref for the lhs_operand is a pointer type
                if (isSgPointerType(lhs_type)) {
                    ptr_assign_found = true;
                // if top mem ref for lhs is a reference type and rhs has lval
                } else if (isReferenceTypeRequiringModeling(lhs_type) && is_lval(rhs_memref)) {
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
			lhs_mre->setMemRefType(OA::MemRefExpr::DEF);
                        mMemref2mreSetMap[lhs_memref].insert(lhs_mre);
                    }
#if 0
	 	    // removed by BSW 9/12/06
                    // set the addressOf for rhs as well}
                    mIter = getMemRefExprIterator(rhs_memref);
                    for ( ; mIter->isValid(); ++(*mIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> rhs_mre = mIter->current();
                        mMemref2mreSetMap[rhs_memref].erase(rhs_mre);
                        rhs_mre = rhs_mre->setAddressTaken();
                        mMemref2mreSetMap[rhs_memref].insert(rhs_mre);
                    }
#endif
                }
                // if a ptr assignment was found then make pairs
                if (ptr_assign_found) {
                    makePtrAssignPair(stmt, lhs_memref, rhs_memref);
                }
#endif

            }
            break;
        }
    case V_SgPlusAssignOp:
    case V_SgMinusAssignOp:
        {
            SgBinaryOp* binaryOp = isSgBinaryOp(astNode);
            ROSE_ASSERT(binaryOp);

            // recurse on children
            findAllMemRefsAndPtrAssigns(binaryOp->get_lhs_operand(),stmt);
            findAllMemRefsAndPtrAssigns(binaryOp->get_rhs_operand(),stmt);

            // if the lhs is an array type or ptr type
            // then this node should be a MemRefHandle
            // because a[2] is represented as a+2 in Sage
            // if the operand is a pointer type
            // then this node should be a MemRefHandle due to 
            // ptr arithmetic
            bool removeChild = false;
            bool nodeHasMemRefHandle =            
                createMemRefExprsForPtrArith(binaryOp, 
                                             binaryOp->get_lhs_operand(), 
                                             stmt,
                                             removeChild);

            OA::MemRefHandle memref = getMemRefHandle(astNode);
            if ( nodeHasMemRefHandle ) {
                // If this is node performs ptr arithmetic,
                // a node representing that arithmetic will have
                // been created at astNode by createMemRefExprsForPtrArith.
                // Since removeChild = false, the child will still
                // exist and represents the (implicit) left-hand side.
                // So, we should create ptr assignment pairs, with
                // the child (marked as a DEF) as the left-hand side 
                // and the MREs at this node as the right-hand side.

                OA::MemRefHandle child_memref 
                    = findTopMemRefHandle(binaryOp->get_lhs_operand());
                OA::OA_ptr<OA::MemRefExprIterator> childIter
                    = getMemRefExprIterator(child_memref);
                OA::OA_ptr<OA::MemRefExprIterator> iter
                    = getMemRefExprIterator(memref);
                for ( ; childIter->isValid(); ++(*childIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> child_mre = childIter->current();
                    child_mre->setMemRefType(OA::MemRefExpr::USEDEF);
              
                    for (iter->reset(); iter->isValid(); ++(*iter) ) {
                        OA::OA_ptr<OA::MemRefExpr> mre = iter->current();
                        makePtrAssignPair(stmt, child_mre, mre);
                    }
                }

            } else {
                // Actually, this node should always have a MemRefHandle.
                // If it wasn't created in createMemRefExprsForPtrArith,
                // do it here.  In this case, it is not a ptr assignment
                // pair, but a non-ptr assign pair.  As above, make
                // the child the lhs/DEF.  Clone that child (as a USE)
                // for the rhs and place it on this node.
                mStmt2allMemRefsMap[stmt].insert(memref);

                OA::MemRefHandle child_memref 
                    = findTopMemRefHandle(binaryOp->get_lhs_operand());
                OA::OA_ptr<OA::MemRefExprIterator> childIter
                    = getMemRefExprIterator(child_memref);
                OA::OA_ptr<OA::MemRefExprIterator> iter
                    = getMemRefExprIterator(memref);
                for ( ; childIter->isValid(); ++(*childIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> child_mre = childIter->current();

                    OA::OA_ptr<OA::MemRefExpr> mre = child_mre->clone();

                    // Transfer type to cloned MRE
                    mMre2TypeMap[mre] = mMre2TypeMap[child_mre];

                    mre->setMemRefType(OA::MemRefExpr::USE);
                    mMemref2mreSetMap[memref].insert(mre);

                    child_mre->setMemRefType(OA::MemRefExpr::USEDEF);
                }
                makeAssignPair(stmt, child_memref, memref);
            }

            break;
        }
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

                // Transfer type to cloned MRE
                mMre2TypeMap[mre] = mMre2TypeMap[lhs_mre];

                mre->setMemRefType(OA::MemRefExpr::USE);
                mMemref2mreSetMap[memref].insert(mre);
                lhs_mre->setMemRefType(OA::MemRefExpr::USEDEF);
            }
            makeAssignPair(stmt, lhs_memref, memref);
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
                int numDerefs = 1;
                OA::OA_ptr<OA::MemRefExpr> nullMRE;

                OA::OA_ptr<OA::MemRefExpr> composed_mre;
                deref_mre = new OA::Deref( OA::MemRefExpr::USE,
                                           nullMRE,
                                           numDerefs);

                OA::OA_ptr<OA::MemRefExpr> composedMre
                      = deref_mre->composeWith(child_mre);


                if( child_mre->isaRefOp() ){

                  OA::OA_ptr<OA::RefOp> refOp;
                  refOp = child_mre.convert<OA::RefOp>();
  
                  if(refOp->isaSubSetRef()) { 
                     OA::OA_ptr<OA::SubSetRef> subset_mre;
                     OA::OA_ptr<OA::MemRefExpr> nullMRE;
                     OA::OA_ptr<OA::MemRefExpr> composed_mre;

                     subset_mre = new OA::SubSetRef(
                                             OA::MemRefExpr::USE,
                                             nullMRE
                                     );
                     composedMre
                         = subset_mre->composeWith(composedMre->clone());
                  }
                }

                // use composeWith so that any necessary canonicalization occurs

                // Transfer type to composed MRE
    	        mMre2TypeMap[composedMre] = other;

                mMemref2mreSetMap[memref].insert(composedMre);
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
                

                OA::OA_ptr<OA::AddressOf> address_mre;
                OA::OA_ptr<OA::MemRefExpr> nullMRE;
                
                address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

                
                child_mre = address_mre->composeWith(child_mre);

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

            // if the operand is an array type or ptr type
            // then this node should be a MemRefHandle
            // because a[2] is represented as a+2 in Sage
            // if the operand is a pointer type
            // then this node should be a MemRefHandle due to 
            // ptr arithmetic
            bool removeChild = false;
            bool nodeHasMemRefHandle =            
                createMemRefExprsForPtrArith(unaryOp, 
                                             unaryOp->get_operand(), 
                                             stmt,
                                             removeChild);

            OA::MemRefHandle memref = getMemRefHandle(astNode);
            if ( nodeHasMemRefHandle ) {
                // If this is node performs ptr arithmetic,
                // a node representing that arithmetic will have
                // been created at astNode by createMemRefExprsForPtrArith.
                // Since removeChild = false, the child will still
                // exist and represents the (implicit) left-hand side.
                // So, we should create ptr assignment pairs, with
                // the child (marked as a DEF) as the left-hand side 
                // and the MREs at this node as the right-hand side.

                OA::MemRefHandle child_memref 
                    = findTopMemRefHandle(unaryOp->get_operand());
                OA::OA_ptr<OA::MemRefExprIterator> childIter
                    = getMemRefExprIterator(child_memref);
                OA::OA_ptr<OA::MemRefExprIterator> iter
                    = getMemRefExprIterator(memref);
                for ( ; childIter->isValid(); ++(*childIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> child_mre = childIter->current();
                    child_mre->setMemRefType(OA::MemRefExpr::USEDEF);
              
                    for (iter->reset(); iter->isValid(); ++(*iter) ) {
                        OA::OA_ptr<OA::MemRefExpr> mre = iter->current();
                        makePtrAssignPair(stmt, child_mre, mre);
                    }
                }

            } else {
                // Actually, this node should always have a MemRefHandle.
                // If it wasn't created in createMemRefExprsForPtrArith,
                // do it here.  In this case, it is not a ptr assignment
                // pair, but a non-ptr assign pair.  As above, make
                // the child the lhs/DEF.  Clone that child (as a USE)
                // for the rhs and place it on this node.
                mStmt2allMemRefsMap[stmt].insert(memref);

                OA::MemRefHandle child_memref 
                    = findTopMemRefHandle(unaryOp->get_operand());
                OA::OA_ptr<OA::MemRefExprIterator> childIter
                    = getMemRefExprIterator(child_memref);
                OA::OA_ptr<OA::MemRefExprIterator> iter
                    = getMemRefExprIterator(memref);
                for ( ; childIter->isValid(); ++(*childIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> child_mre = childIter->current();

                    OA::OA_ptr<OA::MemRefExpr> mre = child_mre->clone();

                    // Transfer type to cloned MRE
                    mMre2TypeMap[mre] = mMre2TypeMap[child_mre];

                    mre->setMemRefType(OA::MemRefExpr::USE);
                    mMemref2mreSetMap[memref].insert(mre);

                    child_mre->setMemRefType(OA::MemRefExpr::USEDEF);
              
                }
                makeAssignPair(stmt, child_memref, memref);
            }

#if 0
            // its child's MREs should be made USEDEF, 
            // and it should have a similar set of MREs with USE
            OA::MemRefHandle child_memref 
                = findTopMemRefHandle(unaryOp->get_operand());
            OA::OA_ptr<OA::MemRefExprIterator> mIter
                = getMemRefExprIterator(child_memref);
            for ( ; mIter->isValid(); ++(*mIter) ) {
                OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
                OA::OA_ptr<OA::MemRefExpr> mre = child_mre->clone();

                // Transfer type to cloned MRE
                mMre2TypeMap[mre] = mMre2TypeMap[child_mre];

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
#endif
            break;
        }

    // ---------------------------------------- Statement cases
    case V_SgExprStatement:
        {
            SgExprStatement *exprStatement = isSgExprStatement(astNode);
            ROSE_ASSERT(exprStatement!=NULL);
#ifdef ROSE_PRE_0_8_10A
            findAllMemRefsAndPtrAssigns(exprStatement->get_the_expr(),stmt);
#else
            findAllMemRefsAndPtrAssigns(exprStatement->get_expression(),stmt);
#endif

            // Remove Association of TopMemRefHandle 
            // in case of Assignment Statement
            // a = b + c

            // Need to check if Statement is AssignOp type because
            // we dont want to remove all the TopMemRefHandles of
            // SgExprStatement.
            // e.g.  if(x) or *a or const_array[i]            

#ifdef ROSE_PRE_0_8_10A
            SgBinaryOp *assignOp = isSgBinaryOp(exprStatement->get_the_expr());
#else
            SgBinaryOp *assignOp = isSgBinaryOp(exprStatement->get_expression());
#endif
            if(assignOp) {
#ifdef ROSE_PRE_0_8_10A
               OA::MemRefHandle child_memref
                       =
               findTopMemRefHandle(exprStatement->get_the_expr());
#else
               OA::MemRefHandle child_memref
                       =
               findTopMemRefHandle(exprStatement->get_expression());
#endif

               mMemref2mreSetMap[child_memref].clear();
               mStmt2allMemRefsMap[stmt].erase(child_memref);
            } 

            break;
        }
    //case V_SgCaseOptionStatement: // NOT in enum? spelled differently?
    //    {
    //        ROSE_ASSERT(0);
    //        break;
    //    }
    case V_SgTryStmt:
        {
            
            SgTryStmt *tryStmt = isSgTryStmt(astNode);
            ROSE_ASSERT(tryStmt != NULL);
            // Do not visit anything here.  The stmt iterator should 
            // visit the body and catch statements.
            // findAllMemRefsAndPtrAssigns(tryStmt->get_body(),stmt);
            // findAllMemRefsAndPtrAssigns(tryStmt->get_catch_statement_seq_root(),stmt);
            break;
        }
    case V_SgReturnStmt:
        {
            SgReturnStmt *returnStmt = isSgReturnStmt(astNode);
            ROSE_ASSERT(returnStmt != NULL);

            SgNode *retExpr;
#ifdef ROSE_PRE_0_8_10A
            retExpr = returnStmt->get_return_expr();
#else
            retExpr = returnStmt->get_expression();
#endif
            if ( retExpr != NULL ) {
                findAllMemRefsAndPtrAssigns(retExpr, stmt);
    
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
                if (isReferenceTypeRequiringModeling(return_type) || isSgPointerType(return_type)){
      
                    // Create the lhs to represent the method declaration.
                    OA::SymHandle symHandle = getProcSymHandle(functionDeclaration);
        
                    OA::OA_ptr<OA::MemRefExpr> function;

                    function = new OA::NamedRef(OA::MemRefExpr::DEF,
                                                symHandle);

    
                    // Record the type of the MRE (reference or non-reference).
                    mMre2TypeMap[function] = ( isSgReferenceType(return_type) ? reference : other );
    
                    // get the memory reference handle for the return exp
                    OA::MemRefHandle child_memref 

                        = findTopMemRefHandle(retExpr);
                    SgExpression *child_node=NULL;
                    SgType *child_type;
                    if (child_memref!=OA::MemRefHandle(0)) {
                        child_node = isSgExpression(getSgNode(child_memref));
                        ROSE_ASSERT(child_node);
                        child_type = getBaseType(child_node->get_type());
                    }
    
    
                    OA::OA_ptr<OA::MemRefExprIterator> mIter;
                    mIter = getMemRefExprIterator(child_memref);
                    for ( ; mIter->isValid(); ++(*mIter) ) {
                        OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
    
                        // if returning a reference then need to modify the MREs
                        // for the return expression
                        // FIXME: very similar to some code in SgInitializedName case
                        if (isReferenceTypeRequiringModeling(return_type)) {
    
                            // If the rhs has an lvalue, then set the 
                            // addressOf flag for the MREs for the node under 
                            // the SgInitializer node.  i.e., apply
                            // reference conversion rule 4 to the rhs.
                            // If the rhs does not have an lvalue, then
                            // apply reference conversion 3 to the _lhs_.
                            if ( is_lval(child_mre) ) {
                                // Apply reference conversion rule 4-- 
                                // take the address of the rhs.  But only
                                // do this in the context of the implicit
                                // ptr assign.  i.e., do not modify it 
                                // within the expression in the program.
                            
                                // an address taken will cancel out a deref and
                                // return the result
                                mMemref2mreSetMap[child_memref].erase(child_mre);

                                OA::OA_ptr<OA::AddressOf> address_mre;
                                OA::OA_ptr<OA::MemRefExpr> nullMRE;
                                
                                address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

                                child_mre = address_mre->composeWith(child_mre);

                                
                                mMemref2mreSetMap[child_memref].insert(child_mre);
    
                                // We also need to be careful to remove the
                                // original reference base, lest we get two of
                                // them.  This was inserted as the Sg_File_Info
                                // of the deref'ed MRE.  XXX:  Ugly.  Encapsulate.
				//                                SgNode *node = (SgNode*)(child_memref.hval());
                                SgNode *node = getNodePtr(child_memref);
                                ROSE_ASSERT(node != NULL);
                     
                                Sg_File_Info *fileInfo = node->get_file_info();
                                ROSE_ASSERT(fileInfo != NULL);
    
                			    OA::MemRefHandle hiddenMemref = getMemRefHandle(fileInfo);
    
                                // Need to erase the mre w/o the deref.
                                mMemref2mreSetMap[hiddenMemref].erase(child_mre);
    
                                // Model the reference initialization as
                                // a pointer assignment.
                                
                                makePtrAssignPair(stmt, function, child_mre);
                            } else {
                                // Apply reference conversion rules 2 and 4.
                                OA::OA_ptr<OA::MemRefExpr> addr_of_lhs_tmp_mre;
                                addr_of_lhs_tmp_mre =
                                    applyReferenceConversionRules2And4(stmt,
                                                                       return_type,
                                                                       returnStmt,
                                                                       child_node,
                                                                       child_memref,
                                                                       child_mre);
    
                                // Now model the reference initialization 
                                //    t_l &lhs = lhsTmp;             
                                // as the pointer assignment
                                //    t_l *lhs = &lhsTmp;

                                makePtrAssignPair(stmt, function, addr_of_lhs_tmp_mre);
                            }
                        // Else must be returning a SgPointerType.
                        } else {
                            ROSE_ASSERT(isSgPointerType(return_type));
                            makePtrAssignPair(stmt, function, child_mre);
                        }
                    }
                }
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
                SgInitializedName *lhs = *varIter;
                ROSE_ASSERT(lhs != NULL);
                findAllMemRefsAndPtrAssigns(lhs,stmt);

#if 1
                SgConstructorInitializer *ctorInitializer =
		  isSgConstructorInitializer(lhs->get_initptr());
                if ( ctorInitializer != NULL ) { 
                    // If a constructor is here, this is an object declaration.
                    // We need to create implicit ptr assigns for
                    // the virtual methods, which will involve a MRE
                    // for the newly-declared object.

                    // Get an MRE for it ..
                    OA::OA_ptr<OA::MemRefExpr> receiver_mre =
                        createConstructorInitializerReceiverMRE(ctorInitializer);
                    // But that returns a receiver intended as an
                    // implicit this-- i.e., with its address taken.
                    // We just want the object, so deref it.

                    /* deprecated addressTaken 1/2/2007
                    ROSE_ASSERT(receiver_mre->hasAddressTaken());
                    */

                    if(receiver_mre->isaRefOp()) {
                        OA::OA_ptr<OA::RefOp> refop = receiver_mre.convert<OA::RefOp>();
                        ROSE_ASSERT(refop->isaAddressOf());

                    } else {
                       assert(0);
                    }


                    OA::OA_ptr<OA::MemRefExpr> nullMRE;
                    OA::OA_ptr<OA::Deref> deref_mre;
                    int numDerefs = 1;

                    deref_mre = new OA::Deref(
                                              OA::MemRefExpr::USE,
                                              nullMRE,
                                              numDerefs);

                    
   		            receiver_mre = deref_mre->composeWith(receiver_mre);

                    mMre2TypeMap[receiver_mre] = other;

                    createImplicitPtrAssignPairsForObjectDeclaration(stmt,
                                                                     receiver_mre,
                                                                     lhs);

                }
#else
                // Retrieve any MREs create for a variable declaration.
                // Really, there should be at most one.  This should
                // have been created by the above call to 
                // findAllMemRefsAndPtrAssigns.
                OA::MemRefHandle var_memref 
                    = findTopMemRefHandle(lhs);
                OA::OA_ptr<OA::MemRefExprIterator> mIter
                    = getMemRefExprIterator(var_memref);

#if 1


                // For each MRE, determine if there are
                // implicit ptr assign pairs, which arise if this
                // is an object declaration.
                for ( ; mIter->isValid(); ++(*mIter) ) {
                    OA::OA_ptr<OA::MemRefExpr> mre = mIter->current();
                    createImplicitPtrAssignPairsForObjectDeclaration(stmt,
                                                                     mre,
                                                                     lhs);
                }
#endif
#endif
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
            
	  std::cout << "Got an ASM stmt!" << std::endl;
          astNode->get_file_info()->display("ASM stmt:");
	  std::cout << astNode->unparseToCompleteString();
          SgAsmStmt *asmStmt = isSgAsmStmt(astNode);
          ROSE_ASSERT(asmStmt != NULL);
          SgExpressionPtrList &expressions = asmStmt->get_operands();
          for (SgExpressionPtrList::iterator it = expressions.begin();
               it != expressions.end(); ++it) {
            SgExpression *expr = *it;
            ROSE_ASSERT(expr != NULL);
	    std::cout << "expr: " << expr->unparseToString() << std::endl;
          }
	  SgAsmStmt::AsmRegisterNameList &clobberList = asmStmt->get_clobberRegisterList();
          for (SgAsmStmt::AsmRegisterNameList::iterator it = clobberList.begin();
               it != clobberList.end(); ++it) {
	    SgInitializedName::asm_register_name_enum name = *it;
	    std::cout << "name: " << name << std::endl;
          }

	  //            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionParameterList:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgCtorInitializerList:
        {
            // FIXME: is this something that should only recurse
            // on children if this node is the stmt?
            SgCtorInitializerList *initializerList 
                = isSgCtorInitializerList(astNode);
            ROSE_ASSERT(initializerList != NULL);

            // recurse on children
            SgInitializedNamePtrList &list = initializerList->get_ctors();
            SgInitializedNamePtrList::iterator listIter;
            for (listIter=list.begin(); listIter!=list.end(); listIter++) {
                SgInitializedName* initName = *listIter;
                ROSE_ASSERT(initName != NULL);
                findAllMemRefsAndPtrAssigns(initName,stmt);
            }
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
#ifdef ROSE_PRE_0_8_10A
            SgExpression *incrExpr = forStatement->get_increment_expr();
#else
            SgExpression *incrExpr = forStatement->get_increment();
#endif
            if (incrExpr != NULL) {
                findAllMemRefsAndPtrAssigns( incrExpr, stmt );
            }
            break;
        }

    case V_SgForInitStatement: 
        {
            SgForInitStatement *initStmt = isSgForInitStatement(astNode);
            ROSE_ASSERT(initStmt != NULL);

            SgStatementPtrList &stmtList = initStmt->get_init_stmt();
            for (SgStatementPtrList::iterator it = stmtList.begin();
                 it != stmtList.end(); ++it) {
                SgStatement *statement = *it;
                ROSE_ASSERT(statement != NULL);

                // FIXME:  BSW, although init statements are statements
                // in their own right, I am associating them with
                // stmt, which corresponds to the SgForStatement.
                // Should be OK?
                findAllMemRefsAndPtrAssigns( statement, stmt );
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
        ROSE_ASSERT(0);
        break;

    case V_SgClassDeclaration:
        {

            
            SgClassDeclaration *classDecl = isSgClassDeclaration(astNode);
            ROSE_ASSERT(classDecl != NULL);
#if 0
	    classDecl = getDefiningDeclaration(classDecl);
	    ROSE_ASSERT(classDecl != NULL);
            findAllMemRefsAndPtrAssigns( classDecl->get_definition(), stmt );
#else
            classDecl = getDefiningDeclaration(classDecl);
            if ( ( classDecl ) && ( classDecl->get_definition() != NULL ) ) {
                findAllMemRefsAndPtrAssigns( classDecl->get_definition(), stmt );
            }
#endif
            break;
        }

    case V_SgClassDefinition:
        {
            
            SgClassDefinition *classDefn = isSgClassDefinition(astNode);
            ROSE_ASSERT(classDefn != NULL);

            // If we are using the virtual function table model,
            // we need to create implicit ptr assignments on a 
            // class definition.
     	    std::list<SgMemberFunctionDeclaration *> visitedVirtualMethods;
            createImplicitPtrAssignPairsForClassDefinition(stmt,
                                                           classDefn,
                                                           classDefn,
                                                           visitedVirtualMethods);
            break;
        }

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
            //bool addressTaken = true;

            // This does _not_ accurately represent the memory 
            // expression, as this would require the precise calling context.
            //bool accuracy = false;
            // default MemRefType, ancestors will change this if necessary
            OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
            // get the symbol for the string
            OA::OA_ptr<OA::MemRefExpr> mre;

            OA::ExprHandle exprHandle = findTopExprHandle(stringVal);
            if ( isSgExpression(stringVal) ) {
                 mre = new OA::UnnamedRef(mrType, exprHandle);
            } else {
                 assert(0);
            }

            OA::OA_ptr<OA::SubSetRef> subset_mre;
            OA::OA_ptr<OA::MemRefExpr> nullMRE;
            OA::OA_ptr<OA::MemRefExpr> composed_mre;

            subset_mre = new OA::SubSetRef(
                                           OA::MemRefExpr::USE,
                                           nullMRE
                                          );
            mre
                   = subset_mre->composeWith(mre->clone());

            
            OA::OA_ptr<OA::AddressOf> address_mre;

            address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

            mre = address_mre->composeWith(mre);
            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[mre] = other;
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
    case V_SgCatchStatementSeq:  
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
    case V_SgFunctionDeclaration:
    case V_SgNullExpression:
        break;

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
        {
            // if this node has a non-null operand then recurse on it
            // this is used in at least the sizeof case
            SgValueExp *valueExp = isSgValueExp(astNode);
            ROSE_ASSERT(valueExp!=NULL);
      
#ifdef ROSE_0_8_9a
            SgExpression* expr = valueExp->get_originalExpressionTree();
#else
            SgExpression* expr = valueExp->get_valueExpressionTree();
#endif
            if (expr!=NULL) {
                findAllMemRefsAndPtrAssigns(expr, stmt);
            }
            break;
        }
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
} // end findAllMemRefsAndPtrAssigns

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
#ifdef ROSE_0_8_9a      
            // One of these children may be a folded expression,
            // while the other is the original expression
            // (accessible via get_originalExpressionTree).
            // This may occur for SgValueExp (e.g., where the sizeof
            // operator is in the expression tree and has been folded)
            // and for SgCastExp.  Handle these two cases
            // explicitly.
            if ( isSgValueExp(astNode) ) {
                // SgValueExps should only have one child-- the
                // original expression tree.  Therefore, we should not
                // be here.
                ROSE_ABORT();
            } else if ( isSgCastExp(astNode) ) {
                // If some analysis/optimization has determined that it
                // was safe to fold an expression, then we should
                // ignore the original expression and just look at the
                // folded value.
                SgCastExp *castExp = isSgCastExp(astNode);
                ROSE_ASSERT(castExp != NULL);

                return findTopMemRefHandle(castExp->get_operand());
            }
#endif
            return getMemRefHandle(0);
        }
    }
}

//! finds the topmost ExprHandle in the subtree rooted at the given node
OA::ExprHandle SageIRInterface::findTopExprHandle(SgNode *astNode)
{
    if (astNode==NULL) {
        return OA::ExprHandle(0);
    }

    OA::ExprHandle expr = getMemRefHandle(astNode);
    if ( isSgExpression(astNode) ) {
        return expr;        
    }

    // determine the number of children the node has
    std::vector< SgNode * > kids = 
        astNode->get_traversalSuccessorContainer();

    // if only one child the return topExprHandle on that
    if (kids.size()==1) {
        return findTopExprHandle(kids[0]);
    }

    return getMemRefHandle(0);
}

#if 0
//! For a given MemRefHandle determines if the associated MREs 
//! are lvals.  MREs that have addressOf set are not lvals since
//! the indicate the computation of an address instead of describing
//! a location that has an address.
bool SageIRInterface::is_lval(OA::MemRefHandle memref) 
{
    ROSE_ABORT();
    // Should not be asking is_lval of a MemRefHandle, but rather of
    // an MRE.  Consider (where int *&rpInt is a reference to a ptr):
    //
    // int x = 5;
    // int *xPtr = &x;
    // int *&refPtr = xPtr;
    // int *&rpInt = ( cond ? refPtr : xPtr ); 
    // 
    // from the last stmt we should get two pointer assignments:
    // rpInt = refPtr;   // where rpInt and refPtr are modeled as int**
    // rpInt = &xPtr;    // where rpInt is modeled as int** and xPtr as int*
    // 
    // according to ROSE, the expressions above have the following type:
    //
    // expr xPtr has type: SgPointerType
    // expr refPtr has type: SgReferenceType
    // expr ((cond)?refPtr:xPtr) has type: SgReferenceType
    //
    // i.e., notice that the SgConditionalExp has a type that matches
    // only one of its sub-expressions -> we need to be looking
    // directly at the MREs for the subexpressions, not the
    // MemRefHandle for the expression (here the SgConditionalExp).

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
#endif

//! Determines if the MRE has an lval. 
//! MREs that have addressOf set are not lvals since
//! they indicate the computation of an address instead of describing
//! a location that has an address.
bool is_lval(OA::OA_ptr<OA::MemRefExpr> mre) 
{
    /* deprecated addressTaken 1/2/2007
    // This is not an lvalue if its address has been taken.
    if ( mre->hasAddressTaken() ) {
        return false;
    }
    */

    // This is not an lvalue if its address has been taken.
    if(mre->isaRefOp()) {
        OA::OA_ptr<OA::RefOp> refop = mre.convert<OA::RefOp>();
       if(refop->isaAddressOf()) {
           return false; 
       }
    } 
    
#if 0
    // Wrong!
    // Nor is it an lvalue if it is a formal parameter.
    if (mre->isaNamed()) {
        OA::OA_ptr<OA::NamedRef> named_mre = mre.convert<OA::NamedRef>();
	OA::SymHandle symHandle = named_mre->getSymHandle();

        // Ugly!  Unsafe!
        SgInitializedName *initName = isSgInitializedName(symHandle);
        ROSE_ASSERT(initName != NULL);

        bool isFormal = false;
        SgNode *parent = initName;
        while ( ( parent != NULL ) && ( !isSgGlobal(parent) ) ) {
            if ( isSgParameterList(parent) ) {
                isFormal = true;
                break;
            }
            parent = parent->get_parent();
        }
    }
#endif
    return true;
}

/** \brief Create implicit pointer assignment pairs to model
 *         virtual methods in classDefinition and its base classes.
 *  \param  stmt  The StmtHandle of the statement generating
 *                these implicit assignments.
 *  \param  lhsMRE  The MemRefExpr that will be used as the base
 *                  to create implicit pointer assignment pairs.
 *  \param  classDefinition  The class for which we need to 
 *                           model virtual methods.
 *  \param  visitedVirtualMethods  A list of virtual methods for
 *                                 which we have already created
 *                                 implicit pointer assignment
 *                                 pairs for with respect to the
 *                                 top-level invocation of this
 *                                 method on lhsMRE/classDefinition.
 *
 *  This method handles both the virtual function table model
 *  and the per-method model.
 *
 *  Virtual function table model:
 *  If classDefinition does not have any virtual methods
 *      recurse up its class hierarchy.
 *  Else
 *      Create an implicit ptr assignment pair < lhsMRE."vptr", &classDefn >
 *         where classDefn is a NamedRef representing the class.
 *        
 *  Per-method model:
 *  For each virtual method vm in classDefinition
 *      Create an implicit ptr assignment pair < lhsMRE."vm", &vm >
 *         where vm is a NamedRef representing the method.
 *  Recurse up classDefinition's class hierarchy (always).
 *
 *  NB:  We expect that lhs is already Deref'ed if it needs to be.
 */
void
SageIRInterface::
createImplicitPtrAssignPairsForVirtualMethods(OA::StmtHandle stmt,
                                              OA::OA_ptr<OA::MemRefExpr> lhsMRE,
                                              SgClassDefinition *classDefinition,
                                              std::list<SgMemberFunctionDeclaration *> &visitedVirtualMethods)
{
    // A lhs need not be a pointer.  We now create implicit ptr assignments
    // for object allocations, e.g., A a;.  While it is true that 
    // we can statically determine virtual methods invocations for 
    // objects (as opposed to pointers), we make take the address of
    // the object and assign it to a pointer.  A *b = &a.  
    // We need b to have access to these implicit ptr assignments.

    ROSE_ASSERT(classDefinition != NULL);

    if ( mUseVtableOpt ) {

        bool hasMethods = classHasVirtualMethods(classDefinition);
        if ( hasMethods == true ) {

            // Use the vtable optimization.  Rather than
            // create implicit ptr assigns for each method of an object a 
            // of class A, we create a single implicit assignment:
            // < a.FieldHandle(OA_VTABLE_STR), &A >
            // This is effectively a pointer to a (virtual) table,
            // though we create it whenever there are methods, virtual
            // or otherwise.
            // 1/15/06:  No longer true, now we just create it if there
            // are virtual methods.  Why were we ever creating for
            // non-virtual methods.

            // At each declaration of class A we create implicit pairs
            // for each method of the form:
            // < A.method, &A::method >
            // where the first MRE is a FieldAccess
            // and the second is a NamedRef.
            // Thus *((*(*a).FieldHandle(OA_VTABLE_STR)).method), as
            // returned by getCallMemRefExpr(), is aliased to *A.method 
            // is aliased to A::method
            // via FIAlias, which resolves the string-based FieldAccess to
            // the symbol-based NamedRef which unambiguously 
            // specifies the method.

            // Create the implicit ptr assignment.
            OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::DEF;
            string mangledMethodName = OA_VTABLE_STR;
	
            // NB:  we expect that lhsMRE was already Deref'ed
            //      before being passed here if it is a pointer.
            OA::OA_ptr<OA::FieldAccess> fieldAccess;

            fieldAccess = new OA::FieldAccess(memRefType,
                                              lhsMRE,
                                              mangledMethodName);


            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[fieldAccess] = other;

            // Create the rhs to represent the class of the lhs.
            memRefType = OA::MemRefExpr::USE;
            OA::SymHandle symHandle;
            symHandle = getVTableBaseSymHandle(classDefinition);
	
            OA::OA_ptr<OA::MemRefExpr> classMRE;

            classMRE = new OA::NamedRef(memRefType,
                                        symHandle);

            OA::OA_ptr<OA::AddressOf> address_mre;
            OA::OA_ptr<OA::MemRefExpr> nullMRE;

            address_mre = new OA::AddressOf(  OA::MemRefExpr::USE,
                                              nullMRE);

            classMRE = address_mre->composeWith(classMRE);


            // Record the type of the MRE (reference or non-reference).
            mMre2TypeMap[classMRE] = other;

            _makePtrAssignPair(stmt, fieldAccess, classMRE);	
 
            // Do not recurse at this point.
            // This virtual table will suffice to handle
            // all virtual methods in the parent class as well.
            return;
        }

    } else {

        // This is the non-virtual table optimization, i.e., per-method
        // virtual method model.

        // Create an implicit ptr assignment pair for all methods in this
        // class definition and for all methods in all superclasses.

        // First, visit all methods in this class.
        SgDeclarationStatementPtrList &members = 
            classDefinition->get_members(); 
        for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
             it != members.end(); ++it) { 

            SgDeclarationStatement *declarationStatement = *it; 
            ROSE_ASSERT(declarationStatement != NULL);

            switch(declarationStatement->variantT()) {
            case V_SgMemberFunctionDeclaration:
                {
                    SgMemberFunctionDeclaration *functionDeclaration =  
                        isSgMemberFunctionDeclaration(declarationStatement); 
                    // Base case: 
                    // Create implicit pointer assignment pairs for any
                    // methods declared in this class.
                    if ( ( functionDeclaration != NULL ) &&
                         ( isVirtual(functionDeclaration)  ) &&
			 ( !isPureVirtual(functionDeclaration) || hasDefinition(functionDeclaration) ) ) {
                        // Don't visit a virtual method if we have already
                        // visited a virtual method which overrides it.
                        bool visitedAnOverridingMethod = false;
                        std::list<SgMemberFunctionDeclaration *>::iterator it =
                            visitedVirtualMethods.begin();
                        for(; it != visitedVirtualMethods.end(); ++it) {
                            SgMemberFunctionDeclaration *memberFuncDecl = *it;
                            ROSE_ASSERT(memberFuncDecl != NULL);
                            if ( matchingFunctions(memberFuncDecl, 
                                                   functionDeclaration) ) {
                                visitedAnOverridingMethod = true;
                                break;
                            }
                        }

                        if ( !visitedAnOverridingMethod ) {
                            visitedVirtualMethods.push_back(functionDeclaration);
		
                            // Create an implicit pointer assignment pair 
                            // for this method m of class C of the form:
                            // < (*lhs).m, &C::m >
                            // Since we do not accurately model structures,
                            // we collapse the lhs 'a.b.c' to 'a' and 
                            // flag it is an inaccurate.
                            // 1/15/06:  If lhsMRE is inaccurate, 
                            // it will already have been marked as such.

                            // Create the lhs of the pair.
                            OA::OA_ptr<OA::MemRefExpr> baseLHS;
                            baseLHS = lhsMRE->clone();

                            // Transfer type to cloned MRE
                            mMre2TypeMap[baseLHS] = mMre2TypeMap[lhsMRE];

                            OA::MemRefExpr::MemRefType memRefType = 
                                OA::MemRefExpr::DEF;
                            string mangledMethodName = 
                                mangleFunctionName(functionDeclaration);
		
                            // As above, we expect that the lhs has
                            // already been Deref'ed if need be.
                            
                            OA::OA_ptr<OA::MemRefExpr> fieldAccess;

                            // fieldAccess accuracy depends on baseLHS accuracy
                         
                            fieldAccess = new OA::FieldAccess(memRefType,
                                                              baseLHS,
                                                              mangledMethodName);
                          
                            if(baseLHS->isaRefOp()) { 
                               
                               OA::OA_ptr<OA::RefOp> refOp;
                               refOp = baseLHS.convert<OA::RefOp>();

                               if(refOp->isaSubSetRef()) {
                                  OA::OA_ptr<OA::SubSetRef> subset_mre;
                                  OA::OA_ptr<OA::MemRefExpr> nullMRE;
                                  OA::OA_ptr<OA::MemRefExpr> composed_mre;

                                  subset_mre = new OA::SubSetRef(
                                                OA::MemRefExpr::USE,
                                                nullMRE
                                             );
                                  fieldAccess
                                  = subset_mre->composeWith(fieldAccess->clone());
                               }
		                    }
 
                            // Record the type of the MRE (reference or non-reference).
                            mMre2TypeMap[fieldAccess] = other;

                            // Create the rhs to represent the method 
                            // declaration.
                            memRefType = OA::MemRefExpr::USE;
                            OA::SymHandle symHandle;
                            symHandle = getProcSymHandle(functionDeclaration);

                            OA::OA_ptr<OA::MemRefExpr> method;

                            method = new OA::NamedRef(memRefType,
                                                      symHandle);

                            if(baseLHS->isaRefOp()) {

                               OA::OA_ptr<OA::RefOp> refOp;
                               refOp = baseLHS.convert<OA::RefOp>();

                               if(refOp->isaSubSetRef()) {
                                  OA::OA_ptr<OA::SubSetRef> subset_mre;
                                  OA::OA_ptr<OA::MemRefExpr> nullMRE;
                                  OA::OA_ptr<OA::MemRefExpr> composed_mre;

                                  subset_mre = new OA::SubSetRef(
                                                OA::MemRefExpr::USE,
                                                nullMRE
                                             );
                                  method
                                  = subset_mre->composeWith(method->clone()
);
                               }
                            }


                            OA::OA_ptr<OA::AddressOf> address_mre;
                            OA::OA_ptr<OA::MemRefExpr> nullMRE;

                             address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

                             method = address_mre->composeWith(method);

		
                            // Record the type of the MRE (reference or non-reference).
                            mMre2TypeMap[method] = other;

                            // Create the pair.
                            _makePtrAssignPair(stmt, fieldAccess, method);
                        }
                    }
                    break;
                }
            default:
                {
                    break;
                }
            }
        }
    }

    // Recursively call this method on each base class.
    SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances(); 
    for (SgBaseClassPtrList::iterator i = baseClassList.begin(); 
         i != baseClassList.end(); ++i) {

        SgBaseClass *baseClass = *i;
        ROSE_ASSERT(baseClass != NULL);
 
        SgClassDeclaration *classDeclaration = baseClass->get_base_class(); 
        ROSE_ASSERT(classDeclaration != NULL);

        classDeclaration = getDefiningDeclaration(classDeclaration);
        ROSE_ASSERT(classDeclaration != NULL);

        SgClassDefinition  *parentClassDefn = 
            classDeclaration->get_definition(); 
        if ( parentClassDefn != NULL ) {
            createImplicitPtrAssignPairsForVirtualMethods(stmt,
                                                          lhsMRE,
                                                          parentClassDefn,
                                                          visitedVirtualMethods);
        }
    }
}

/** \brief If the explicit ptr assign pair <lhs_mre, rhs_mre> allocates
 *         and instantiates an object via new (in rhs_mre), then create
 *         implicit pointer assign pairs to model virtual methods.
 *  \param stmt  The statement within which the <lhs_mre, rhs_mre> pair
 *               occurs.
 *  \param lhs_mre  A MemRefExpr for the left-hand side of the assignment.
 *  \param rhs_mre  A MemRefExpr for the right-hand side of the assignment.
 *
 *  The implicit pointer assignments bind a field of an object
 *  to the symbol for a virtual method.
 *
 *  We offer two models: the default virtual table model and 
 *  a per-method model.  Selection is controlled via mUseVtableOpt.
 *
 *  Under the virtual table optimization:
 *  For each <lhs, rhs> where rhs is a new expression
 *     type = class allocated by rhs new expression
 *     lhs->"vptr" = &type
 *
 *  For each class definition (SgClassDefinition) of type t
 *     For each virtual method vm of t
 *        t.vm = &t::vm
 *
 *  Under the per-method model:
 *  For each <lhs, rhs> where rhs is a new expression
 *     type = class allocated by rhs new expression
 *     For each virtual method vm of type
 *         lhs->vm = &type::vm
 */
void
SageIRInterface::createImplicitPtrAssignPairsForDynamicObjectAllocation(OA::StmtHandle stmt, 
                                                                        OA::OA_ptr<OA::MemRefExpr> lhs_mre, 
                                                                        OA::OA_ptr<OA::MemRefExpr> rhs_mre)
{

   OA::ExprHandle rhs_expr;
   OA::OA_ptr<OA::MemRefExpr> newmre ;

   newmre = rhs_mre->clone();

   if(rhs_mre->isaRefOp()) {
       // for the case like AddressOf(SubSetRef(UnnamedRef(...)..)..)
       // recurse on MemRefExpr until UnnamedRef is found
       OA::OA_ptr<OA::RefOp> refOp = rhs_mre.convert<OA::RefOp>();
       
       newmre = refOp->getMemRefExpr();

       while(newmre->isaRefOp()) {
          refOp = newmre.convert<OA::RefOp>();
          newmre = refOp->getMemRefExpr();
       }
   } 

   if( newmre.ptrEqual(0) || !newmre->isaUnnamed()) {
        return;
   }

   
    OA::OA_ptr<OA::UnnamedRef> unnamed_mre
              = newmre.convert<OA::UnnamedRef>();
   
    ROSE_ASSERT(!unnamed_mre.ptrEqual(0));
    rhs_expr = unnamed_mre->getExprHandle();

    // Verify that this stmt handle maps to an AST node of an
    // expected type.
    verifyStmtHandleType(stmt);

    SgNode *node = getNodePtr(rhs_expr);
    ROSE_ASSERT(node != NULL);

    SgNewExp *newExp = isSgNewExp(node);
    if ( newExp == NULL ) {
        return;
    }

    SgType *type = getBaseType(newExp->get_type());
    ROSE_ASSERT(type != NULL);

    // We only create implicit pointer assignment pairs
    // for classes/structs, not basic types.
    SgPointerType *pointerType = isSgPointerType(type);
    if ( pointerType ) {
        type = getBaseType(pointerType->get_base_type());
        ROSE_ASSERT(type != NULL);
    }

    if ( !isSgNamedType(type) ) {
        return;
    }

    // The explicit ptr assignment pair does involve a right-hand
    // side that is allocating an object.  Therefore, we do
    // need to create implicit assignment pairs.

    // Extract the class being allocated.
    SgClassDeclaration *classDeclaration =
        getClassDeclaration(type);
    if ( classDeclaration == NULL ) {
        return;
    }

    classDeclaration = getDefiningDeclaration(classDeclaration);
    if ( classDeclaration == NULL ) {
        return;
    }

    SgClassDefinition *classDefinition = 
        classDeclaration->get_definition();

    if ( classDefinition == NULL ) {
        return;
    }

    std::list<SgMemberFunctionDeclaration *> visitedVirtualMethods;

    // The lhs is obviously a pointer if it is assigned
    // to a new expression.  Therefore, dereference it
    // so that createImplicitPtrAssignPairsForVirtualMethods
    // may add a field access to it.
    OA::OA_ptr<OA::Deref> deref_mre;

    int numDerefs = 1;

    OA::OA_ptr<OA::MemRefExpr> nullMRE;

    OA::OA_ptr<OA::MemRefExpr> composed_mre;
    deref_mre = new OA::Deref(OA::MemRefExpr::USE,
                              nullMRE,
                              numDerefs);

    OA::OA_ptr<OA::MemRefExpr> base;
    base = lhs_mre->clone();
    base->setMemRefType(OA::MemRefExpr::USE);
    composed_mre = deref_mre->composeWith(base);
    
    if(lhs_mre->isaRefOp()) {
        
       OA::OA_ptr<OA::RefOp> refOp;
       refOp = lhs_mre.convert<OA::RefOp>();
       if(refOp->isaRefOp()) {
           
          OA::OA_ptr<OA::SubSetRef> subset_mre;
          OA::OA_ptr<OA::MemRefExpr> nullMRE;

          subset_mre = new OA::SubSetRef(
                                         OA::MemRefExpr::USE,
                                         nullMRE
                                         );

          composed_mre 
                 = subset_mre->composeWith(composed_mre->clone());                 
        }
    }

    OA::OA_ptr<OA::MemRefExpr> composedMre;
    composedMre = composed_mre->clone();

    
    // Record the type of the MRE (reference or non-reference).
    mMre2TypeMap[composedMre] = other;

    createImplicitPtrAssignPairsForVirtualMethods(stmt,
                                                  composedMre,
                                                  classDefinition,
                                                  visitedVirtualMethods);
}

/** \brief If the declaration corresponding to initName declares
 *         an object, then create implicit pointer assign
 *         pairs to model its virtual methods.
 *  \param stmt  The statement within which the object declaration occurs.
 *  \param lhs_mre  A MemRefExpr for the object declared.
 *  \param initName  a SgNode representing an initialized name from
 *                   a declaration.
 *
 *  The implicit pointer assignments bind a field of an object
 *  to the symbol for a virtual method.
 *
 *  We offer two models: the default virtual table model and 
 *  a per-method model.  Selection is controlled via mUseVtableOpt.
 *
 *  Under the virtual table optimization:
 *  For each object declaration where lhs is the object declared.
 *     type = object's class
 *     lhs."vptr" = &type
 *
 *  For each class definition (SgClassDefinition) of type t
 *     For each virtual method vm of t
 *        t.vm = &t::vm
 *
 *  Under the per-method model:
 *  For each object declaration where lhs is the object declared.
 *     type = declared object's class
 *     For each virtual method vm of type
 *         lhs.vm = &type::vm
 */
void
SageIRInterface::createImplicitPtrAssignPairsForObjectDeclaration(OA::StmtHandle stmt, 
                                                                  OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                                                                  SgInitializedName *initName)
{
    ROSE_ASSERT(initName != NULL);

    SgType *type = getBaseType(initName->get_type());
    ROSE_ASSERT(type != NULL);
  
    // We are only interested in object instantiations, 
    // not pointer or reference declarations.
    if ( isSgReferenceType(type) || isSgPointerType(type) ) {
        return;    
    }

    // Could replace the above conditional with this;
    // just being explicit.
    if ( !isSgNamedType(type) ) {
        return;
    }

    SgClassDeclaration *classDeclaration =
        getClassDeclaration(type);
    if ( classDeclaration == NULL ) {
        return;
    }

    classDeclaration = getDefiningDeclaration(classDeclaration);
    if ( classDeclaration == NULL ) {
        return;
    }

    SgClassDefinition *classDefinition = 
        classDeclaration->get_definition();
    if ( classDefinition == NULL ) {
        return;
    }

    if ( !classHasVirtualMethods(classDefinition) ) {
        return;
    }

    std::list<SgMemberFunctionDeclaration *> visitedVirtualMethods;

    createImplicitPtrAssignPairsForVirtualMethods(stmt,
                                                  lhs_mre,
                                                  classDefinition,
                                                  visitedVirtualMethods);
}


/** \brief Create implicit pointer assignment pairs to model
 *         virtual methods in classDefinition and its base classes
 *         for the virtual function table model.
 *  \param  stmt  The StmtHandle of the statement generating
 *                these implicit assignments.
 *  \param  classDefinition  The class for which we need to 
 *                           model virtual methods.
 *  \param  visitedVirtualMethods  A list of virtual methods for
 *                                 which we have already created
 *                                 implicit pointer assignment
 *                                 pairs for with respect to the
 *                                 top-level invocation of this
 *                                 method on classDefinition.
 *
 *
 *  For all virtual methods vm in class C
 *     create pair < C."vm", &C::vm >
 *
 *  Recurse up class hierarhcy.
 *
 *  NB:  The recursion means that the virtual function table
 *       for this class with contain entries to virtual methods
 *       defined by it or any of its ancestors.
 *
 *  Compare this method with createImplicitPtrAssignPairsForVirtualMethods,
 *  which creates per-method implicit assignments at object creation
 *  and instantiation for the non-virtual function table model.  
 *  Instead of creating those pairs once per creation/instantiation,
 *  we create analogous pairs here once per class definition.
 *
 *  Expect use of these pairs:
 *  A call handle for the virtual function table model should
 *  be *((*receiver)."OA_VTABLE_STR")."method".  
 *  createImplicitPtrAssignPairsForVirtualMethods should create
 *  < (*reciever)."OA_VTABLE_STR", &Class > for the virtual function
 *  table model.
 *  Here, we create < Class."method", Class::method >.  Unifying the
 *  above resolves the call handle to
 *  < *((*receiver)."OA_VTABLE_STR")."method", Class::method >.
 */
void
SageIRInterface::createImplicitPtrAssignPairsForClassDefinition(OA::StmtHandle stmt,
                                                                SgClassDefinition *topClassDefinition,
                                                                SgClassDefinition *classDefinition,
                                                                std::list<SgMemberFunctionDeclaration *> &visitedVirtualMethods)
{
    if ( !mUseVtableOpt ) {
        return;
    }

    ROSE_ASSERT(topClassDefinition != NULL);
    ROSE_ASSERT(classDefinition != NULL);

    // Create an implicit ptr assignment pair for all methods in this
    // class definition and for all methods in all superclasses.
    // Such ptr assignments will assign a function symbol to a slot
    // in the virtual function table.

    // First, visit all methods in this class.
    SgDeclarationStatementPtrList &members = classDefinition->get_members(); 
    for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
         it != members.end(); ++it) { 

        SgDeclarationStatement *declarationStatement = *it; 
        ROSE_ASSERT(declarationStatement != NULL);

        switch(declarationStatement->variantT()) {
        case V_SgMemberFunctionDeclaration:
            {
                SgMemberFunctionDeclaration *functionDeclaration =  
                    isSgMemberFunctionDeclaration(declarationStatement); 
	
                // Base case: 
                // Create implicit pointer assignment pairs for any methods
                // declared in this class.
                if ( ( functionDeclaration != NULL ) && 
                     ( isVirtual(functionDeclaration) ) && 
                     ( !isPureVirtual(functionDeclaration) || hasDefinition(functionDeclaration) ) ) {
                    // Don't visit a virtual method if we have already
                    // visited a virtual method which overrides it.
                    bool visitedAnOverridingMethod = false;
                    std::list<SgMemberFunctionDeclaration *>::iterator it =
                        visitedVirtualMethods.begin();
                    for(; it != visitedVirtualMethods.end(); ++it) {
                        SgMemberFunctionDeclaration *memberFuncDecl = *it;
                        ROSE_ASSERT(memberFuncDecl != NULL);
                        if ( matchingFunctions(memberFuncDecl, 
                                               functionDeclaration) ) {
                            visitedAnOverridingMethod = true;
                            break;
                        }
                    }

                    if ( !visitedAnOverridingMethod ) {
                        visitedVirtualMethods.push_back(functionDeclaration);

                        // Create the lhs MRE-- i.e., the vtable slot entry
                        // for this method: A."method".
                        OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
    
                        OA::SymHandle symHandle;
			//                        symHandle = getVTableBaseSymHandle(classDefinition);
                        // Important!  We are flattening the virtual function pointer 
                        // table into a single monolithic table for a 
                        // class, therefore always use its SgClassDefinition,
                        // rather than a super class's, as the base.
        		        symHandle = getVTableBaseSymHandle(topClassDefinition);
    
                        OA::OA_ptr<OA::NamedRef> classMRE;

                        classMRE = new OA::NamedRef(memRefType,
                                                    symHandle);

    
                        // Record the type of the MRE (reference or non-reference).
                        mMre2TypeMap[classMRE] = other;

                        string mangledMethodName = 
                            mangleFunctionName(functionDeclaration);
                        memRefType = OA::MemRefExpr::DEF;
    
                        OA::OA_ptr<OA::FieldAccess> fieldAccess;


                        fieldAccess = new OA::FieldAccess(memRefType,
                                                          classMRE,
                                                          mangledMethodName);

    
                        // Record the type of the MRE (reference or non-reference).
                        mMre2TypeMap[fieldAccess] = other;

                        // Create the rhs MRE-- i.e., the resolved function
                        // symbol &A::method.
                        memRefType = OA::MemRefExpr::USE;
                        symHandle = getProcSymHandle(functionDeclaration);
    	  
                        OA::OA_ptr<OA::MemRefExpr> method;

                        method = new OA::NamedRef(memRefType,
                                                  symHandle);

                        OA::OA_ptr<OA::AddressOf> address_mre;
                        OA::OA_ptr<OA::MemRefExpr> nullMRE;

                        

                        address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

                        method = address_mre->composeWith(method);

                        // Record the type of the MRE (reference or non-reference).
                        mMre2TypeMap[method] = other;
    
                        // Create the pair.
                        _makePtrAssignPair(stmt, fieldAccess, method);
                    }
                }
                break;
            }
        default:
            {
                break;
            }
        }
    }

    // Now recurse up the class hierarchy.  Any virtual methods defined
    // in a base class and not overridden by this class will be 
    // flatten into this class' virtual function table.
    SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances(); 
    for (SgBaseClassPtrList::iterator i = baseClassList.begin(); 
         i != baseClassList.end(); ++i) {
 
        SgBaseClass *baseClass = *i;
        ROSE_ASSERT(baseClass != NULL);

        SgClassDeclaration *classDeclaration = baseClass->get_base_class(); 
        ROSE_ASSERT(classDeclaration != NULL);

        classDeclaration = getDefiningDeclaration(classDeclaration);
        ROSE_ASSERT(classDeclaration != NULL);

        SgClassDefinition  *parentClassDefinition  = 
            classDeclaration->get_definition(); 

        if ( parentClassDefinition != NULL ) {
            createImplicitPtrAssignPairsForClassDefinition(stmt,
                                                           topClassDefinition,
                                                           parentClassDefinition,
                                                           visitedVirtualMethods);
        }
    }
}

void 
SageIRInterface::_makePtrAssignPair(OA::StmtHandle stmt,
                                   OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                                   OA::OA_ptr<OA::MemRefExpr> rhs_mre)
{
    // You should _not_ be calling this method. 
    // It should only be invoked (from methods invoked) from
    // makePtrAssignPair.
    mStmtToPtrPairs[stmt].insert(
        pair<OA::OA_ptr<OA::MemRefExpr>, 
             OA::OA_ptr<OA::MemRefExpr> >(lhs_mre,rhs_mre));
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
          makePtrAssignPair(stmt, lhs_mre, rhs_mre);
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
      makePtrAssignPair(stmt, lhs_mre, rhs_mre);
    } 
}

void 
SageIRInterface::makePtrAssignPair(OA::StmtHandle stmt,
                                   OA::OA_ptr<OA::MemRefExpr> lhs_mre,
                                   OA::OA_ptr<OA::MemRefExpr> rhs_mre)
{
    _makePtrAssignPair(stmt, lhs_mre, rhs_mre);
    // We need to create implicit pointer assignment pairs
    // to model virtual method calls if the RHS allocates
    // and instantiates an object via new. 
    createImplicitPtrAssignPairsForDynamicObjectAllocation(stmt, lhs_mre, rhs_mre);
}

void 
SageIRInterface::makeAssignPair(OA::StmtHandle stmt,
                                OA::MemRefHandle lhs_memref,
                                OA::ExprHandle rhs_expr)
{
    mStmtToAssignPairs[stmt].insert(
        pair<OA::MemRefHandle,
             OA::ExprHandle>(lhs_memref, rhs_expr));
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
    ROSE_ASSERT(0);
    
    // This method needs to be phased out.  It is not nearly general
    // enough.  e.g., it will die on a->field, a.b.field, etc.
    // This is too difficult to do from OA, just do it from ROSE--
    // i.e., just convert the rhs operand of an arrow or dot
    // expression to a string.

    std::string retval = "<none>";
    OA::OA_ptr<OA::MemRefExprIterator> mIter
        = getMemRefExprIterator(memref);
    for ( ; mIter->isValid(); ++(*mIter) ) {
        OA::OA_ptr<OA::MemRefExpr> mre = mIter->current();
        if (mre->isaNamed()) {
            OA::OA_ptr<OA::NamedRef> named_mre = mre.convert<OA::NamedRef>();
            retval = toStringWithoutScope(named_mre->getSymHandle());
        } else {
	  dump(mre, std::cout);
            ROSE_ASSERT(0);
        }
    }
    return retval;
}

/*!
   Takes the node where pointer arithmetic might be occuring
   and determines if it is based on the given child.
   Returns boolean indicating whether node is assigned a 
   memrefhandle.
 */
bool SageIRInterface::createMemRefExprsForPtrArith(SgExpression* node, 
						   SgExpression* child, OA::StmtHandle stmt,
                                                   bool removeChild)
{
    bool nodeHasMemRefHandle = false;
    OA::MemRefHandle child_memref  = findTopMemRefHandle(child);
    SgExpression *child_node=NULL;
    SgType *child_type;
    if (child_memref!=OA::MemRefHandle(0)) {
        child_node = isSgExpression(getSgNode(child_memref));
        ROSE_ASSERT(child_node);
        child_type = getBaseType(child_node->get_type());
    }
    if (child_node && 
        (isSgArrayType(child_type)||isSgPointerType(child_type)))
    {
        // this node is a MemRefHandle
        OA::MemRefHandle memref = getMemRefHandle(node);
        mStmt2allMemRefsMap[stmt].insert(memref);

        nodeHasMemRefHandle = true;

        OA::OA_ptr<OA::MemRefExprIterator> mIter 
            = getMemRefExprIterator(child_memref);
        for ( ; mIter->isValid(); ++(*mIter) ) {
            OA::OA_ptr<OA::MemRefExpr> child_mre = mIter->current();
            if (isSgArrayType(child_type)) {
                if ( removeChild ) {
                    mMemref2mreSetMap[child_memref].erase(child_mre);
                    mStmt2allMemRefsMap[stmt].erase(child_memref);
                }
                // set accuracy to false please see below

                OA::OA_ptr<OA::SubSetRef> subset_mre;
                OA::OA_ptr<OA::MemRefExpr> nullMRE;
                OA::OA_ptr<OA::MemRefExpr> composed_mre;

                subset_mre = new OA::SubSetRef(
                                           OA::MemRefExpr::USE,
                                           nullMRE
                                          );


                child_mre
                   = subset_mre->composeWith(child_mre);

                
                // set the addressTaken for the var because could
                // be computing the address of the array
		        // child_mre->setAddressTaken(true);
        
                OA::OA_ptr<OA::AddressOf> address_mre;

                address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

                child_mre = address_mre->composeWith(child_mre);
                
                // conservatively assume that we are computing address 
                // somewhere into the array
                
                // take the MRE away from the lhs and make it
                // belong to this node
                mMemref2mreSetMap[memref].insert(child_mre);
            }
            if (isSgPointerType(child_type)) {
                // want  Deref(NamedRef(a),addressOf=T,part)
                // addressOf doesn't cancel out deref because
                // deref is partial but the namedRef to a is full
                // get a clone of the child
   	            //	      if ( !(isDeref(child_mre) && !child_mre->hasFullAccuracy()) ) {

                OA::OA_ptr<OA::MemRefExpr> mre = child_mre->clone();

                // Transfer type to cloned MRE
                mMre2TypeMap[mre] = mMre2TypeMap[child_mre];

                int numDerefs = 1;

                OA::OA_ptr<OA::MemRefExpr> nullMRE;
                OA::OA_ptr<OA::Deref> derefMRE;
                derefMRE = new OA::Deref(OA::MemRefExpr::USE,
                                    nullMRE, numDerefs);
                mre = derefMRE->composeWith(mre);

                // set accuracy false
                OA::OA_ptr<OA::SubSetRef> subset_mre;
                OA::OA_ptr<OA::MemRefExpr> composed_mre;

                subset_mre = new OA::SubSetRef(
                                           OA::MemRefExpr::USE,
                                           nullMRE
                                          );
                mre = subset_mre->composeWith(mre->clone());


                // * PLM : 1/19/2007
                // * Do I need one more level of Deref for cases like
                // * 
                // * int *constPtr;
                // * int * q = &constPtr[i];
                // *
                // * Need to ask Michelle

                // mre = new OA::Deref(accuracy,OA::MemRefExpr::USE,
                //                    mre, numDerefs);

                OA::OA_ptr<OA::AddressOf> address_mre;
                
                address_mre = new OA::AddressOf(
                                          OA::MemRefExpr::USE,
                                          nullMRE);

                
                mre = address_mre->composeWith(mre);

                // Record the type of the MRE (reference or non-reference).
                mMre2TypeMap[mre] = other;

                mMemref2mreSetMap[memref].insert(mre);
		//	      }
            }
        }
    }
    return nodeHasMemRefHandle;
}

void SageIRInterface::createUseDefForVarArg(OA::MemRefHandle memref,
        OA::MemRefHandle valist_memref)
{
    // clone each MRE associated with the first parameter
    OA::OA_ptr<OA::MemRefExprIterator> mIter 
        = getMemRefExprIterator(valist_memref);
    for ( ; mIter->isValid(); ++(*mIter) ) {
        OA::OA_ptr<OA::MemRefExpr> mre = mIter->current();

        /* changed to MemRefExpr
        OA::OA_ptr<OA::Deref> deref_mre;
        */

        OA::OA_ptr<OA::MemRefExpr> deref_mre;

        int numDerefs = 1;
        OA::OA_ptr<OA::MemRefExpr> nullMRE;
 
     	OA::OA_ptr<OA::MemRefExpr> cloned = mre->clone();
        // Transfer type to cloned MRE
        mMre2TypeMap[cloned] = mMre2TypeMap[mre];

        deref_mre = new OA::Deref( OA::MemRefExpr::USEDEF,
                                   cloned,
                                   numDerefs);


        // set partial accuracy
        OA::OA_ptr<OA::SubSetRef> subset_mre;
        OA::OA_ptr<OA::MemRefExpr> composed_mre;

        subset_mre = new OA::SubSetRef(
                                     OA::MemRefExpr::USE,
                                     nullMRE
                                          );
        deref_mre = subset_mre->composeWith(deref_mre->clone());



        // Record the type of the MRE (reference or non-reference).
        mMre2TypeMap[deref_mre] = other;

        mMemref2mreSetMap[memref].insert(deref_mre);
        /* can't do this effectively because the comparison on mres
         * ignores the MRType 
        mre = deref_mre->clone();
        mre->setMemRefType(OA::MemRefExpr::DEF);
        mMemref2mreSetMap[memref].insert(mre);
        */
    }
}
/**  
 *   \brief  Return an MRE for the SgConstructorInitializer's implicit this.
 *   \param  ctorInitializer  A SgConstructorInitializer representing
 *                            the invocation of a constructor.
 *   \returns An expression used to represent the this (implicit or
 *            explicit) of a constructor invocation.
 *
 *   In the following examples, we return an MRE with addressOf set for the lhs:
 *       Foo f;
 *       Foo lhs(f);   // copy constructor.
 *       Foo lhs;      // default constructor.
 *       Foo lhs(1);   // some other constructor;
 *       Foo *lhs = new Foo;
 *
 *   In the following examples, the object constructed is
 *   anonymous, so we return that anonymous expression (or UnnamedRef)-- 
 *   in these examples, new Foo.
 *       return (new Foo);
 *       bar(new Foo);
 *       (new Foo)->methodCall();
 *
 *   For cases involving the invocation of a baseclass constructor:
 * 
 *       class Foo : public Bar { Foo(Foo &f) : Bar(f) { } };
 *
 *   we return a NamedRef to the "this" formal of Foo.
 *
 *   For cases involving the construction of a member variable:
 * 
 *       class Foo  { Foo(Bar &f) : mBar(f) { } Bar mBar; };
 *
 *   we return &(Foo::this->mBar).
 *
 *   For cases involving the construction of an actual:
 *  
 *       void foo(char* op, Tree t) {
 *           UnaryNode nodePtr(op,t);
 *       }
 *
 *   (here, a copy constructor is called for t), we return the copy
 *   constructor to represent the anonymous temporary-- i.e.,
 *   &(SgConstructorInitializer(t)).
 *
 *   For cases involving the construction of a return expr:
 *
 *       A a;
 *       ...
 *       return a;
 *
 *       or
 *
 *       return Bippin(123);   // where Bippin is a class.
 *
 *
 *   we return the copy constructor to represent the anonymous temporary--
 *   i.e., &(SgConstructorInitializer(a)).
 *
 *   For cases involving the construction of a rhs:
 *
 *   A lhs;
 *   A rhs;
 *   lhs = rhs;
 *
 *   we return &(SgConstructorInitializer(rhs)).  Note that
 *   we could return &(lhs), but there might be multiple lhs:
 *     ( cond ? lhs1 : lhs2 ) = rhs;
 *   such that we would have to return a list of MREs.
 * 
 */
OA::OA_ptr<OA::MemRefExpr> 
SageIRInterface::createConstructorInitializerReceiverMRE( SgConstructorInitializer *ctorInitializer)
{
    ROSE_ASSERT( ctorInitializer != NULL );
    OA::OA_ptr<OA::MemRefExpr> mre;
  
    // Recurse up the parents of ctorInitializer.  Return the lhs
    // of an assignment or the SgInitializedName of a
    // SgAssignInitializer.  These handle a new expression.
    // If the parent of the constructor initializer is a 
    // SgInitializedName, then figure out whether we have a base
    // class invocation, a member variable initialization, or a stack
    // declaration.
    // Stop the recursion and return
    // NULL if we reach a SgStatement without first finding
    // any of these cases.

    SgNode *parent = ctorInitializer->get_parent();

    SgInitializedName *initName = isSgInitializedName(parent);
    if ( initName != NULL ) {
      // If the parent is a SgInitializedName whose name is the
      // same as the constructor being invoked, then that SgInitializedName
      // is not a variable, but a base class.
      if ( isBaseClassInvocation(ctorInitializer) ) {
          OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
          // get the symbol for the implicit formal this.
          OA::SymHandle symHandle = getThisFormalSymHandle(initName);

          mre = new OA::NamedRef( mrType,
                                  symHandle);

          // Record the type of the MRE (reference or non-reference).
          mMre2TypeMap[mre] = other;

      // this is the case where a constructor is being called
      // to construct a member variable
      } else if (isSgCtorInitializerList(initName->get_parent()))  {
          // FIXME: very similiar to some code in the SgInitialiazedName
          // case of findMemRefExprsAndPtrAssigns
          //======= create a USE FieldAccess with its address taken
          OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
          // get the symbol for the implicit formal this.
          OA::SymHandle symHandle = getThisFormalSymHandle(initName);

          mre = new OA::NamedRef(mrType, symHandle);
          
          // Deref the MRE.

          // Record the type of the MRE (reference or non-reference).
          mMre2TypeMap[mre] = other;

          int numDerefs = 1;
#if 1
          OA::OA_ptr<OA::MemRefExpr> deref_mre;
          deref_mre = derefMre(mre, OA::MemRefExpr::USE, numDerefs);
#else
          OA::OA_ptr<OA::Deref> deref_mre;
          deref_mre = new OA::Deref(addressTaken, accuracy, OA::MemRefExpr::USE,
                                    mre, numDerefs);
#endif

          // Record the type of the MRE (reference or non-reference).
          mMre2TypeMap[deref_mre] = other;

          // Create the FieldAccess MRE.
          std::string field_name = toStringWithoutScope(initName);

          mre = new OA::FieldAccess(OA::MemRefExpr::USE,
                                    deref_mre,
                                    field_name);

          
          OA::OA_ptr<OA::AddressOf> address_mre;
          OA::OA_ptr<OA::MemRefExpr> nullMRE;

          address_mre = new OA::AddressOf(
                                         OA::MemRefExpr::USE,
                                         nullMRE);

          mre = address_mre->composeWith(mre);

          // Record the type of the MRE (reference or non-reference).
          mMre2TypeMap[mre] = ( isSgReferenceType(initName->get_type()) ? reference : other );


      // this is the case where a stack variable is being constructed
      } else if (isSgVariableDeclaration(initName->get_parent()))  {
          // need the address of the stack variable being initialized
          OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
          OA::SymHandle sym = getNodeNumber(initName);

          mre = new OA::NamedRef(mrType, sym);

          OA::OA_ptr<OA::AddressOf> address_mre;
          OA::OA_ptr<OA::MemRefExpr> nullMRE;

          address_mre = new OA::AddressOf(
                                          OA::MemRefExpr::USE,
                                          nullMRE);

          mre = address_mre->composeWith(mre);


          // Record the type of the MRE (reference or non-reference).
          mMre2TypeMap[mre] = ( isSgReferenceType(initName->get_type()) ? reference : other );

      } else {
          ROSE_ASSERT(0); // should not get here
      }

      return mre;
    }
  
    // If the parent is a SgExprListExp, whose parent is a 
    // SgConstructorInitializer or a SgFunctionCallExp, then 
    // this is an invocation of a copy constructor for an object
    // passed as an actual argument.
    //
    // void foo(char* op, Tree t) {
    //   UnaryNode *nodePtr = new UnaryNode(op,t);
    // }
    //
    // return the address of an anonymous memory obj-- use
    // the SgConstructorInitializer for t.
    if ( isSgExprListExp(parent) ) {
        SgNode *grandParent = parent->get_parent();
        if ( isSgConstructorInitializer(grandParent) || 
             isSgFunctionCallExp(grandParent) ) {

          OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;

          OA::ExprHandle exprHandle = findTopExprHandle(ctorInitializer);
          if ( isSgExpression(ctorInitializer) ) {
               mre = new OA::UnnamedRef(mrType, exprHandle);
          } else {
               assert(0);
          }

          OA::OA_ptr<OA::AddressOf> address_mre;
          OA::OA_ptr<OA::MemRefExpr> nullMRE;

          address_mre = new OA::AddressOf(
                                              OA::MemRefExpr::USE,
                                              nullMRE);

          mre = address_mre->composeWith(mre);


          // Record the type of the MRE (reference or non-reference).
          mMre2TypeMap[mre] = other;
          return mre;
        }
        // Case of an aggregate initializer
        else if ( isSgAggregateInitializer(grandParent) ) {
             SgInitializedName *initName =
                isSgInitializedName(grandParent->get_parent());
             ROSE_ASSERT(initName != NULL);
             SgVariableDeclaration *varDecl =
                isSgVariableDeclaration(initName->get_parent());
             ROSE_ASSERT(varDecl != NULL);

             bool addressTaken = true;
             bool accuracy = true;
             OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
             OA::SymHandle sym = getNodeNumber(initName);
             mre = new OA::NamedRef(mrType, sym);

             // Record the type of the MRE (reference or non-reference).
             mMre2TypeMap[mre] =
                ( isSgReferenceType(initName->get_type()) ?
                reference : other );
             return mre;
        }
    }

    // If the parent is an expression root, and its parent is
    // a return statement, then a copy constructor is invoked
    // because we are returning an object.
    SgNode *grandParent2 = parent->get_parent();
    if ( ( isSgExpressionRoot(parent) && isSgReturnStmt(grandParent2) ) ||
         ( isSgReturnStmt(parent) ) ) {
       OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;

       OA::ExprHandle exprHandle = findTopExprHandle(ctorInitializer);
       if ( isSgExpression(ctorInitializer) ) {
            mre = new OA::UnnamedRef(mrType, exprHandle);
       } else {
            assert(0);
       }

       OA::OA_ptr<OA::AddressOf> address_mre;
       OA::OA_ptr<OA::MemRefExpr> nullMRE;

       address_mre = new OA::AddressOf(OA::MemRefExpr::USE,
                                       nullMRE);

       mre = address_mre->composeWith(mre);

       // Record the type of the MRE (reference or non-reference).
       mMre2TypeMap[mre] = other;
       return mre;
    }

    // It looks like the parent could be an assign op as well.
    // In that case, return the address of the lhs.
    if ( isSgAssignOp(parent) ) {
        
        OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;

        OA::ExprHandle exprHandle = findTopExprHandle(ctorInitializer);
        if ( isSgExpression(ctorInitializer) ) {
             mre = new OA::UnnamedRef(mrType, exprHandle);
        } else {
             assert(0);
        }

        OA::OA_ptr<OA::AddressOf> address_mre;
        OA::OA_ptr<OA::MemRefExpr> nullMRE;

        address_mre = new OA::AddressOf(
                                        OA::MemRefExpr::USE,
                                        nullMRE);

        mre = address_mre->composeWith(mre);

  
        // Record the type of the MRE (reference or non-reference).
        mMre2TypeMap[mre] = other;
        return mre;
    }

    // At this point we know that the immediate parent is not
    // a SgInitializedName, because we took care of that case
    // above and then returned.
    // Now we keep moving up the tree.
    SgNode *lhs; 
    while ( ( parent != NULL ) && ( !isSgGlobal(parent) ) ) {
    
        if ( isSgStatement(parent) ) break;

        // this is the case where we have, Foo* f = new Foo();
        if ( isSgInitializedName(parent) ) {
            lhs = parent;
            break;
            
        // this is the case where we have, Foo* f; f = new Foo();
        } else {
            SgAssignOp *assignOp = isSgAssignOp(parent);
            if ( assignOp ) {
                lhs = assignOp->get_lhs_operand();
                break;
            }
        }
        parent = parent->get_parent();
    }

    if ( mre.ptrEqual(NULL) ) {
        // We were unable to find a lhs.  Hopefully this is
        // one of the anonymous memory object cases named above.
        // In that case, our parent had better be a SgNewExp.
        lhs = isSgNewExp(ctorInitializer->get_parent());
    }
    if ( lhs == NULL ) {
        std::cout << "error at: " <<
            ctorInitializer->get_startOfConstruct()->get_filenameString() <<
            ctorInitializer->get_startOfConstruct()->get_line() << endl;
      std::cout << "lhs NULL: " << ctorInitializer->get_parent()->get_parent()->unparseToString() << std::endl;
    }
    ROSE_ASSERT(lhs != NULL);

    OA::MemRefHandle memref = findTopMemRefHandle(lhs);
    OA::OA_ptr<OA::MemRefExprIterator> mIter 
        = getMemRefExprIterator(memref);
    ROSE_ASSERT(mIter->isValid());
    mre = mIter->current();
    mre = mre->clone();

    // Transfer type to cloned MRE
    mMre2TypeMap[mre] = mMre2TypeMap[mIter->current()];

    mre->setMemRefType(OA::MemRefExpr::USE);

    return mre;
}


