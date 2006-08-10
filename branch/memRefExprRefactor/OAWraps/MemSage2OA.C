#include "MemSage2OA.h"
using namespace std;

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
  SgNode *node = mIR->getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  // if haven't already determined the set of memrefs for the program
  // then call initMemRefMaps
  if (mIR->mStmt2allMemRefsMap.empty() ) {
      mIR->initMemRefMaps();
  }

  // loop through MemRefHandle's for this statement and for now put them
  // into our own list
  std::set<OA::MemRefHandle>::iterator setIter;
  for (setIter=mIR->mStmt2allMemRefsMap[h].begin();
       setIter!=mIR->mStmt2allMemRefsMap[h].end(); setIter++)
    {
      mMemRefList.push_back(*setIter);
    }

}

void SageIRInterface::initMemRefMaps()
{
  // iterate over all procedures
  bool excludeInputFiles = false;  // FIXME: should this be true?
  OA_ptr<IRProcIterator> procIter = new SageIRProcIterator(wholeProject, 
                                              nodeArrayPtr, excludeInputFiles);
  for (procIter->reset() ; procIter->isValid(); ++(*procIter)) {
      ProcHandle currProc = procIter->current();

      // Iterate over the statements of this procedure
      OA_ptr<IRStmtIterator> stmtIterPtr = mIR->getStmtIterator(currProc);
      for ( ; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
          StmtHandle stmt = stmtIterPtr->current();

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
                                                   StmtHandle stmt)
{
    ROSE_ASSERT(astNode != NULL);  bool retVal = false;
    switch(astNode->variantT()) {

    // ---------------------------------------- Expression cases
    case V_SgArrowExp:

    // ---------------------------------------- Statement cases
    case V_SgExprStatement:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgCaseOptionStatement:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgTryStatement:
        {
            ROSE_ASSERT(0);
            break;
        }
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
            ROSE_ASSERT(0);
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
    case V_SgCatcheStatementSeq:
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
        ROSE_ASSERT(0);  // should have a case for all possible nodes

    } // end of switch
}
