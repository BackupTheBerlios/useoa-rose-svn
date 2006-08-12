#include "MemSage2OA.h"
#include "SageOACallGraph.h"
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
      mIR.initMemRefMaps();
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

void SageIRInterface::initMemRefMaps()
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
          findAllMemRefsAndMemRefExprs( getSgNode(stmt), stmt );
      }
  }
}

/*!
   Should originally be called on a statement so astNode and stmt will
   be the same.  This is a recursive procedure that traverses down
   the AST and then does work in postorder.
   As MemRefHandles are found and MREs created, mStmt2allMemRefsMap,
   mMemRef2StmtMap, mMemref2mreSetMap, and mCallToMRE map are
   updated.
*/
void SageIRInterface::findAllMemRefsAndMemRefExprs(SgNode *astNode, 
                                                   OA::StmtHandle stmt)
{
    ROSE_ASSERT(astNode != NULL);  bool retVal = false;
    switch(astNode->variantT()) {

    // ---------------------------------------- Expression cases
    case V_SgExprListExp:
        {
            ROSE_ASSERT(0);
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
            ROSE_ASSERT(0);
            break;
        }
    case V_SgMemberFunctionRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionCallExp:
        {
            ROSE_ASSERT(0);
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
                ROSE_ASSERT(0); // planned but not implemented yet

                // is a MemRefHandle

                // make a NamedRef for the variable being initialized

                // if is a reference then set the addressOf flag for the
                // MREs for the node under the SgInitializer node
            }
            break;
        }
    case V_SgAggregateInitializer:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgConstructorInitializer:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAssignInitializer:
        {
            ROSE_ASSERT(0);
            break;
        }

    // ---------------------------------------- Binary Op cases
    case V_SgArrowExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgDotExp:
        {
            ROSE_ASSERT(0);
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

    case V_SgEqualityOp:
    case V_SgLessThanOp:
    case V_SgGreaterThanOp:
    case V_SgNotEqualOp: 
    case V_SgLessOrEqualOp:
    case V_SgGreaterOrEqualOp:
    case V_SgAddOp:
    case V_SgSubtractOp:
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
            ROSE_ASSERT(0);
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
            findAllMemRefsAndMemRefExprs(assignOp->get_lhs_operand(), stmt); 
            findAllMemRefsAndMemRefExprs(assignOp->get_rhs_operand(), stmt); 

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

            break;
        }
    case V_SgPlusAssignOp:
    case V_SgMinusAssignOp:
    case V_SgAndAssignOp:
    case V_SgIorAssignOp:
    case V_SgMultAssignOp:
    case V_SgDivAssignOp:
    case V_SgModAssignOp:
    case V_SgXorAssignOp:
    case V_SgLshiftAssignOp:
    case V_SgRshiftAssignOp:
        {
            ROSE_ASSERT(0);
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
            findAllMemRefsAndMemRefExprs(((SgUnaryOp*)astNode)->get_operand(), stmt);
            break;
        }
    case V_SgPointerDerefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAddressOfOp:
        {
            SgAddressOfOp *addressOfOp = isSgAddressOfOp(astNode);
            ROSE_ASSERT(addressOfOp != NULL);

            // recurse on child
            findAllMemRefsAndMemRefExprs(addressOfOp->get_operand_i(),stmt);

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
                child_mre->setAddressTaken();
                mMemref2mreSetMap[memref].insert(child_mre);
            }

            // child is no longer a MemRefHandle
            mMemref2mreSetMap[child_memref].clear();
            break;
        }
    case V_SgMinusMinusOp:
    case V_SgPlusPlusOp:
        {
            ROSE_ASSERT(0);
            break;
        }

    // ---------------------------------------- Statement cases
    case V_SgExprStatement:
        {
            SgExprStatement *exprStatement = isSgExprStatement(astNode);
            ROSE_ASSERT(exprStatement!=NULL);
            findAllMemRefsAndMemRefExprs(exprStatement->get_expression_root(),stmt);
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
            findAllMemRefsAndMemRefExprs(returnStmt->get_return_expr(), stmt);
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
                findAllMemRefsAndMemRefExprs(lhs,stmt);
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
            ROSE_ASSERT(0);
            break;
        }
    case V_SgForStatement:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgWhileStmt:
    case V_SgDoWhileStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgCatchOptionStmt:
        {
            ROSE_ASSERT(0);
            break;
        }


    // The statement cases that we should not see
    case V_SgNamespaceDefinitionStatement:
    case V_SgClassDefinition:
        ROSE_ASSERT(0);
        break;

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
        break;

    default:
        // do nothing, there are nodes not listed above such as SgIntVal
        break;

    } // end of switch
}

//! finds the topmost MemRefHandle in the subtree rooted at the given node
//! if the tree represents only an rvalue, then MemRefHandle(0) is returned
//! unless the rvalue is an addressOf, which is assigned a MemRefHandle
OA::MemRefHandle SageIRInterface::findTopMemRefHandle(SgNode *astNode)
{
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
