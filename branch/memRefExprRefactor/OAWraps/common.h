
#ifndef _USEOA_COMMON_H
#define _USEOA_COMMON_H

#include "rose.h"
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

bool isMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp);

SgPointerDerefExp *isFunctionPointer(SgFunctionCallExp *functionCall);

SgFunctionDeclaration *getFunctionDeclaration(SgFunctionCallExp *functionCall);

SgTypePtrList &getFormalTypes(SgNode *node);

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

std::string mangleFunctionName(SgFunctionDeclaration *funcDecl);

SgClassDeclaration *
getClassDeclaration(SgType *type);

SgClassDefinition *
getClassDefinition(SgType *type);

SgFunctionDeclaration *getDefiningDeclaration(SgFunctionDeclaration *funcDecl);
SgClassDeclaration *getDefiningDeclaration(SgClassDeclaration *funcDecl);

bool matchingFunctions(SgFunctionDeclaration *decl1, 
                       SgFunctionDeclaration *decl2);

bool isFunc(SgFunctionCallExp *functionCallExp,
                             char *funcName);

bool returnsAddress(SgFunctionCallExp *functionCallExp);

SgNode *getThisExpNode(SgNode *node);

bool isObjectInitialization(SgConstructorInitializer *ctorInitializer);

bool isBaseClassInvocation(SgConstructorInitializer *ctorInitializer);

SgNode *getConstructorInitializerLhs(SgConstructorInitializer *ctorInitializer);

#endif /* _USEOA_COMMON_H */
