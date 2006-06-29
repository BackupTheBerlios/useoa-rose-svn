#ifdef HAVE_CONFIG_H
//#include <config.h>
#endif

#include <rose.h>
#include <iostream>
#include "CallGraph.h"

/** \brief Strip the "const" keyword from a string.
 *  \param name  A string (intending to represent a formal parameter
 *               type).
 *  \returns  A string holding name stripped of the "const" prefix.
 *
 *  This was copied from 
 *  .../ROSE/src/midend/astUtil/astInterface/AstInterface.C
 *
 *  To do:  move this to a generally-accessible utilities file
 *          or make it accessible from AstInterface (by adding
 *          its declaration to AstInterface.h).
 */
static std::string StripParameterType( const std::string& name)
{
  char *const_start = strstr( name.c_str(), "const");
  std::string r = (const_start == 0)? name : std::string(const_start + 5);
  int end = r.size()-1;
  if (r[end] == '&') {
       r[end] = ' ';
  }
  std::string result = "";
  for (unsigned int i = 0; i < r.size(); ++i) {
    if (r[i] != ' ')
      result.push_back(r[i]);
  }
  return result; 
} 

/** \brief Return a type's name and size.
 *  \param t  A Sage type.
 *  \param tname  On output, holds the string name of the type.
 *  \param stripname  On output, holds the string name of the type,
 *                    stripped of any "const" prefix.
 *  \param size  On output, holds the size?  I don't know what
 *               this size corresponds to.  Certainly not the size
 *               of the type (which needn't be a word) or string.
 *
 *  This was copied from 
 *  .../ROSE/src/midend/astUtil/astInterface/AstInterface.C
 *
 *  To do:  move this to a generally-accessible utilities file
 *          or make it accessible from AstInterface (by adding
 *          its declaration to AstInterface.h).
 */
static void GetTypeInfo(SgType *t, std::string *tname, 
			std::string* stripname, int* size = 0)
{
  std::string r1 = get_type_name(t);
  std::string result = "";
  for (unsigned int i = 0; i < r1.size(); ++i) {
    if (r1[i] != ' ')
      result.push_back(r1[i]);
  }
  if (tname != 0)
    *tname = result;
  if (stripname != 0)
    *stripname = StripParameterType(result);
  if (size != 0)
    *size = 4;
}

bool matchingMemberFunctions(SgMemberFunctionDeclaration *methodDecl1, 
			     SgMemberFunctionDeclaration *methodDecl2)
{
  // Compare the names of the two methods.
  SgName name1 = methodDecl1->get_name();
  SgName name2 = methodDecl2->get_name();
  if ( name1 != name2 )
    return false;

  // Compare the return types of the two methods.
  SgType *method1Type = methodDecl1->get_orig_return_type();
  SgType *method2Type = methodDecl2->get_orig_return_type();

  std::string ret1Type, ret2Type;
  GetTypeInfo(method1Type, 0, &ret1Type);
  GetTypeInfo(method2Type, 0, &ret2Type);
  if ( method1Type != method2Type )
    return false;


  // Compare the number and types of the formal arguments
  SgInitializedNamePtrList &method1Params = methodDecl1->get_args();
  SgInitializedNamePtrList &method2Params = methodDecl2->get_args();
  if ( method1Params.size() != method2Params.size() )
    return false;

  SgInitializedNamePtrList::iterator p1 = method1Params.begin();
  SgInitializedNamePtrList::iterator p2 = method2Params.begin();
  for ( ; p1 != method1Params.end(); ++p1, ++p2) {
    SgType* t1 = (*p1)->get_type();
    SgType* t2 = (*p2)->get_type();
    
    std::string param1Type, param2Type;
    GetTypeInfo(t1, 0, &param1Type);
    GetTypeInfo(t2, 0, &param2Type);
    if (param1Type != param2Type) {
      return false;
    }
  }

  return true;
}

