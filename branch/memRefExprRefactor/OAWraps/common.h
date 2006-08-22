
#ifndef _USEOA_COMMON_H
#define _USEOA_COMMON_H

#include "rose.h"
#include <OpenAnalysis/IRInterface/IRHandles.hpp>

bool isMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp);

SgPointerDerefExp *isFunctionPointer(SgFunctionCallExp *functionCall);

SgFunctionDeclaration *getFunctionDeclaration(SgFunctionCallExp *functionCall);

SgTypePtrList &getFormalTypes(SgNode *node);

void verifyCallHandleNodeType(SgNode *node);

SgMemberFunctionDeclaration *
lookupDestructorInClass(SgClassDefinition *classDefinition);

void getActuals(SgNode *node, std::list<SgNode *> &actuals);

//! Return the method in which node occurs.
SgFunctionDefinition *getEnclosingFunction(SgNode *node);

#endif /* _USEOA_COMMON_H */
