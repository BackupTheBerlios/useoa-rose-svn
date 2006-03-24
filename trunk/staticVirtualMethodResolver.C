#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rose.h>
#include <iostream>
#include "CallGraph.h"

bool isMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp)
{
  ROSE_ASSERT(functionCall != NULL);

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  bool isMethod = false;
  isDotExp = false;

  switch(expression->variantT()) {
  case V_SgDotExp:
    {
      isMethod = true;

      SgDotExp *dotExp = isSgDotExp(expression);
      ROSE_ASSERT(dotExp != NULL);
	  
      SgExpression *lhs = dotExp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);
	  
      SgPointerDerefExp *pointerDerefExp =
	isSgPointerDerefExp(lhs);
	  
      if ( pointerDerefExp != NULL ) {
	;
      } else {
	isDotExp = true;
      }

      break;
    }
  case V_SgMemberFunctionRefExp:
  case V_SgArrowExp:
    {
      isMethod = true;
      break;
    }
  case V_SgFunctionRefExp:
  case V_SgPointerDerefExp:
    {
      isMethod = false;
      break;
    }
  default:
    {
      std::cerr << "Was not expecting an " << expression->sage_class_name() << std::endl;
      std::cerr << "in a function call." << std::endl;
      ROSE_ABORT();
    }
  }

  return isMethod;
}