bool isMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp, bool &lhsIsRefOrPtr)
{
  ROSE_ASSERT(functionCall != NULL);

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  bool isMethod = false;
  isDotExp = false;
  lhsIsRefOrPtr = false;

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

      lhsIsRefOrPtr = isSgReferenceType(lhs->get_type());

      break;
    }
  case V_SgArrowExp:
    {
      isMethod = true;
      lhsIsRefOrPtr = true;
      break;
    }
  case V_SgMemberFunctionRefExp:
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

  std::cout << "looks virtual" << std::endl;

#if 0
  // Hmmm ... couldn't we just compare mangled names?
  // No, we can't because this includes the scope info, which will
  // differ between classes.
  std::cout << "methodDecl: " << methodDecl->get_mangled_name().str() << std::endl;
  std::cout << "virtualMethodDecl: " << virtualMethodDecl->get_mangled_name().str() << std::endl;
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

#if 0
  std::list<SgNode *> nodes2 = NodeQuery::querySubTree(project,
						      V_SgVariableDefinition);

  for (std::list<SgNode *>::iterator it = nodes2.begin();
       it != nodes2.end(); ++it ) {

    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);

    SgVariableDefinition *varDefn =
      isSgVariableDefinition(n);
    ROSE_ASSERT(varDefn != NULL);

    std::cout << "Var defn: " << varDefn->unparseToCompleteString() << std::endl;

  }

  std::list<SgNode *> nodes1 = NodeQuery::querySubTree(project,
						      V_SgVariableDeclaration);

  for (std::list<SgNode *>::iterator it = nodes1.begin();
       it != nodes1.end(); ++it ) {

    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);

    SgVariableDeclaration *varDecl =
      isSgVariableDeclaration(n);
    ROSE_ASSERT(varDecl != NULL);

    SgInitializedNamePtrList &variables =
      varDecl->get_variables();
    SgInitializedNamePtrList::iterator varIter;
    for (varIter = variables.begin(); 
	 varIter != variables.end(); ++varIter) {
      
      SgNode *var = *varIter;
      ROSE_ASSERT(var != NULL);
      
      SgInitializedName *initName =
	isSgInitializedName(var);
      ROSE_ASSERT(initName != NULL);
      
      if ( isSgClassType(initName->get_type()) ) {

	SgClassType *classType = isSgClassType(initName->get_type());
	ROSE_ASSERT(classType != NULL);

	SgDeclarationStatement *declStmt = classType->get_declaration();
	ROSE_ASSERT(declStmt != NULL);
	
	SgClassDeclaration *classDeclaration = isSgClassDeclaration(declStmt);
	ROSE_ASSERT(classDeclaration != NULL);
      
	//	std::cout << "From var decl got: " << classDeclaration->unparseToCompleteString() << std::endl;

	SgClassDefinition *classDefinition =
	  classDeclaration->get_definition();
	if ( classDefinition != NULL ) {
	  std::cout << "From var decl got: " << classDefinition->unparseToCompleteString() << std::endl;
	}

      }

    }
    

  }

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
#endif
#if 1
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
    bool isLhsRefOrPtr = false;
    if ( !isMethodCall(functionCallExp, isDotExp, isLhsRefOrPtr) )
      continue;
    
    numCallSites++;
    // Certainly can be resolved to the static method.
    numPossibleResolutions++;

    if ( isDotExp && !isLhsRefOrPtr ) {
      // If this is a dot expression (i.e., a.foo()), we can
      // statically determine its type-- unless the left-hand
      // side is a reference type.
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

	//	std::cout << "subclass" << std::endl;

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

	  //	  std::cout << "checking overrides" << std::endl;
	  // Determine whether subclass of the class defining this
	  // method overrides the method.
#if 1
	  if ( matchingMemberFunctions(method,
				       memberFunctionDeclaration) ) {
	    //	    std::cout << "overries" << std::endl;
	    numOverridesForMethod++;
	  }
#else
	  if ( methodOverridesVirtualMethod(method, 
					    memberFunctionDeclaration) ) {
	    //	    std::cout << "overries" << std::endl;
	    numOverridesForMethod++;
	  }
#endif
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
