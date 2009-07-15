#ifndef MemSage2OA_H
#define MemSage2OA_H

#include <list>
#include <OpenAnalysis/IRInterface/AliasIRInterface.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include "rose.h"

#include "Sage2OA.h"


class SageIRMemRefIterator : public OA::MemRefHandleIterator 
{
  public:
    SageIRMemRefIterator(OA::StmtHandle h, SageIRInterface& ir);
    virtual ~SageIRMemRefIterator() { };
  
    virtual OA::MemRefHandle current() const; 
    virtual bool isValid() const { 
      return ( mValid && ( mMemRefIter != mEnd ) ); 
    }
          
    virtual void operator++();
    virtual void reset();

  private:
    void create(OA::StmtHandle h);

    std::list<OA::MemRefHandle> mMemRefList;
    
    std::list<OA::MemRefHandle>::iterator mEnd;
    std::list<OA::MemRefHandle>::iterator mBegin;
    std::list<OA::MemRefHandle>::iterator mMemRefIter;
    bool mValid;
    SageIRInterface& mIR;
};

// This is identical to Open64MemRefHandleIterator.
class SageMemRefHandleIterator
: public OA::IRHandleListIterator<OA::MemRefHandle>,
  public virtual OA::MemRefHandleIterator
{
 public:
  SageMemRefHandleIterator (OA::OA_ptr<std::list<OA::MemRefHandle> > pList);
  
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


class MREVisitor {
  public:
    void visit(SgNode *node, OA::StmtHandle stmt);

  private:
    void visitSgStatementExpression(
        SgStatementExpression *node, OA::StmtHandle stmt);

    void visitSgExprListExp(
        SgExprListExp *node, OA::StmtHandle stmt);

    void visitSgVarRefExp(
        SgVarRefExp *node, OA::StmtHandle stmt);

    void visitSgClassNameRefExp(
        SgClassNameRefExp *node, OA::StmtHandle stmt);

    void visitSgFunctionRefExp(
        SgFunctionRefExp *node, OA::StmtHandle stmt);

    void visitSgMemberFunctionRefExp(
        SgMemberFunctionRefExp *node, OA::StmtHandle stmt);

    void visitSgFunctionCallExp(
        SgFunctionCallExp *node, OA::StmtHandle stmt);

    void visitSgSizeOfOp(
        SgSizeOfOp *node, OA::StmtHandle stmt);

    void visitSgConditionalExp(
        SgConditionalExp *node, OA::StmtHandle stmt);

    void visitSgNewExp(
        SgNewExp *node, OA::StmtHandle stmt);

    void visitSgDeleteExp(
        SgDeleteExp *node, OA::StmtHandle stmt);

    void visitSgThisExp(
        SgThisExp *node, OA::StmtHandle stmt);

    void visitSgVarArgStartOp(
        SgVarArgStartOp *node, OA::StmtHandle stmt);

    void visitSgVarArgCopyOp(
        SgVarArgCopyOp *node, OA::StmtHandle stmt);

    void visitSgVarArgStartOneOperandOp(
        SgVarArgStartOneOperandOp *node, OA::StmtHandle stmt);

    void visitSgVarArgOp(
        SgVarArgOp *node, OA::StmtHandle stmt);

    void visitSgVarArgEndOp(
        SgVarArgEndOp *node, OA::StmtHandle stmt);

    void visitSgInitializedName(
        SgInitializedName *node, OA::StmtHandle stmt);

    void visitSgAggregateInitializer(
        SgAggregateInitializer *node, OA::StmtHandle stmt);

    void visitSgConstructorInitializer(
        SgConstructorInitializer *node, OA::StmtHandle stmt);

    void visitSgAssignInitializer(
        SgAssignInitializer *node, OA::StmtHandle stmt);

    void visitSgArrowExp(
        SgArrowExp *node, OA::StmtHandle stmt);

    void visitSgDotExp(
        SgDotExp *node, OA::StmtHandle stmt);

    void visitSgDotStarOp(
        SgDotStarOp *node, OA::StmtHandle stmt);

    void visitSgArrowStarOp(
        SgArrowStarOp *node, OA::StmtHandle stmt);

    void visitSgAddOp(
        SgAddOp *node, OA::StmtHandle stmt);

    void visitSgSubtractOp(
        SgSubtractOp *node, OA::StmtHandle stmt);

    void visitSgEqualityOp(
        SgEqualityOp *node, OA::StmtHandle stmt);

    void visitSgLessThanOp(
        SgLessThanOp *node, OA::StmtHandle stmt);

    void visitSgGreaterThanOp(
        SgGreaterThanOp *node, OA::StmtHandle stmt);

    void visitSgNotEqualOp(
        SgNotEqualOp *node, OA::StmtHandle stmt);

    void visitSgLessOrEqualOp(
        SgLessOrEqualOp *node, OA::StmtHandle stmt);

    void visitSgGreaterOrEqualOp(
        SgGreaterOrEqualOp *node, OA::StmtHandle stmt);

    void visitSgMultiplyOp(
        SgMultiplyOp *node, OA::StmtHandle stmt);

    void visitSgDivideOp(
        SgDivideOp *node, OA::StmtHandle stmt);

    void visitSgIntegerDivideOp(
        SgIntegerDivideOp *node, OA::StmtHandle stmt);

    void visitSgModOp(
        SgModOp *node, OA::StmtHandle stmt);

    void visitSgAndOp(
        SgAndOp *node, OA::StmtHandle stmt);

    void visitSgOrOp(
        SgOrOp *node, OA::StmtHandle stmt);

    void visitSgBitXorOp(
        SgBitXorOp *node, OA::StmtHandle stmt);

    void visitSgBitAndOp(
        SgBitAndOp *node, OA::StmtHandle stmt);

    void visitSgBitOrOp(
        SgBitOrOp *node, OA::StmtHandle stmt);

    void visitSgLshiftOp(
        SgLshiftOp *node, OA::StmtHandle stmt);

    void visitSgRshiftOp(
        SgRshiftOp *node, OA::StmtHandle stmt);

    void visitSgCommaOpExp(
        SgCommaOpExp *node, OA::StmtHandle stmt);

    void visitSgPntrArrRefExp(
        SgPntrArrRefExp *node, OA::StmtHandle stmt);

    void visitSgAssignOp(
        SgAssignOp *node, OA::StmtHandle stmt);

    void visitSgPlusAssignOp(
        SgPlusAssignOp *node, OA::StmtHandle stmt);

    void visitSgMinusAssignOp(
        SgMinusAssignOp *node, OA::StmtHandle stmt);

    void visitSgAndAssignOp(
        SgAndAssignOp *node, OA::StmtHandle stmt);

    void visitSgIorAssignOp(
        SgIorAssignOp *node, OA::StmtHandle stmt);

    void visitSgMultAssignOp(
        SgMultAssignOp *node, OA::StmtHandle stmt);

    void visitSgDivAssignOp(
        SgDivAssignOp *node, OA::StmtHandle stmt);

    void visitSgModAssignOp(
        SgModAssignOp *node, OA::StmtHandle stmt);

    void visitSgXorAssignOp(
        SgXorAssignOp *node, OA::StmtHandle stmt);

    void visitSgLshiftAssignOp(
        SgLshiftAssignOp *node, OA::StmtHandle stmt);

    void visitSgRshiftAssignOp(
        SgRshiftAssignOp *node, OA::StmtHandle stmt);

    void visitSgExpressionRoot(
        SgExpressionRoot *node, OA::StmtHandle stmt);

    void visitSgMinusOp(
        SgMinusOp *node, OA::StmtHandle stmt);

    void visitSgUnaryAddOp(
        SgUnaryAddOp *node, OA::StmtHandle stmt);

    void visitSgNotOp(
        SgNotOp *node, OA::StmtHandle stmt);

    void visitSgBitComplementOp(
        SgBitComplementOp *node, OA::StmtHandle stmt);

    void visitSgCastExp(
        SgCastExp *node, OA::StmtHandle stmt);

    void visitSgThrowOp(
        SgThrowOp *node, OA::StmtHandle stmt);

    void visitSgTypeIdOp(
        SgTypeIdOp *node, OA::StmtHandle stmt);

    void visitSgPointerDerefExp(
        SgPointerDerefExp *node, OA::StmtHandle stmt);

    void visitSgAddressOfOp(
        SgAddressOfOp *node, OA::StmtHandle stmt);

    void visitSgMinusMinusOp(
        SgMinusMinusOp *node, OA::StmtHandle stmt);

    void visitSgPlusPlusOp(
        SgPlusPlusOp *node, OA::StmtHandle stmt);

    void visitSgExprStatement(
        SgExprStatement *node, OA::StmtHandle stmt);

    void visitSgTryStmt(
        SgTryStmt *node, OA::StmtHandle stmt);

    void visitSgReturnStmt(
        SgReturnStmt *node, OA::StmtHandle stmt);

    void visitSgSpawnStmt(
        SgSpawnStmt *node, OA::StmtHandle stmt);

    void visitSgVariableDeclaration(
        SgVariableDeclaration *node, OA::StmtHandle stmt);

    void visitSgVariableDefinition(
        SgVariableDefinition *node, OA::StmtHandle stmt);

    void visitSgAsmStmt(
        SgAsmStmt *node, OA::StmtHandle stmt);

    void visitSgFunctionParameterList(
        SgFunctionParameterList *node, OA::StmtHandle stmt);

    void visitSgCtorInitializerList(
        SgCtorInitializerList *node, OA::StmtHandle stmt);

    void visitSgIfStmt(
        SgIfStmt *node, OA::StmtHandle stmt);

    void visitSgSwitchStatement(
        SgSwitchStatement *node, OA::StmtHandle stmt);

    void visitSgForStatement(
        SgForStatement *node, OA::StmtHandle stmt);

    void visitSgForInitStatement(
        SgForInitStatement *node, OA::StmtHandle stmt);

    void visitSgWhileStmt(
        SgWhileStmt *node, OA::StmtHandle stmt);

    void visitSgDoWhileStmt(
        SgDoWhileStmt *node, OA::StmtHandle stmt);

    void visitSgCatchOptionStmt(
        SgCatchOptionStmt *node, OA::StmtHandle stmt);

    void visitSgNamespaceDefinitionStatement(
        SgNamespaceDefinitionStatement *node, OA::StmtHandle stmt);

    void visitSgClassDeclaration(
        SgClassDeclaration *node, OA::StmtHandle stmt);

    void visitSgClassDefinition(
        SgClassDefinition *node, OA::StmtHandle stmt);

    void visitSgStringVal(
        SgStringVal *node, OA::StmtHandle stmt);

    void visitSgFunctionDefinition(
        SgFunctionDefinition *node, OA::StmtHandle stmt);

    void visitSgLabelStatement(
        SgLabelStatement *node, OA::StmtHandle stmt);

    void visitSgDefaultOptionStmt(
        SgDefaultOptionStmt *node, OA::StmtHandle stmt);

    void visitSgBreakStmt(
        SgBreakStmt *node, OA::StmtHandle stmt);

    void visitSgContinueStmt(
        SgContinueStmt *node, OA::StmtHandle stmt);

    void visitSgGotoStatement(
        SgGotoStatement *node, OA::StmtHandle stmt);

    void visitSgCatchStatementSeq(
        SgCatchStatementSeq *node, OA::StmtHandle stmt);

    void visitSgClinkageStartStatement(
        SgClinkageStartStatement *node, OA::StmtHandle stmt);

    void visitSgEnumDeclaration(
        SgEnumDeclaration *node, OA::StmtHandle stmt);

    void visitSgTemplateDeclaration(
        SgTemplateDeclaration *node, OA::StmtHandle stmt);

    void visitSgTypedefDeclaration(
        SgTypedefDeclaration *node, OA::StmtHandle stmt);

    void visitSgNamespaceDeclarationStatement(
        SgNamespaceDeclarationStatement *node, OA::StmtHandle stmt);

    void visitSgNamespaceAliasDeclarationStatement(
        SgNamespaceAliasDeclarationStatement *node, OA::StmtHandle stmt);

    void visitSgUsingDirectiveStatement(
        SgUsingDirectiveStatement *node, OA::StmtHandle stmt);

    void visitSgUsingDeclarationStatement(
        SgUsingDeclarationStatement *node, OA::StmtHandle stmt);

    void visitSgPragmaDeclaration(
        SgPragmaDeclaration *node, OA::StmtHandle stmt);

    void visitSgGlobal(
        SgGlobal *node, OA::StmtHandle stmt);

    void visitSgBasicBlock(
        SgBasicBlock *node, OA::StmtHandle stmt);

    void visitSgFunctionDeclaration(
        SgFunctionDeclaration *node, OA::StmtHandle stmt);

    void visitSgNullExpression(
        SgNullExpression *node, OA::StmtHandle stmt);

    void visitSgNullStatement(
        SgNullStatement *node, OA::StmtHandle stmt);

    void visitSgBoolValExp(
        SgBoolValExp *node, OA::StmtHandle stmt);

    void visitSgCharVal(
        SgCharVal *node, OA::StmtHandle stmt);

    void visitSgDoubleVal(
        SgDoubleVal *node, OA::StmtHandle stmt);

    void visitSgEnumVal(
        SgEnumVal *node, OA::StmtHandle stmt);

    void visitSgFloatVal(
        SgFloatVal *node, OA::StmtHandle stmt);

    void visitSgIntVal(
        SgIntVal *node, OA::StmtHandle stmt);

    void visitSgLongDoubleVal(
        SgLongDoubleVal *node, OA::StmtHandle stmt);

    void visitSgLongIntVal(
        SgLongIntVal *node, OA::StmtHandle stmt);

    void visitSgLongLongIntVal(
        SgLongLongIntVal *node, OA::StmtHandle stmt);

    void visitSgShortVal(
        SgShortVal *node, OA::StmtHandle stmt);

    void visitSgUnsignedCharVal(
        SgUnsignedCharVal *node, OA::StmtHandle stmt);

    void visitSgUnsignedIntVal(
        SgUnsignedIntVal *node, OA::StmtHandle stmt);

    void visitSgUnsignedLongLongIntVal(
        SgUnsignedLongLongIntVal *node, OA::StmtHandle stmt);

    void visitSgUnsignedLongVal(
        SgUnsignedLongVal *node, OA::StmtHandle stmt);

    void visitSgUnsignedShortVal(
        SgUnsignedShortVal *node, OA::StmtHandle stmt);

    void visitSgWcharVal(
        SgWcharVal *node, OA::StmtHandle stmt);

    void visitSgComplexVal(
        SgComplexVal *node, OA::StmtHandle stmt);
};

#endif