SgFunctionDeclaration * 
getFunctionDeclaration(SgFunctionCallExp *functionCall) 
{ 
  SgFunctionDeclaration *funcDec = NULL; 

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  switch(expression->variantT()) {
  case V_SgMemberFunctionRefExp:
    {
      SgMemberFunctionRefExp *memberFunctionRefExp =
	isSgMemberFunctionRefExp(expression);
      ROSE_ASSERT(memberFunctionRefExp != NULL);

      funcDec = memberFunctionRefExp->get_symbol_i()->get_declaration(); 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgDotExp:
    {
      SgDotExp *dotExp = isSgDotExp(expression);
      ROSE_ASSERT(dotExp != NULL);

      if(dotExp->get_traversalSuccessorContainer().size()>=2) { 

	SgMemberFunctionRefExp *memberFunctionRefExp = 
	  isSgMemberFunctionRefExp(dotExp->get_traversalSuccessorContainer()[1]); 
	funcDec = memberFunctionRefExp->get_symbol_i()->get_declaration(); 
      } 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgArrowExp:
    {
      SgArrowExp *arrowExp = isSgArrowExp(expression);
      ROSE_ASSERT(arrowExp != NULL);

      if(arrowExp->get_traversalSuccessorContainer().size()>=2) { 

	SgMemberFunctionRefExp *memberFunctionRefExp = 
	  isSgMemberFunctionRefExp(arrowExp->get_traversalSuccessorContainer()[1]); 
	funcDec = memberFunctionRefExp->get_symbol_i()->get_declaration(); 
      } 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgFunctionRefExp:
    {
      SgFunctionRefExp *functionRefExp = 
	isSgFunctionRefExp(expression);
      ROSE_ASSERT(functionRefExp != NULL);

      // found a standard function reference  
      funcDec = functionRefExp->get_symbol_i()->get_declaration(); 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgPointerDerefExp:
    {
      ROSE_ABORT();
      break;
    }
  default:
    {
      ROSE_ABORT();
    }
  }

  return funcDec; 
} 

bool isVirtual(SgFunctionDeclaration *functionDeclaration)
{
  if ( functionDeclaration == NULL ) return false;

  if ( functionDeclaration->get_functionModifier().isVirtual() ) 
    return true;

  SgDeclarationStatement *firstNondefiningDeclaration =
    functionDeclaration->get_firstNondefiningDeclaration();

  if ( firstNondefiningDeclaration == NULL )
    return false;

  SgFunctionDeclaration *firstNondefiningFuncDeclaration =
    isSgFunctionDeclaration(firstNondefiningDeclaration);
  ROSE_ASSERT(firstNondefiningFuncDeclaration != NULL);

  return firstNondefiningFuncDeclaration->get_functionModifier().isVirtual();
}

bool isDeclaredVirtualWithinClassAncestry(SgFunctionDeclaration *functionDeclaration, SgClassDefinition *classDefinition)
{
  SgType *functionType =
    functionDeclaration->get_type();
  ROSE_ASSERT(functionType != NULL);

  // Look in each of the class' parent classes.
  SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances(); 
  for (SgBaseClassPtrList::iterator i = baseClassList.begin(); 
       i != baseClassList.end(); ++i) {
 
    SgBaseClass *baseClass = *i;
    ROSE_ASSERT(baseClass != NULL);

    SgClassDeclaration *classDeclaration = baseClass->get_base_class(); 
    ROSE_ASSERT(classDeclaration != NULL);

    SgClassDefinition  *parentClassDefinition  = 
      classDeclaration->get_definition(); 

    // Visit all methods in the parent class.
    SgDeclarationStatementPtrList &members = 
      parentClassDefinition->get_members(); 

    bool isDeclaredVirtual = false;

    for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
	 it != members.end(); ++it) { 
    
      SgDeclarationStatement *declarationStatement = *it; 
      ROSE_ASSERT(declarationStatement != NULL);
      
      switch(declarationStatement->variantT()) {
      
      case V_SgMemberFunctionDeclaration:
	{
	  SgMemberFunctionDeclaration *memberFunctionDeclaration =  
	    isSgMemberFunctionDeclaration(declarationStatement); 

	  if ( isVirtual(memberFunctionDeclaration) ) {

	    SgType *parentMemberFunctionType =
	      memberFunctionDeclaration->get_type();
	    ROSE_ASSERT(parentMemberFunctionType != NULL);

	    if ( parentMemberFunctionType == functionType ) {
	      return true;
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

    if ( isDeclaredVirtualWithinClassAncestry(functionDeclaration, 
					      parentClassDefinition) ) {
      return true;
    }

  }

  return false;
}

// Returns true if the function declaration is declared virtual in
// some parent class, regardless of whether it is declared virtual
// in its own class.
bool isDeclaredVirtualWithinAncestor(SgFunctionDeclaration *functionDeclaration)
{
  SgMemberFunctionDeclaration *memberFunctionDeclaration =
    isSgMemberFunctionDeclaration(functionDeclaration);
  if ( memberFunctionDeclaration == NULL )
    return false;

  SgClassDefinition *classDefinition = 
    isSgClassDefinition(memberFunctionDeclaration->get_scope());
  ROSE_ASSERT(classDefinition != NULL);

  return isDeclaredVirtualWithinClassAncestry(functionDeclaration,
					      classDefinition);
}

/**
 * \brief Return true if methodDecl overrides virtualMethodDecl.
 * \param methodDecl  a method declaration.
 * \param virtualMethodDecl a method declaration.
 * \return Returns true if virtualMethodDecl is declared as a virtual
 *         method and methodDecl has the same type signature and name
 *         as virtualMethodDecl.  
 * 
 * NB:  It is assumed that the class defining virtualMethodDecl is a base
 *      class of the class defining methodDecl.
 */
bool
methodOverridesVirtualMethod(SgMemberFunctionDeclaration *methodDecl, 
			     SgMemberFunctionDeclaration *virtualMethodDecl)
{
  if ( !isVirtual(virtualMethodDecl) )
    return false;

#if 1
  // Hmmm ... couldn't we just compare mangled names?
  return ( methodDecl->get_mangled_name() == virtualMethodDecl->get_mangled_name() );

#else
  if ( methodDecl->get_name() != virtualMethodDecl->get_name() )
    return false;
  
  SgType *methodReturnType = methodDecl->get_orig_return_type();
  SgType *virtualMethodReturnType = virtualMethodDecl->get_orig_return_type();

  if ( methodReturnType != virtualMethodReturnType )
    return false;

  int numMethodParams = 0;
  int numVirtualMethodParams = 0;

  SgFunctionParameterList *methodParameterList = 
    methodDecl->get_parameterList(); 

  if (methodParameterList != NULL) {
    numMethodParams = methodParameterList->get_args().size();
  }

  SgFunctionParameterList *virtualMethodParameterList = 
    virtualMethodDecl->get_parameterList(); 

  if (virtualMethodParameterList != NULL) {
    numVirtualMethodParams = virtualMethodParameterList->get_args().size();
  }

  if ( numMethodParams != numVirtualMethodParams )
    return false;

  if ( numMethodParams == 0 )
    return true;

  const SgInitializedNamePtrList &methodFormalParams = 
    methodParameterList->get_args(); 
  const SgInitializedNamePtrList &virtualMethodFormalParams = 
    virtualMethodParameterList->get_args(); 
  SgInitializedNamePtrList::const_iterator methodIt;
  SgInitializedNamePtrList::const_iterator virtualMethodIt;
  for(methodIt = methodFormalParams.begin(), 
	virtualMethodIt = virtualMethodFormalParams.begin();
      methodIt != methodFormalParams.end(); ++methodIt, ++virtualMethodIt) { 
      
      SgInitializedName* methodFormalParam = *methodIt;  
      ROSE_ASSERT(methodFormalParam != NULL); 

      SgInitializedName* virtualMethodFormalParam = *virtualMethodIt;  
      ROSE_ASSERT(virtualMethodFormalParam != NULL); 
      
      if ( methodFormalParam->get_type() != 
	   virtualMethodFormalParam->get_type() )
	return false;

  }

  return true;
#endif
}

int main(int argc, char **argv)
{
  SgProject *project = frontend(argc, argv);
  
  // Instantiate a class hierarchy wrapper.
  ClassHierarchyWrapper classHierarchy( project );

  std::list<SgNode *> nodes = NodeQuery::querySubTree(project,
						      V_SgClassDeclaration);

  for (std::list<SgNode *>::iterator it = nodes.begin();
       it != nodes.end(); ++it ) {

    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);

    SgClassDeclaration *classDeclaration1 =
      isSgClassDeclaration(n);
    ROSE_ASSERT(classDeclaration1 != NULL);

    SgDeclarationStatement *definingDecl =
      classDeclaration1->get_definingDeclaration();
    if ( definingDecl == NULL )
      continue;
    
    SgClassDeclaration *classDeclaration =
      isSgClassDeclaration(definingDecl);
    ROSE_ASSERT(classDeclaration != NULL);


    SgClassDefinition *classDefinition =
      classDeclaration->get_definition();
    ROSE_ASSERT(classDefinition != NULL);

    std::cout << "Calling getSubclasses on " << classDefinition->unparseToCompleteString() << std::endl;

    SgClassDefinitionPtrList subclasses = 
      classHierarchy.getSubclasses(classDefinition);

    // Iterate over all subclasses.
    for (SgClassDefinitionPtrList::iterator subclassIt = subclasses.begin();
	 subclassIt != subclasses.end(); ++subclassIt) {
      
      SgClassDefinition *subclass = *subclassIt;
      ROSE_ASSERT(subclass != NULL);
      
      std::cout << "subclass" << std::endl;

    }

  }
#if 0
#if 0
  std::list<SgNode *> nodes = NodeQuery::querySubTree(project,
						      V_SgClassDefinition);

  for (std::list<SgNode *>::iterator it = nodes.begin();
       it != nodes.end(); ++it ) {

    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);

    SgClassDefinition *classDefinition =
      isSgClassDefinition(n);
    ROSE_ASSERT(classDefinition != NULL);

    std::cout << "Calling getSubclasses on " << classDefinition->unparseToCompleteString() << std::endl;

    SgClassDefinitionPtrList subclasses = 
      classHierarchy.getSubclasses(classDefinition);

    // Iterate over all subclasses.
    for (SgClassDefinitionPtrList::iterator subclassIt = subclasses.begin();
	 subclassIt != subclasses.end(); ++subclassIt) {
      
      SgClassDefinition *subclass = *subclassIt;
      ROSE_ASSERT(subclass != NULL);
      
      std::cout << "subclass" << std::endl;

    }

  }
#else
  // Collect all function/method invocations.
  std::list<SgNode *> nodes = NodeQuery::querySubTree(project,
						      V_SgFunctionCallExp);

  unsigned int numCallSites = 0;
  unsigned int numMonomorphicCallSites = 0;
  unsigned int numPossibleResolutions = 0;

  // Visit each call site.
  for (std::list<SgNode *>::iterator it = nodes.begin();
       it != nodes.end(); ++it ) {

    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);

    SgFunctionCallExp *functionCallExp =
      isSgFunctionCallExp(n);
    ROSE_ASSERT(functionCallExp != NULL);

    // We are only interested in examining method invocations.
    bool isDotExp = false;
    if ( !isMethodCall(functionCallExp, isDotExp) )
      continue;
    
    numCallSites++;
    // Certainly can be resolved to the static method.
    numPossibleResolutions++;

    if ( isDotExp ) {
      // If this is a dot expression (i.e., a.foo()), we can
      // statically determine its type.
      numMonomorphicCallSites++;
      continue;
    }

    // Retrieve the static function declaration.
    SgFunctionDeclaration *functionDeclaration = 
      getFunctionDeclaration(functionCallExp);

    // Ensure it is actually a method declaration.
    SgMemberFunctionDeclaration *memberFunctionDeclaration =
      isSgMemberFunctionDeclaration(functionDeclaration);
    ROSE_ASSERT(memberFunctionDeclaration != NULL);

    unsigned int numOverridesForMethod = 0;

    if ( ( isVirtual(functionDeclaration) ) ||
	 ( isDeclaredVirtualWithinAncestor(functionDeclaration) ) ) {
      
      SgClassDefinition *classDefinition = 
	isSgClassDefinition(memberFunctionDeclaration->get_scope());
      ROSE_ASSERT(classDefinition != NULL);
      
      SgClassDefinitionPtrList subclasses = 
	classHierarchy.getSubclasses(classDefinition);

      // Iterate over all subclasses.
      for (SgClassDefinitionPtrList::iterator subclassIt = subclasses.begin();
	   subclassIt != subclasses.end(); ++subclassIt) {

	SgClassDefinition *subclass = *subclassIt;
	ROSE_ASSERT(subclass != NULL);

	std::cout << "subclass" << std::endl;

	// Iterate over all of the methods defined in this subclass.
	SgDeclarationStatementPtrList &decls =
	  subclass->get_members();
	for (SgDeclarationStatementPtrList::iterator declIter = decls.begin();
	     declIter != decls.end(); ++declIter) {

	  SgDeclarationStatement *declStmt = *declIter;
	  ROSE_ASSERT(declStmt != NULL);

	  SgMemberFunctionDeclaration *method =
	    isSgMemberFunctionDeclaration(declStmt);
	  if ( method == NULL ) {
	    continue;
	  }

	  // Determine whether subclass of the class defining this
	  // method overrides the method.
	  if ( methodOverridesVirtualMethod(method, 
					    memberFunctionDeclaration) ) {
	    numOverridesForMethod++;
	  }

	}

      }

      if ( numOverridesForMethod == 0 )
	numMonomorphicCallSites++;
      numPossibleResolutions += numOverridesForMethod;

      std::cout << "Method invocation has " << numOverridesForMethod + 1 << " possible resolutions " << std::endl;
      std::cout << functionCallExp->unparseToCompleteString() << std::endl;

    }

  }
#endif
#endif
  return 0;
}
