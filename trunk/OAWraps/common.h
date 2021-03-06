
#ifndef _USEOA_COMMON_H
#define _USEOA_COMMON_H

#include "rose.h"
#include "CallGraph.h"
#include <OpenAnalysis/IRInterface/IRHandles.hpp>


namespace UseOA {

bool isNonStaticMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp);

SgPointerDerefExp *isFunctionPointer(SgFunctionCallExp *functionCall);

SgFunctionDeclaration *getFunctionDeclaration(SgFunctionCallExp *functionCall);

void getFormalTypes(SgNode *node, std::vector<SgType *> &formalTypes);

void verifyCallHandleNodeType(SgNode *node);

void verifySymHandleNodeType(SgNode *node);

void verifyStmtHandleNodeType(SgNode *node);

SgMemberFunctionDeclaration *
lookupDestructorInClass(SgClassDefinition *classDefinition);

void getActuals(SgNode *node, std::list<SgNode *> &actuals);

//! Return the method in which node occurs.
SgFunctionDefinition *getEnclosingFunction(SgNode *node);

void getTypeInfo(SgType *t, std::string *tname,
                 std::string* stripname, int* size = 0);

struct eqTypes
{
    bool operator()(SgType* t1, SgType* t2) const
    {
        std::string t1Str, t2Str;
        getTypeInfo(t1, 0, &t1Str);
        getTypeInfo(t2, 0, &t2Str);
        return ( t1Str == t2Str );
    }
};

bool classHasVirtualMethods(SgClassDefinition *classDefinition);

bool isVirtual(SgFunctionDeclaration *functionDeclaration);

bool isPureVirtual(SgFunctionDeclaration *functionDeclaration);

std::string mangleFunctionName(SgFunctionDeclaration *funcDecl);

SgClassDeclaration *
getClassDeclaration(SgType *type);

SgClassDefinition *
getClassDefinition(SgType *type);

SgFunctionDeclaration *getDefiningDeclaration(SgFunctionDeclaration *funcDecl);
SgClassDeclaration *getDefiningDeclaration(SgClassDeclaration *funcDecl);

bool matchingFunctions(SgFunctionDeclaration *decl1,
                       SgFunctionDeclaration *decl2);

bool isFunc(SgFunctionCallExp *functionCallExp, std::string funcName);

bool returnsReference(SgFunctionCallExp *functionCallExp);

bool returnsAddress(SgFunctionCallExp *functionCallExp);

SgNode *getThisExpNode(SgNode *node);

bool isObjectInitialization(SgConstructorInitializer *ctorInitializer);

bool isBaseClassInvocation(SgConstructorInitializer *ctorInitializer);

SgNode *getConstructorInitializerLhs(SgConstructorInitializer *ctorInitializer);

SgMemberFunctionDeclaration *
getInvokedMethod(SgMemberFunctionRefExp *memberFunctionRefExp);

bool
isNonStaticMethod(SgMemberFunctionDeclaration *memberFunctionDecl);

bool
isVaStart(SgFunctionCallExp *functionCallExp);

SgType* getBaseType(SgType *type);

SgType* lookThruReferenceType(SgType *type);

//bool isConstType(SgType *type);

//bool isNonconstReference(SgType *type);

bool isArrowExp(SgExpression *function);

bool hasDefinition(SgFunctionDeclaration *functionDeclaration);

bool isConstructor(SgMemberFunctionDeclaration *memberFunctionDeclaration);

//SHK - This has been reimplemented in canon and xaif
SgGlobal * globalScope(SgFile * file);

bool isDestructor(SgMemberFunctionDeclaration *memberFunctionDeclaration);

bool findNodeInSubtree(SgNode *needle, SgNode *haystack);

bool isPlacementNew(SgNewExp *newExp);

SgFunctionDeclaration *isPlacementDelete(SgDeleteExp *deleteExp);

bool isOperatorNew(SgFunctionDeclaration *functionDeclaration);

std::set<SgFunctionDeclaration *>
getFunctionsConsistentWithMethodPointerInvocation(SgBinaryOp *dotStarOrArrowStar,
                                                  ClassHierarchyWrapper *classHierarchy);

} // end of namespace UseOA

#endif /* _USEOA_COMMON_H */
