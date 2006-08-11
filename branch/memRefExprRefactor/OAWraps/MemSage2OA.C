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
            // if not an access to field (how to tell this?) FIXME
                // is a MemRefHandle
                OA::MemRefHandle memref = (OA::irhandle_t)astNode;
                // create a NamedRef
                bool addressTaken = false;
                bool accuracy = true;
                OA::MemRefExpr::MemRefType mrType = OA::MemRefExpr::USE;
                OA::SymHandle sym= getNodeNumber(astNode);
                OA::OA_ptr<OA::MemRefExpr> mre;
                mre = new OA::NamedRef(addressTaken,accuracy, mrType, sym);
                mStmt2allMemRefsMap[stmt].insert(memref);
                mMemref2mreSetMap[memref].insert(mre);

                // if is a reference type then FIXME: implement this
                    // wrap the NamedRef in a Deref

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
            // change MREType for lhs to DEF FIXME

            // if parent is not a SgExpressionRoot
            if (!isSgExpressionRoot(astNode->get_parent())) {
                ROSE_ASSERT(0); // planned out but not implemented yet
                //`is a MemRefHandle because it results in a use 
                // of the location indicated by the lhs            

                // the MREs for this are the MREs for left child set to USE
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
            // is a MemRefHandle
            OA::MemRefHandle memref = (OA::irhandle_t)astNode;

            // MREs are all MREs for child node with addressOf=true
            // FIXME: this needs to be a call to topMemRef

            // child is no longer a MemRefHandle
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
            findAllMemRefsAndMemRefExprs(exprStatement->get_the_expr(),stmt);
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
            ROSE_ASSERT(0);
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
    case V_SgFunctionDefinition:
    case V_SgNamespaceDefinitionStatement:
    case V_SgClassDefinition:
        ROSE_ASSERT(0);
        break;

    // The "do nothing" statement cases
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
