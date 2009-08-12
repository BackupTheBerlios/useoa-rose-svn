#include "MemSage2OA.h"
#include "SageOACallGraph.h"
#include "common.h"

#define ROSE_0_8_9a      

using namespace std;
using namespace OA;
using namespace UseOA;
using namespace SageInterface;

bool debug = false;

SageIRMemRefIterator::SageIRMemRefIterator(OA::StmtHandle h,
                       SageIRInterface& ir)
  : mValid(true), mIR(ir)
{
  OA_DEBUG_CTRL_MACRO("DEBUG_MemSage2OA:ALL", debug);
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

SageMemRefHandleIterator::SageMemRefHandleIterator (OA::OA_ptr<std::list<OA::MemRefHandle> > pList)
    : OA::IRHandleListIterator<OA::MemRefHandle>(pList)
{
    OA_DEBUG_CTRL_MACRO("DEBUG_MemSage2OA:ALL", debug);
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
                //ROSE_ABORT();
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
            //return getMemRefHandle(0);
            return findTopMemRefHandle(kids[0]);
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
    for (lhsIter->reset(); lhsIter->isValid(); ++(*lhsIter) ) {
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

    // AIS for refactoring w/ visitor: I'm commenting this out for now.
    //createImplicitPtrAssignPairsForDynamicObjectAllocation(stmt, lhs_mre, rhs_mre);
}








/**
 * Iterates through every statement in every procedure of the program
 * and constructs abstractions for all the memory reference expressions,
 * pointer assignments, and function calls in those statements.
 **/
void SageIRInterface::initMemRefAndPtrAssignMaps() {
    MREVisitor visitor(*this);

    // iterate over all procedures
    OA_ptr<IRProcIterator> procIter;
    procIter = new SageIRProcIterator(wholeProject, *this);
    for(procIter->reset(); procIter->isValid(); ++(*procIter)) {
        ProcHandle currProc = procIter->current();

        // iterate over the statements of this procedure
        OA_ptr<IRStmtIterator> stmtIterPtr;
        stmtIterPtr = getStmtIterator(currProc);
        for(stmtIterPtr->reset(); stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
            StmtHandle stmt = stmtIterPtr->current();

            // using an MREVisitor object visit all the nodes in the
            // statement gathering MRE's, pointer assigns, and call handles
            visitor.visit(getSgNode(stmt), stmt);
        }
    }
}


void MREVisitor::visit(SgNode *node, OA::StmtHandle stmt) {
    // if node pointer is null ignoare
    if(node == NULL) { return; }
    VariantT nType = node->variantT();

    // if not a node that should not recurse, recurse on children
    
    if(nType != V_SgSourceFile &&
       nType != V_SgGlobal &&
       nType != V_SgFunctionDeclaration &&
       nType != V_SgFunctionDefinition &&
       nType != V_SgBasicBlock)
    {
        typedef vector<SgNode *> VectorOfNodes;
        VectorOfNodes children = node->get_traversalSuccessorContainer();
        for(VectorOfNodes::iterator i = children.begin();
            i != children.end(); i++)
        {
            visit(*i, stmt);
        }
    }

    // process this node
    switch(node->variantT()) {
        case V_SgStatementExpression:
            visitSgStatementExpression(isSgStatementExpression(node), stmt);
        break;

        case V_SgExprListExp:
            visitSgExprListExp(isSgExprListExp(node), stmt);
        break;

        case V_SgVarRefExp:
            visitSgVarRefExp(isSgVarRefExp(node), stmt);
        break;

        case V_SgClassNameRefExp:
            visitSgClassNameRefExp(isSgClassNameRefExp(node), stmt);
        break;

        case V_SgFunctionRefExp:
            visitSgFunctionRefExp(isSgFunctionRefExp(node), stmt);
        break;

        case V_SgMemberFunctionRefExp:
            visitSgMemberFunctionRefExp(isSgMemberFunctionRefExp(node), stmt);
        break;

        case V_SgFunctionCallExp:
            visitSgFunctionCallExp(isSgFunctionCallExp(node), stmt);
        break;

        case V_SgSizeOfOp:
            visitSgSizeOfOp(isSgSizeOfOp(node), stmt);
        break;

        case V_SgConditionalExp:
            visitSgConditionalExp(isSgConditionalExp(node), stmt);
        break;

        case V_SgNewExp:
            visitSgNewExp(isSgNewExp(node), stmt);
        break;

        case V_SgDeleteExp:
            visitSgDeleteExp(isSgDeleteExp(node), stmt);
        break;

        case V_SgThisExp:
            visitSgThisExp(isSgThisExp(node), stmt);
        break;

        case V_SgVarArgStartOp:
            visitSgVarArgStartOp(isSgVarArgStartOp(node), stmt);
        break;

        case V_SgVarArgCopyOp:
            visitSgVarArgCopyOp(isSgVarArgCopyOp(node), stmt);
        break;

        case V_SgVarArgStartOneOperandOp:
            visitSgVarArgStartOneOperandOp(
                isSgVarArgStartOneOperandOp(node), stmt);
        break;

        case V_SgVarArgOp:
            visitSgVarArgOp(isSgVarArgOp(node), stmt);
        break;

        case V_SgVarArgEndOp:
            visitSgVarArgEndOp(isSgVarArgEndOp(node), stmt);
        break;

        case V_SgInitializedName:
            visitSgInitializedName(isSgInitializedName(node), stmt);
        break;

        case V_SgAggregateInitializer:
            visitSgAggregateInitializer(isSgAggregateInitializer(node), stmt);
        break;

        case V_SgConstructorInitializer:
            visitSgConstructorInitializer(
                isSgConstructorInitializer(node), stmt);
        break;

        case V_SgAssignInitializer:
            visitSgAssignInitializer(isSgAssignInitializer(node), stmt);
        break;

        case V_SgArrowExp:
            visitSgArrowExp(isSgArrowExp(node), stmt);
        break;

        case V_SgDotExp:
            visitSgDotExp(isSgDotExp(node), stmt);
        break;

        case V_SgDotStarOp:
            visitSgDotStarOp(isSgDotStarOp(node), stmt);
        break;

        case V_SgArrowStarOp:
            visitSgArrowStarOp(isSgArrowStarOp(node), stmt);
        break;

        case V_SgAddOp:
            visitSgAddOp(isSgAddOp(node), stmt);
        break;

        case V_SgSubtractOp:
            visitSgSubtractOp(isSgSubtractOp(node), stmt);
        break;

        case V_SgEqualityOp:
            visitSgEqualityOp(isSgEqualityOp(node), stmt);
        break;

        case V_SgLessThanOp:
            visitSgLessThanOp(isSgLessThanOp(node), stmt);
        break;

        case V_SgGreaterThanOp:
            visitSgGreaterThanOp(isSgGreaterThanOp(node), stmt);
        break;

        case V_SgNotEqualOp:
            visitSgNotEqualOp(isSgNotEqualOp(node), stmt);
        break;

        case V_SgLessOrEqualOp:
            visitSgLessOrEqualOp(isSgLessOrEqualOp(node), stmt);
        break;

        case V_SgGreaterOrEqualOp:
            visitSgGreaterOrEqualOp(isSgGreaterOrEqualOp(node), stmt);
        break;

        case V_SgMultiplyOp:
            visitSgMultiplyOp(isSgMultiplyOp(node), stmt);
        break;

        case V_SgDivideOp:
            visitSgDivideOp(isSgDivideOp(node), stmt);
        break;

        case V_SgIntegerDivideOp:
            visitSgIntegerDivideOp(isSgIntegerDivideOp(node), stmt);
        break;

        case V_SgModOp:
            visitSgModOp(isSgModOp(node), stmt);
        break;

        case V_SgAndOp:
            visitSgAndOp(isSgAndOp(node), stmt);
        break;

        case V_SgOrOp:
            visitSgOrOp(isSgOrOp(node), stmt);
        break;

        case V_SgBitXorOp:
            visitSgBitXorOp(isSgBitXorOp(node), stmt);
        break;

        case V_SgBitAndOp:
            visitSgBitAndOp(isSgBitAndOp(node), stmt);
        break;

        case V_SgBitOrOp:
            visitSgBitOrOp(isSgBitOrOp(node), stmt);
        break;

        case V_SgLshiftOp:
            visitSgLshiftOp(isSgLshiftOp(node), stmt);
        break;

        case V_SgRshiftOp:
            visitSgRshiftOp(isSgRshiftOp(node), stmt);
        break;

        case V_SgCommaOpExp:
            visitSgCommaOpExp(isSgCommaOpExp(node), stmt);
        break;

        case V_SgPntrArrRefExp:
            visitSgPntrArrRefExp(isSgPntrArrRefExp(node), stmt);
        break;

        case V_SgAssignOp:
            visitSgAssignOp(isSgAssignOp(node), stmt);
        break;

        case V_SgPlusAssignOp:
            visitSgPlusAssignOp(isSgPlusAssignOp(node), stmt);
        break;

        case V_SgMinusAssignOp:
            visitSgMinusAssignOp(isSgMinusAssignOp(node), stmt);
        break;

        case V_SgAndAssignOp:
            visitSgAndAssignOp(isSgAndAssignOp(node), stmt);
        break;

        case V_SgIorAssignOp:
            visitSgIorAssignOp(isSgIorAssignOp(node), stmt);
        break;

        case V_SgMultAssignOp:
            visitSgMultAssignOp(isSgMultAssignOp(node), stmt);
        break;

        case V_SgDivAssignOp:
            visitSgDivAssignOp(isSgDivAssignOp(node), stmt);
        break;

        case V_SgModAssignOp:
            visitSgModAssignOp(isSgModAssignOp(node), stmt);
        break;

        case V_SgXorAssignOp:
            visitSgXorAssignOp(isSgXorAssignOp(node), stmt);
        break;

        case V_SgLshiftAssignOp:
            visitSgLshiftAssignOp(isSgLshiftAssignOp(node), stmt);
        break;

        case V_SgRshiftAssignOp:
            visitSgRshiftAssignOp(isSgRshiftAssignOp(node), stmt);
        break;

        case V_SgExpressionRoot:
            visitSgExpressionRoot(isSgExpressionRoot(node), stmt);
        break;

        case V_SgMinusOp:
            visitSgMinusOp(isSgMinusOp(node), stmt);
        break;

        case V_SgUnaryAddOp:
            visitSgUnaryAddOp(isSgUnaryAddOp(node), stmt);
        break;

        case V_SgNotOp:
            visitSgNotOp(isSgNotOp(node), stmt);
        break;

        case V_SgBitComplementOp:
            visitSgBitComplementOp(isSgBitComplementOp(node), stmt);
        break;

        case V_SgCastExp:
            visitSgCastExp(isSgCastExp(node), stmt);
        break;

        case V_SgThrowOp:
            visitSgThrowOp(isSgThrowOp(node), stmt);
        break;

        case V_SgTypeIdOp:
            visitSgTypeIdOp(isSgTypeIdOp(node), stmt);
        break;

        case V_SgPointerDerefExp:
            visitSgPointerDerefExp(isSgPointerDerefExp(node), stmt);
        break;

        case V_SgAddressOfOp:
            visitSgAddressOfOp(isSgAddressOfOp(node), stmt);
        break;

        case V_SgMinusMinusOp:
            visitSgMinusMinusOp(isSgMinusMinusOp(node), stmt);
        break;

        case V_SgPlusPlusOp:
            visitSgPlusPlusOp(isSgPlusPlusOp(node), stmt);
        break;

        case V_SgExprStatement:
            visitSgExprStatement(isSgExprStatement(node), stmt);
        break;

        case V_SgTryStmt:
            visitSgTryStmt(isSgTryStmt(node), stmt);
        break;

        case V_SgReturnStmt:
            visitSgReturnStmt(isSgReturnStmt(node), stmt);
        break;

        case V_SgSpawnStmt:
            visitSgSpawnStmt(isSgSpawnStmt(node), stmt);
        break;

        case V_SgVariableDeclaration:
            visitSgVariableDeclaration(isSgVariableDeclaration(node), stmt);
        break;

        case V_SgVariableDefinition:
            visitSgVariableDefinition(isSgVariableDefinition(node), stmt);
        break;

        case V_SgAsmStmt:
            visitSgAsmStmt(isSgAsmStmt(node), stmt);
        break;

        case V_SgFunctionParameterList:
            visitSgFunctionParameterList(isSgFunctionParameterList(node), stmt);
        break;

        case V_SgCtorInitializerList:
            visitSgCtorInitializerList(isSgCtorInitializerList(node), stmt);
        break;

        case V_SgIfStmt:
            visitSgIfStmt(isSgIfStmt(node), stmt);
        break;

        case V_SgSwitchStatement:
            visitSgSwitchStatement(isSgSwitchStatement(node), stmt);
        break;

        case V_SgForStatement:
            visitSgForStatement(isSgForStatement(node), stmt);
        break;

        case V_SgForInitStatement:
            visitSgForInitStatement(isSgForInitStatement(node), stmt);
        break;

        case V_SgWhileStmt:
            visitSgWhileStmt(isSgWhileStmt(node), stmt);
        break;

        case V_SgDoWhileStmt:
            visitSgDoWhileStmt(isSgDoWhileStmt(node), stmt);
        break;

        case V_SgCatchOptionStmt:
            visitSgCatchOptionStmt(isSgCatchOptionStmt(node), stmt);
        break;

        case V_SgNamespaceDefinitionStatement:
            visitSgNamespaceDefinitionStatement(
                isSgNamespaceDefinitionStatement(node), stmt);
        break;

        case V_SgClassDeclaration:
            visitSgClassDeclaration(isSgClassDeclaration(node), stmt);
        break;

        case V_SgClassDefinition:
            visitSgClassDefinition(isSgClassDefinition(node), stmt);
        break;

        case V_SgStringVal:
            visitSgStringVal(isSgStringVal(node), stmt);
        break;

        case V_SgFunctionDefinition:
            visitSgFunctionDefinition(isSgFunctionDefinition(node), stmt);
        break;

        case V_SgLabelStatement:
            visitSgLabelStatement(isSgLabelStatement(node), stmt);
        break;

        case V_SgDefaultOptionStmt:
            visitSgDefaultOptionStmt(isSgDefaultOptionStmt(node), stmt);
        break;

        case V_SgBreakStmt:
            visitSgBreakStmt(isSgBreakStmt(node), stmt);
        break;

        case V_SgContinueStmt:
            visitSgContinueStmt(isSgContinueStmt(node), stmt);
        break;

        case V_SgGotoStatement:
            visitSgGotoStatement(isSgGotoStatement(node), stmt);
        break;

        case V_SgCatchStatementSeq:
            visitSgCatchStatementSeq(isSgCatchStatementSeq(node), stmt);
        break;

        case V_SgClinkageStartStatement:
            visitSgClinkageStartStatement(
                isSgClinkageStartStatement(node), stmt);
        break;

        case V_SgEnumDeclaration:
            visitSgEnumDeclaration(isSgEnumDeclaration(node), stmt);
        break;

        case V_SgTemplateDeclaration:
            visitSgTemplateDeclaration(isSgTemplateDeclaration(node), stmt);
        break;

        case V_SgTypedefDeclaration:
            visitSgTypedefDeclaration(isSgTypedefDeclaration(node), stmt);
        break;

        case V_SgNamespaceDeclarationStatement:
            visitSgNamespaceDeclarationStatement(
                isSgNamespaceDeclarationStatement(node), stmt);
        break;

        case V_SgNamespaceAliasDeclarationStatement:
            visitSgNamespaceAliasDeclarationStatement(
                isSgNamespaceAliasDeclarationStatement(node), stmt);
        break;

        case V_SgUsingDirectiveStatement:
            visitSgUsingDirectiveStatement(
                isSgUsingDirectiveStatement(node), stmt);
        break;

        case V_SgUsingDeclarationStatement:
            visitSgUsingDeclarationStatement(
                isSgUsingDeclarationStatement(node), stmt);
        break;

        case V_SgPragmaDeclaration:
            visitSgPragmaDeclaration(isSgPragmaDeclaration(node), stmt);
        break;

        case V_SgGlobal:
            visitSgGlobal(isSgGlobal(node), stmt);
        break;

        case V_SgBasicBlock:
            visitSgBasicBlock(isSgBasicBlock(node), stmt);
        break;

        case V_SgFunctionDeclaration:
            visitSgFunctionDeclaration(isSgFunctionDeclaration(node), stmt);
        break;

        case V_SgNullExpression:
            visitSgNullExpression(isSgNullExpression(node), stmt);
        break;

        case V_SgNullStatement:
            visitSgNullStatement(isSgNullStatement(node), stmt);
        break;

        case V_SgBoolValExp:
            visitSgBoolValExp(isSgBoolValExp(node), stmt);
        break;

        case V_SgCharVal:
            visitSgCharVal(isSgCharVal(node), stmt);
        break;

        case V_SgDoubleVal:
            visitSgDoubleVal(isSgDoubleVal(node), stmt);
        break;

        case V_SgEnumVal:
            visitSgEnumVal(isSgEnumVal(node), stmt);
        break;

        case V_SgFloatVal:
            visitSgFloatVal(isSgFloatVal(node), stmt);
        break;

        case V_SgIntVal:
            visitSgIntVal(isSgIntVal(node), stmt);
        break;

        case V_SgLongDoubleVal:
            visitSgLongDoubleVal(isSgLongDoubleVal(node), stmt);
        break;

        case V_SgLongIntVal:
            visitSgLongIntVal(isSgLongIntVal(node), stmt);
        break;

        case V_SgLongLongIntVal:
            visitSgLongLongIntVal(isSgLongLongIntVal(node), stmt);
        break;

        case V_SgShortVal:
            visitSgShortVal(isSgShortVal(node), stmt);
        break;

        case V_SgUnsignedCharVal:
            visitSgUnsignedCharVal(isSgUnsignedCharVal(node), stmt);
        break;

        case V_SgUnsignedIntVal:
            visitSgUnsignedIntVal(isSgUnsignedIntVal(node), stmt);
        break;

        case V_SgUnsignedLongLongIntVal:
            visitSgUnsignedLongLongIntVal(isSgUnsignedLongLongIntVal(node),
                stmt);
        break;

        case V_SgUnsignedLongVal:
            visitSgUnsignedLongVal(isSgUnsignedLongVal(node), stmt);
        break;

        case V_SgUnsignedShortVal:
            visitSgUnsignedShortVal(isSgUnsignedShortVal(node), stmt);
        break;

        case V_SgWcharVal:
            visitSgWcharVal(isSgWcharVal(node), stmt);
        break;

        case V_SgComplexVal:
            visitSgComplexVal(isSgComplexVal(node), stmt);
        break;
    }
}

void MREVisitor::visitSgStatementExpression(
    SgStatementExpression *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgStatementExpression visit function" << endl;
}

void MREVisitor::visitSgExprListExp(
    SgExprListExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgExprListExp visit function" << endl;
}

void MREVisitor::visitSgVarRefExp(
    SgVarRefExp *node, OA::StmtHandle stmt)
{
    /**
     * The variable specified by the l-value of the left hand side expression
     * is assigned to the value of the right hand side expression.
     *
     * common post-conditions:
     *    - A memref handle is emitted for the node (and related to stmt)
     * common intermediate conditions:
     *    - A namedref MRE is constructed representing the variable access
     *    - A sym-handle is captured for the variable
     * special case (is for a reference variable):
     *    - a deref MRE wraps around the NamedRef MRE, and is asscociated with
     *      this statement
     * normal post-conditions:
     *    - the namedref MRE is asscociated with this statement
     *
     * Example normal case:
     *   Source:
     *     1) int i;
     *     2) i = 5;
     *   AST (for line 2):
     *     SgExpressionStatement
     *       SgAssignOp
     *          SgVarRefExp     <-----
     *       SgIntVal
     *   CToOA output after this function
     *      MEMREFEXPRS = { StmtHandle("i = 5;") =>
     *         [
     *              MemRefHandle("i") => 
     *                  NamedRef( USE, SymHandle("i"))
     *         ] }
     *
     *  Example special case (for access to reference var)
     *   Source:
     *     1) int val;
     *     2) int &ref = val;
     *     3) ref = 5;
     *   AST (for line 3):
     *      SgExpressionStatement
     *       SgAssignOp
     *          SgVarRefExp     <-----
     *       SgIntVal
     *   CToOA:
     *       MEMREFEXPRS = { StmtHandle("ref = 5;") =>
     *          [
     *              MemRefHandle("refRelatedBaseOrTmp") => 
     *                  NamedRef( USE, SymHandle("ref"))
     *              MemRefHandle("ref") => 
     *                  Deref( USE, NamedRef( USE, SymHandle("ref")), 1)
     *          ] }
     *    Notes:
     *      - The AST for this example is the same as the AST for the normal
     *        example.  The way to differentiate between the two cases
     *        is through fields in SgVarRefExp to determine if is a reference
     *        variable or not.
     */

    //-----------------------------------------------------------------------
    // common post-conditions:
    MemRefHandle hMemRef;
    hMemRef = mIR.getMemRefHandle(node);
    mIR.relateMemRefAndStmt(hMemRef, stmt);
    OA_ptr<MemRefExpr> mre;
    
    //-----------------------------------------------------------------------
    // common intermediate conditions:
    SymHandle hSymRef;
    OA_ptr<NamedRef> namedRef;
    
    SgVariableSymbol *symbol = node->get_symbol();
    ROSE_ASSERT(symbol != NULL);
    SgInitializedName *initName = symbol->get_declaration();
    ROSE_ASSERT(initName != NULL);
    OA::SymHandle sym = mIR.getNodeNumber(initName);
 
    namedRef = new NamedRef(MemRefExpr::USE, sym, false);
    mre = namedRef;
    
    //-----------------------------------------------------------------------
    // Special case (variable is a reference variable)
    SgType *type = getBaseType(node->get_type());
    if(isSgReferenceType(type)) {
        // wrap the NamedRef in a Deref, but before doing so also
        // create a MemRefHandle/MRE pair for the reference itself.
        // Use the Sg_File_Info as the MemRefHandle.  This should
        // effectively hide the MRE from anyone calling findTopMemRefHandle,
        // e.g., to create ptr assign pairs.  That's good, because the
        // deref should be the top memrefhandle.
        Sg_File_Info *fileInfo = node->get_file_info();
        ROSE_ASSERT(fileInfo != NULL);
            MemRefHandle hiddenMemref = mIR.getMemRefHandle(fileInfo);
        mIR.relateMemRefAndStmt(hiddenMemref, stmt);
//        mMemref2mreSetMap[hiddenMemref].insert(mre);
        cerr << "not handeling var ref special case" << endl;
        assert(false);

        int numderefs = 1;
//        mre = derefMre(mre, MemRefExpr::USE, numderefs);

        // It is the deref that we will see as the top mem ref handle
        // and ask whether the access it reprsents is to a reference.
//        mMre2TypeMap[mre] = reference;

//        mMemref2mreSetMap[hMemRef].insert(reference);
    }

    //-----------------------------------------------------------------------
    // Normal case
    else {
        mIR.relateMemRefAndMRE(hMemRef, namedRef);
    }
}

void MREVisitor::visitSgClassNameRefExp(
    SgClassNameRefExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgClassNameRefExp visit function" << endl;
}

void MREVisitor::visitSgFunctionRefExp(
    SgFunctionRefExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgFunctionRefExp visit function" << endl;
}

void MREVisitor::visitSgMemberFunctionRefExp(
    SgMemberFunctionRefExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgMemberFunctionRefExp visit function" << endl;
}

void MREVisitor::visitSgFunctionCallExp(
    SgFunctionCallExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgFunctionCallExp visit function" << endl;
}

void MREVisitor::visitSgSizeOfOp(
    SgSizeOfOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgSizeOfOp visit function" << endl;
}

void MREVisitor::visitSgConditionalExp(
    SgConditionalExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgConditionalExp visit function" << endl;
}

void MREVisitor::visitSgNewExp(
    SgNewExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNewExp visit function" << endl;
}

void MREVisitor::visitSgDeleteExp(
    SgDeleteExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDeleteExp visit function" << endl;
}

void MREVisitor::visitSgThisExp(
    SgThisExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgThisExp visit function" << endl;
}

void MREVisitor::visitSgVarArgStartOp(
    SgVarArgStartOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgVarArgStartOp visit function" << endl;
}

void MREVisitor::visitSgVarArgCopyOp(
    SgVarArgCopyOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgVarArgCopyOp visit function" << endl;
}

void MREVisitor::visitSgVarArgStartOneOperandOp(
    SgVarArgStartOneOperandOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgVarArgStartOneOperandOp visit function" << endl;
}

void MREVisitor::visitSgVarArgOp(
    SgVarArgOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgVarArgOp visit function" << endl;
}

void MREVisitor::visitSgVarArgEndOp(
    SgVarArgEndOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgVarArgEndOp visit function" << endl;
}

void MREVisitor::visitSgInitializedName(
    SgInitializedName *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgInitializedName visit function" << endl;
}

void MREVisitor::visitSgAggregateInitializer(
    SgAggregateInitializer *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgAggregateInitializer visit function" << endl;
}

void MREVisitor::visitSgConstructorInitializer(
    SgConstructorInitializer *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgConstructorInitializer visit function" << endl;
}

void MREVisitor::visitSgAssignInitializer(
    SgAssignInitializer *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgAssignInitializer visit function" << endl;
}

void MREVisitor::visitSgArrowExp(
    SgArrowExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgArrowExp visit function" << endl;
}

void MREVisitor::visitSgDotExp(
    SgDotExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDotExp visit function" << endl;
}

void MREVisitor::visitSgDotStarOp(
    SgDotStarOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDotStarOp visit function" << endl;
}

void MREVisitor::visitSgArrowStarOp(
    SgArrowStarOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgArrowStarOp visit function" << endl;
}

void MREVisitor::visitSgAddOp(
    SgAddOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgAddOp visit function" << endl;
}

void MREVisitor::visitSgSubtractOp(
    SgSubtractOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgSubtractOp visit function" << endl;
}

void MREVisitor::visitSgEqualityOp(
    SgEqualityOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgEqualityOp visit function" << endl;
}

void MREVisitor::visitSgLessThanOp(
    SgLessThanOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLessThanOp visit function" << endl;
}

void MREVisitor::visitSgGreaterThanOp(
    SgGreaterThanOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgGreaterThanOp visit function" << endl;
}

void MREVisitor::visitSgNotEqualOp(
    SgNotEqualOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNotEqualOp visit function" << endl;
}

void MREVisitor::visitSgLessOrEqualOp(
    SgLessOrEqualOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLessOrEqualOp visit function" << endl;
}

void MREVisitor::visitSgGreaterOrEqualOp(
    SgGreaterOrEqualOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgGreaterOrEqualOp visit function" << endl;
}

void MREVisitor::visitSgMultiplyOp(
    SgMultiplyOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgMultiplyOp visit function" << endl;
}

void MREVisitor::visitSgDivideOp(
    SgDivideOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDivideOp visit function" << endl;
}

void MREVisitor::visitSgIntegerDivideOp(
    SgIntegerDivideOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgIntegerDivideOp visit function" << endl;
}

void MREVisitor::visitSgModOp(
    SgModOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgModOp visit function" << endl;
}

void MREVisitor::visitSgAndOp(
    SgAndOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgAndOp visit function" << endl;
}

void MREVisitor::visitSgOrOp(
    SgOrOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgOrOp visit function" << endl;
}

void MREVisitor::visitSgBitXorOp(
    SgBitXorOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgBitXorOp visit function" << endl;
}

void MREVisitor::visitSgBitAndOp(
    SgBitAndOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgBitAndOp visit function" << endl;
}

void MREVisitor::visitSgBitOrOp(
    SgBitOrOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgBitOrOp visit function" << endl;
}

void MREVisitor::visitSgLshiftOp(
    SgLshiftOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLshiftOp visit function" << endl;
}

void MREVisitor::visitSgRshiftOp(
    SgRshiftOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgRshiftOp visit function" << endl;
}

void MREVisitor::visitSgCommaOpExp(
    SgCommaOpExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgCommaOpExp visit function" << endl;
}

void MREVisitor::visitSgPntrArrRefExp(
    SgPntrArrRefExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgPntrArrRefExp visit function" << endl;
}

void MREVisitor::visitSgAssignOp(
    SgAssignOp *node, OA::StmtHandle stmt)
{
    /**
     * The variable reference represented by this node may be for both an
     * DEF (lhs of an assignment) or a USE.  At this point of the traversal we
     * assume it be a USE.
     *
     * A special case occurs when the variable being accessed is a reference
     * variable.  In this case the access should be modeled as a dereference.
     *
     * common post-conditions:
     *    - A memref handle relates to this statement
     *    - all MRE's on the lhs have MRType DEF
     *    - an assign pair is emitted relating the memrefhandle of the lhs
     *      and the exprhandle of the rhs
     * special case (assignment of a pointer):
     *    - a pointer assign pair is created
     *
     * Example normal case:
     *   Source:
     *     1) int i;
     *     2) i = 5;
     *   AST (for line 2):
     *     SgExpressionStatement
     *       SgAssignOp         <-----
     *          SgVarRefExp     
     *       SgIntVal
     *   CToOA output after this function
     *      MEMREFEXPRS = { StmtHandle("i = 5;") =>
     *         [
     *              MemRefHandle("i") => 
     *                  NamedRef( DEF, SymHandle("i"))
     *         ] }
     *     ASSIGNPAIRS = { StmtHandle("i = 5;") =>
     *         [
     *              < MemRefHandle("i")
     *              , ExprHandle("5") >
     *         ] }
    **/
    
    //-----------------------------------------------------------------------
    // common post-conditions:
    MemRefHandle hMemRef;
    hMemRef = mIR.getMemRefHandle(node);
    mIR.relateMemRefAndStmt(hMemRef, stmt);

    // change the MRType of all MRE's on the lhs to DEF.
    MemRefHandle lhs_memref = mIR.getMemRefHandle(node->get_lhs_operand());
    OA_ptr<OA::MemRefExprIterator> mIter =
        mIR.getMemRefExprIterator(lhs_memref);
    for (mIter->reset(); mIter->isValid(); ++(*mIter) ) {
        OA_ptr<MemRefExpr> lhs_mre = mIter->current();
        lhs_mre->setMemRefType(MemRefExpr::DEF);
    }

    // create an assign pair for this node
    ExprHandle rhs_expr = mIR.getMemRefHandle(node->get_rhs_operand());
    mIR.makeAssignPair(stmt, lhs_memref, rhs_expr);


    //-----------------------------------------------------------------------
    // special case (assignment of a pointer):
    SgType *lhs_type = getBaseType(node->get_lhs_operand()->get_type());
    MemRefHandle rhs_memref = mIR.findTopMemRefHandle(node->get_rhs_operand());

    // if the RHS is a memory reference (as opposed to a value)
    if(rhs_memref != MemRefHandle(0) && isSgPointerType(lhs_type)) {
        // iterate through mre's on the lhs
        MemRefHandle lhs_memref = mIR.getMemRefHandle(node->get_lhs_operand());
        OA_ptr<MemRefExprIterator> mIter =
            mIR.getMemRefExprIterator(lhs_memref);
        for (mIter->reset(); mIter->isValid(); ++(*mIter) ) {
            OA_ptr<MemRefExpr> lhs_mre = mIter->current();

            // iterate through mre's on the rhs
            OA_ptr<MemRefExprIterator> rhsIter;
            rhsIter = mIR.getMemRefExprIterator(rhs_memref);
            for(rhsIter->reset(); rhsIter->isValid(); ++(*rhsIter)) {
                OA_ptr<MemRefExpr> rhs_mre = rhsIter->current();
                mIR.makePtrAssignPair(stmt, lhs_mre, rhs_mre);
            }
        }
    }
}

void MREVisitor::visitSgPlusAssignOp(
    SgPlusAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgPlusAssignOp visit function" << endl;
}

void MREVisitor::visitSgMinusAssignOp(
    SgMinusAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgMinusAssignOp visit function" << endl;
}

void MREVisitor::visitSgAndAssignOp(
    SgAndAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgAndAssignOp visit function" << endl;
}

void MREVisitor::visitSgIorAssignOp(
    SgIorAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgIorAssignOp visit function" << endl;
}

void MREVisitor::visitSgMultAssignOp(
    SgMultAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgMultAssignOp visit function" << endl;
}

void MREVisitor::visitSgDivAssignOp(
    SgDivAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDivAssignOp visit function" << endl;
}

void MREVisitor::visitSgModAssignOp(
    SgModAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgModAssignOp visit function" << endl;
}

void MREVisitor::visitSgXorAssignOp(
    SgXorAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgXorAssignOp visit function" << endl;
}

void MREVisitor::visitSgLshiftAssignOp(
    SgLshiftAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLshiftAssignOp visit function" << endl;
}

void MREVisitor::visitSgRshiftAssignOp(
    SgRshiftAssignOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgRshiftAssignOp visit function" << endl;
}

void MREVisitor::visitSgExpressionRoot(
    SgExpressionRoot *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgExpressionRoot visit function" << endl;
}

void MREVisitor::visitSgMinusOp(
    SgMinusOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgMinusOp visit function" << endl;
}

void MREVisitor::visitSgUnaryAddOp(
    SgUnaryAddOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUnaryAddOp visit function" << endl;
}

void MREVisitor::visitSgNotOp(
    SgNotOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNotOp visit function" << endl;
}

void MREVisitor::visitSgBitComplementOp(
    SgBitComplementOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgBitComplementOp visit function" << endl;
}

void MREVisitor::visitSgCastExp(
    SgCastExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgCastExp visit function" << endl;
}

void MREVisitor::visitSgThrowOp(
    SgThrowOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgThrowOp visit function" << endl;
}

void MREVisitor::visitSgTypeIdOp(
    SgTypeIdOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgTypeIdOp visit function" << endl;
}

void MREVisitor::visitSgPointerDerefExp(
    SgPointerDerefExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgPointerDerefExp visit function" << endl;
}

void MREVisitor::visitSgAddressOfOp(
    SgAddressOfOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgAddressOfOp visit function" << endl;
}

void MREVisitor::visitSgMinusMinusOp(
    SgMinusMinusOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgMinusMinusOp visit function" << endl;
}

void MREVisitor::visitSgPlusPlusOp(
    SgPlusPlusOp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgPlusPlusOp visit function" << endl;
}

void MREVisitor::visitSgExprStatement(
    SgExprStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgExprStatement visit function" << endl;
}

void MREVisitor::visitSgTryStmt(
    SgTryStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgTryStmt visit function" << endl;
}

void MREVisitor::visitSgReturnStmt(
    SgReturnStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgReturnStmt visit function" << endl;
}

void MREVisitor::visitSgSpawnStmt(
    SgSpawnStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgSpawnStmt visit function" << endl;
}

void MREVisitor::visitSgVariableDeclaration(
    SgVariableDeclaration *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgVariableDeclaration visit function" << endl;
}

void MREVisitor::visitSgVariableDefinition(
    SgVariableDefinition *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgVariableDefinition visit function" << endl;
}

void MREVisitor::visitSgAsmStmt(
    SgAsmStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgAsmStmt visit function" << endl;
}

void MREVisitor::visitSgFunctionParameterList(
    SgFunctionParameterList *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgFunctionParameterList visit function" << endl;
}

void MREVisitor::visitSgCtorInitializerList(
    SgCtorInitializerList *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgCtorInitializerList visit function" << endl;
}

void MREVisitor::visitSgIfStmt(
    SgIfStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgIfStmt visit function" << endl;
}

void MREVisitor::visitSgSwitchStatement(
    SgSwitchStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgSwitchStatement visit function" << endl;
}

void MREVisitor::visitSgForStatement(
    SgForStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgForStatement visit function" << endl;
}

void MREVisitor::visitSgForInitStatement(
    SgForInitStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgForInitStatement visit function" << endl;
}

void MREVisitor::visitSgWhileStmt(
    SgWhileStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgWhileStmt visit function" << endl;
}

void MREVisitor::visitSgDoWhileStmt(
    SgDoWhileStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDoWhileStmt visit function" << endl;
}

void MREVisitor::visitSgCatchOptionStmt(
    SgCatchOptionStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgCatchOptionStmt visit function" << endl;
}

void MREVisitor::visitSgNamespaceDefinitionStatement(
    SgNamespaceDefinitionStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNamespaceDefinitionStatement visit function" << endl;
}

void MREVisitor::visitSgClassDeclaration(
    SgClassDeclaration *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgClassDeclaration visit function" << endl;
}

void MREVisitor::visitSgClassDefinition(
    SgClassDefinition *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgClassDefinition visit function" << endl;
}

void MREVisitor::visitSgStringVal(
    SgStringVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgStringVal visit function" << endl;
}

void MREVisitor::visitSgFunctionDefinition(
    SgFunctionDefinition *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgFunctionDefinition visit function" << endl;
}

void MREVisitor::visitSgLabelStatement(
    SgLabelStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLabelStatement visit function" << endl;
}

void MREVisitor::visitSgDefaultOptionStmt(
    SgDefaultOptionStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDefaultOptionStmt visit function" << endl;
}

void MREVisitor::visitSgBreakStmt(
    SgBreakStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgBreakStmt visit function" << endl;
}

void MREVisitor::visitSgContinueStmt(
    SgContinueStmt *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgContinueStmt visit function" << endl;
}

void MREVisitor::visitSgGotoStatement(
    SgGotoStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgGotoStatement visit function" << endl;
}

void MREVisitor::visitSgCatchStatementSeq(
    SgCatchStatementSeq *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgCatchStatementSeq visit function" << endl;
}

void MREVisitor::visitSgClinkageStartStatement(
    SgClinkageStartStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgClinkageStartStatement visit function" << endl;
}

void MREVisitor::visitSgEnumDeclaration(
    SgEnumDeclaration *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgEnumDeclaration visit function" << endl;
}

void MREVisitor::visitSgTemplateDeclaration(
    SgTemplateDeclaration *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgTemplateDeclaration visit function" << endl;
}

void MREVisitor::visitSgTypedefDeclaration(
    SgTypedefDeclaration *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgTypedefDeclaration visit function" << endl;
}

void MREVisitor::visitSgNamespaceDeclarationStatement(
    SgNamespaceDeclarationStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNamespaceDeclarationStatement visit function" << endl;
}

void MREVisitor::visitSgNamespaceAliasDeclarationStatement(
    SgNamespaceAliasDeclarationStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNamespaceAliasDeclarationStatement visit function" << endl;
}

void MREVisitor::visitSgUsingDirectiveStatement(
    SgUsingDirectiveStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUsingDirectiveStatement visit function" << endl;
}

void MREVisitor::visitSgUsingDeclarationStatement(
    SgUsingDeclarationStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUsingDeclarationStatement visit function" << endl;
}

void MREVisitor::visitSgPragmaDeclaration(
    SgPragmaDeclaration *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgPragmaDeclaration visit function" << endl;
}

void MREVisitor::visitSgGlobal(
    SgGlobal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgGlobal visit function" << endl;
}

void MREVisitor::visitSgBasicBlock(
    SgBasicBlock *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgBasicBlock visit function" << endl;
}

void MREVisitor::visitSgFunctionDeclaration(
    SgFunctionDeclaration *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgFunctionDeclaration visit function" << endl;
}

void MREVisitor::visitSgNullExpression(
    SgNullExpression *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNullExpression visit function" << endl;
}

void MREVisitor::visitSgNullStatement(
    SgNullStatement *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgNullStatement visit function" << endl;
}

void MREVisitor::visitSgBoolValExp(
    SgBoolValExp *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgBoolValExp visit function" << endl;
}

void MREVisitor::visitSgCharVal(
    SgCharVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgCharVal visit function" << endl;
}

void MREVisitor::visitSgDoubleVal(
    SgDoubleVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgDoubleVal visit function" << endl;
}

void MREVisitor::visitSgEnumVal(
    SgEnumVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgEnumVal visit function" << endl;
}

void MREVisitor::visitSgFloatVal(
    SgFloatVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgFloatVal visit function" << endl;
}

void MREVisitor::visitSgIntVal(
    SgIntVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgIntVal visit function" << endl;
}

void MREVisitor::visitSgLongDoubleVal(
    SgLongDoubleVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLongDoubleVal visit function" << endl;
}

void MREVisitor::visitSgLongIntVal(
    SgLongIntVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLongIntVal visit function" << endl;
}

void MREVisitor::visitSgLongLongIntVal(
    SgLongLongIntVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgLongLongIntVal visit function" << endl;
}

void MREVisitor::visitSgShortVal(
    SgShortVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgShortVal visit function" << endl;
}

void MREVisitor::visitSgUnsignedCharVal(
    SgUnsignedCharVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUnsignedCharVal visit function" << endl;
}

void MREVisitor::visitSgUnsignedIntVal(
    SgUnsignedIntVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUnsignedIntVal visit function" << endl;
}

void MREVisitor::visitSgUnsignedLongLongIntVal(
    SgUnsignedLongLongIntVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUnsignedLongLongIntVal visit function" << endl;
}

void MREVisitor::visitSgUnsignedLongVal(
    SgUnsignedLongVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUnsignedLongVal visit function" << endl;
}

void MREVisitor::visitSgUnsignedShortVal(
    SgUnsignedShortVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgUnsignedShortVal visit function" << endl;
}

void MREVisitor::visitSgWcharVal(
    SgWcharVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "SgWcharVal visit function" << endl;
}

void MREVisitor::visitSgComplexVal(
    SgComplexVal *node, OA::StmtHandle stmt)
{
    //cout << "in a " << "MREVisito visit function" << endl;
}

/**
 * Notes on reading visit function comments:
 *  The output of the examples reflects the state after the function finishes,
 *  not neccessarily what the final result will be once CToOA has completed.
 */
