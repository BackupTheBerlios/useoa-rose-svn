#ifdef HAVE_CONFIG_H
//#include <config.h>
#endif

#include <rose.h>
#include <iostream>

#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include "debug.h"
#include <OpenAnalysis/Alias/ManagerAliasMapBasic.hpp>
#include <OpenAnalysis/Alias/ManagerFIAliasEquivSets.hpp>
#include <OpenAnalysis/Alias/ManagerFIAliasAliasMap.hpp>
#include <OpenAnalysis/CallGraph/ManagerCallGraph.hpp>
#include <OpenAnalysis/CFG/ManagerCFG.hpp>
#include <OpenAnalysis/CFG/EachCFGStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/ICFG/ManagerICFG.hpp>
#include <OpenAnalysis/Activity/ManagerICFGDep.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/ReachDefs/ManagerReachDefsStandard.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilderDOT.hpp>
#include <OpenAnalysis/Utils/Util.hpp>
#include <OpenAnalysis/ReachConsts/ManagerReachConstsStandard.hpp>
#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
#include <OpenAnalysis/XAIF/UDDUChainsXAIF.hpp>
#include <OpenAnalysis/XAIF/ManagerUDDUChainsXAIF.hpp>


#include <OpenAnalysis/XAIF/AliasMapXAIF.hpp>
#include<OpenAnalysis/XAIF/ManagerAliasMapXAIF.hpp>

#include "CallGraph.h"

// #define DEBUG

// Taken, slightly modified, from OATest.C
OA::OA_ptr<OA::Alias::EquivSets>
DoFIAliasEquivSets(OA::OA_ptr<SageIRInterface> irInterface, SgProject * p)
{
  int returnvalue=FALSE;
  
  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasEquivSets> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasEquivSets(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  //  bool excludeInputFiles = true;
  // Don't pull in any procedures defined in input files.  For testing
  // purposes only:  avoids unexpected/spurious results due to 
  // stdlib.h, etc.
  //  procIter = new SageIRProcIterator(p, irInterface, excludeInputFiles);
  procIter = new SageIRProcIterator(p, *irInterface);
  OA::OA_ptr<OA::Alias::EquivSets> alias = 
    fialiasman->performAnalysis(procIter);
  return alias;
}

// Return the function in which node occurs.
// Taken, slightly modified, from getEnclosingMethod in SageOA.C.
static 
SgFunctionDefinition *getEnclosingFunction(SgNode *node)
{
  if ( node == NULL ) return NULL;

  if ( isSgGlobal(node) ) return NULL;

  SgFunctionDefinition *functionDefinition =
    isSgFunctionDefinition(node);
  
  if ( functionDefinition != NULL )
    return functionDefinition;

  SgFunctionDeclaration *functionDeclaration =
    isSgFunctionDeclaration(node);

  if ( functionDeclaration != NULL )
    return getEnclosingFunction(functionDeclaration->get_definition());
  
  return getEnclosingFunction(node->get_parent());
}

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
  //  std::cout << name1 << " " << name2 << std::endl;
  if ( name1 != name2 )
    return false;

  // Compare the return types of the two methods.
  SgType *method1Type = methodDecl1->get_orig_return_type();
  SgType *method2Type = methodDecl2->get_orig_return_type();

  std::string ret1Type, ret2Type;
  GetTypeInfo(method1Type, 0, &ret1Type);
  GetTypeInfo(method2Type, 0, &ret2Type);
  if ( ret1Type != ret2Type )
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

/** \brief  Returns true if method is a pure virtual method.
 *  \param  functionDeclaration  A method declaration.
 *  \returns  Boolean indicating whether method is a pure virtual
 *            method.
 */
bool isPureVirtual(SgFunctionDeclaration *functionDeclaration)
{
  if ( functionDeclaration == NULL ) return false;

  if ( functionDeclaration->get_functionModifier().isPureVirtual() ) 
    return true;

  SgDeclarationStatement *firstNondefiningDeclaration =
    functionDeclaration->get_firstNondefiningDeclaration();

  if ( firstNondefiningDeclaration == NULL )
    return false;

  SgFunctionDeclaration *firstNondefiningFuncDeclaration =
    isSgFunctionDeclaration(firstNondefiningDeclaration);
  ROSE_ASSERT(firstNondefiningFuncDeclaration != NULL);

  return firstNondefiningFuncDeclaration->get_functionModifier().isPureVirtual();
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

    SgDeclarationStatement *definingDecl =
      classDeclaration->get_definingDeclaration();
    if ( definingDecl == NULL )
      continue;
    
    SgClassDeclaration *definingClassDeclaration =
      isSgClassDeclaration(definingDecl);
    ROSE_ASSERT(classDeclaration != NULL);

    SgClassDefinition *parentClassDefinition =
      definingClassDeclaration->get_definition();

    if ( parentClassDefinition == NULL )
      continue;

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

  //  std::cout << "looks virtual" << std::endl;

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

void usage(char **argv)
{
  std::cerr << "usage: " << argv[0] << " -out:outputFile < compiler command line > " << std::endl;
  exit(-1);
}

int main(int argc, char **argv)
{
  int modifiedArgc; 
  char **modifiedArgv = NULL; 

  std::list<std::string> argList =  
    CommandlineProcessing::generateArgListFromArgcArgv(argc, argv); 

  const std::string outputFilePrefix = std::string("-out:"); 
  std::list<std::string> tmp; 
  // Get the path to the code templates. 
  tmp = CommandlineProcessing::generateOptionList(argList, 
                                                  outputFilePrefix); 
  if (tmp.size() != 1) { 
    usage(argv);
  }
  std::string outputFile = *(tmp.begin());

  std::ofstream ostr(outputFile.c_str(), std::ios::out | std::ios::app); 
  ROSE_ASSERT(ostr.is_open()); 

  CommandlineProcessing::generateArgcArgvFromList(argList,  
                                                  modifiedArgc, 
                                                  modifiedArgv); 
   
  SgProject *project = frontend(modifiedArgc, modifiedArgv);

  // Perform the alias analysis.
  OA::OA_ptr<SageIRInterface> irInterface;
  std::vector<SgNode*> nodeArray;
  bool p_h=FALSE; //for debugging only switch between persistent and "pointer" handles (pointers are faster, persistent are easier to debug

  irInterface = new SageIRInterface(project, &nodeArray, p_h);

  OA::OA_ptr<OA::Alias::EquivSets> alias = DoFIAliasEquivSets(irInterface,
                                                              project);

#if 0
  AstMerge am;
  am.addAST(project);
  
  project = am.getAST();
  SgNode *root = project;
#else
  SgNode *root = project;
#endif
#if 1

#if 0
  // Loop over every file.   BW 4/13/06
  int filenum = project->numberOfFiles();
  for (int i = 0; i < filenum; ++i) 
    {

        SgFile &sageFile = project->get_file(i);
        SgGlobal *root = sageFile.get_root();
#endif

  std::list<SgNode *> allCls;

  // Keep track of the SgClassDefinitions we have already processed,
  // to avoid redundant processing.
  std::set<SgClassDefinition *> processedClsDefns;

  // Unfortunately, SgClassDefinitions are not always visited by
  // query and traversal mechanisms.  Consider this example:
  //  class Base {
  //  public:
  //    virtual void foo() { }
  //    virtual void bar() { }
  //    virtual void baz() { }
  //  } *a, *b, *p, *q;
  // The SgClassDefinition for Base only shows up in the 
  // SgVariableDeclarations of *a, *b, etc.  Therefore, we need
  // to query the SgVariableDeclarations as well.

  VariantVector v(V_SgClassDefinition); 
  v = v + V_SgVariableDeclaration; 
  allCls = NodeQuery::querySubTree ( root, v );

  // build the class hierarchy
  // start by iterating through all the classes
  for ( std::list<SgNode *>::iterator it = allCls.begin(); it != allCls.end(); it++ )
    {

      SgClassDefinition *clsDescDef = NULL;

      SgNode *n = *it;
      ROSE_ASSERT(n != NULL);

      if ( isSgClassDefinition(n) ) {
	// We received a SgClassDefinition directly from the query.
	clsDescDef = isSgClassDefinition( n );
	if ( clsDescDef ) {

	  SgClassDeclaration *classDecl =
	    clsDescDef->get_declaration();
	  ROSE_ASSERT(classDecl != NULL);

	  SgClassDeclaration *definingClassDecl =
	    isSgClassDeclaration(classDecl->get_definingDeclaration());
	  ROSE_ASSERT(definingClassDecl != NULL);
	  
	  clsDescDef = definingClassDecl->get_definition();
	  ROSE_ASSERT(clsDescDef != NULL);

#ifdef DEBUG
	  std::cout << "Found " << clsDescDef << " parent: " << clsDescDef->get_parent()->sage_class_name() << clsDescDef->unparseToCompleteString() << std::endl;
#endif

	}
      } else if ( isSgVariableDeclaration(n) && 0 ) {
	// This may be the case described above.  See if there
	// is a class definition here.
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
	  
	  SgType *initType = initName->get_type();
	  SgClassType *classType =
	    isSgClassType(initType);
	  
	  if ( !classType ) {
	    SgPointerType *ptrType = isSgPointerType(initType);
	    if (ptrType) {
	      classType = isSgClassType(ptrType->get_base_type());
	    } else {
	      SgReferenceType *refType = isSgReferenceType(initType);
	      if (refType)
		classType = isSgClassType(refType->get_base_type());
	    }
	  }
	  
	  if ( classType ) {
	    
	    SgDeclarationStatement *declStmt = classType->get_declaration();
	    ROSE_ASSERT(declStmt != NULL);
	    
	    SgClassDeclaration *classDeclaration = isSgClassDeclaration(declStmt);
	    ROSE_ASSERT(classDeclaration != NULL);
	    
	    SgClassDeclaration *definingClassDeclaration = 
	      isSgClassDeclaration(classDeclaration->get_definingDeclaration());
	    if ( definingClassDeclaration != NULL ) {
	      clsDescDef = definingClassDeclaration->get_definition();
	      ROSE_ASSERT(clsDescDef != NULL);
	    }
	  }
	}
      } 
      if ( clsDescDef == NULL ) continue;
      // Notice that we may visit this class definition more than once.
      // This will not lead to correctness problems because addNode
      // and addEdge will not introduce redundant nodes or edges.
      // However, for efficiency (and to avoid leaking memory),
      // check to ensure we have not already handled this definition.

#ifdef DEBUG
      std::cout << "Examing class " << clsDescDef->get_qualified_name().str() << std::endl;
#endif

      if ( processedClsDefns.find(clsDescDef) != processedClsDefns.end() )
	continue;
      processedClsDefns.insert(clsDescDef);
      SgBaseClassPtrList &baseClses = clsDescDef->get_inheritances();
	  
      // for each iterate through their parents and add parent - child relationship to the graph
      for ( SgBaseClassPtrList::iterator it = baseClses.begin(); it != baseClses.end(); it++ )
	{
          //AS (032806) Added fix to get the defining class declaration
	  SgClassDeclaration *baseCls = isSgClassDeclaration(( *it )->get_base_class()->get_definingDeclaration());
          ROSE_ASSERT(baseCls!=NULL);
	  SgClassDefinition *baseClsDef = baseCls->get_definition();
	  ROSE_ASSERT ( baseClsDef != NULL);

#ifdef DEBUG
	  //	  Sg_File_Info *fileInfo = root->get_file_info();
	  //	  ROSE_ASSERT(fileInfo);
	  //	  std::cout << "Got " << baseClsDef << " " << baseClsDef->get_qualified_name().str() << " by examining class " << clsDescDef << " " << clsDescDef->get_qualified_name().str() << " in file " << fileInfo->get_filename() << std::endl;
	  std::cout << "Got " << baseClsDef << " " << baseClsDef->get_qualified_name().str() << " by examining class " << clsDescDef << " " << clsDescDef->get_qualified_name().str() << std::endl;
#endif
	  
	}
    }

#if 0
    }
#endif
#endif

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

#ifdef DEBUG
    std::cout << "Var defn: " << varDefn->unparseToCompleteString() << std::endl;
#endif

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
#ifdef DEBUG
	if ( classDefinition != NULL ) {
	  std::cout << "From var decl got: " << classDefinition->unparseToCompleteString() << std::endl;
	}
#endif

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

#ifdef DEBUG
    std::cout << "Calling getSubclasses on " << classDefinition->unparseToCompleteString() << std::endl;
#endif

    SgClassDefinitionPtrList subclasses = 
      classHierarchy.getSubclasses(classDefinition);

    // Iterate over all subclasses.
    for (SgClassDefinitionPtrList::iterator subclassIt = subclasses.begin();
	 subclassIt != subclasses.end(); ++subclassIt) {
      
      SgClassDefinition *subclass = *subclassIt;
      ROSE_ASSERT(subclass != NULL);
      
#ifdef DEBUG
      std::cout << "subclass" << std::endl;
#endif

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

#ifdef DEBUG
    std::cout << "Calling getSubclasses on " << classDefinition->unparseToCompleteString() << std::endl;
#endif

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
  unsigned int numAliasAnalysisMonomorphicCallSites = 0;
  unsigned int numAliasAnalysisPossibleResolutions = 0;

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

#ifdef DEBUG    
    std::cout << "method?: " << functionCallExp->unparseToCompleteString() << std::endl;
#endif

    if ( !isMethodCall(functionCallExp, isDotExp, isLhsRefOrPtr) )
      continue;
    
#ifdef DEBUG    
    std::cout << "method: " << functionCallExp->unparseToCompleteString() << std::endl;
#endif

#if 1
    if ( isDotExp && !isLhsRefOrPtr ) {
      // If this is a dot expression (i.e., a.foo()), we can
      // statically determine its type-- unless the left-hand
      // side is a reference type.
#ifdef DEBUG    
      std::cout << "dot: " << functionCallExp->unparseToCompleteString() << std::endl;
#endif
      continue;
    }
    numCallSites++;

#else
    numCallSites++;

    if ( isDotExp && !isLhsRefOrPtr ) {
      // If this is a dot expression (i.e., a.foo()), we can
      // statically determine its type-- unless the left-hand
      // side is a reference type.
      numMonomorphicCallSites++;
      numPossibleResolutions++;
      numAliasAnalysisMonomorphicCallSites++;
      numAliasAnalysisPossibleResolutions++;
#ifdef DEBUG    
      std::cout << "dot: " << functionCallExp->unparseToCompleteString() << std::endl;
#endif
      continue;
    }
#endif

#ifdef DEBUG    
    std::cout << "methodPtr: " << functionCallExp->unparseToCompleteString() << std::endl;
#endif

    // Retrieve the static function declaration.
    SgFunctionDeclaration *functionDeclaration = 
      getFunctionDeclaration(functionCallExp);

    // Ensure it is actually a method declaration.
    SgMemberFunctionDeclaration *memberFunctionDeclaration =
      isSgMemberFunctionDeclaration(functionDeclaration);
    ROSE_ASSERT(memberFunctionDeclaration != NULL);

    unsigned int numResolutionsForMethod = 0;

#if 0
    // I think this may double count.  Instead, look within the class
    // itself (as opposed to only its parent classes).  I don't think
    // we were doing this.

    // Certainly can be resolved to the static method (unless it
    // is pure virtual).
    if ( !isPureVirtual(memberFunctionDeclaration) ) {
      numResolutionsForMethod++;
    }
#endif

    if ( ( isVirtual(functionDeclaration) ) ||
	 ( isDeclaredVirtualWithinAncestor(functionDeclaration) ) ) {
      
#ifdef DEBUG    
      std::cout << "tracking: " << functionDeclaration->unparseToString() << std::endl;
#endif

      SgClassDefinition *classDefn = 
	isSgClassDefinition(memberFunctionDeclaration->get_scope());
      ROSE_ASSERT(classDefn != NULL);
      
      SgClassDeclaration *classDecl =
	classDefn->get_declaration();
      ROSE_ASSERT(classDecl != NULL);

      SgClassDeclaration *definingClassDecl =
	isSgClassDeclaration(classDecl->get_definingDeclaration());
      ROSE_ASSERT(definingClassDecl != NULL);

      SgClassDefinition *classDefinition =
	definingClassDecl->get_definition();
      ROSE_ASSERT(classDefinition != NULL);
      

      {
      SgClassDefinitionPtrList subclasses = 
	classHierarchy.getAncestorClasses(classDefinition);

#ifdef DEBUG
      if ( classDefinition != NULL )
	std::cout << "Looking up ancestors of " << classDefinition << " " << classDefinition->get_qualified_name().str() << std::endl;
#endif

      // Iterate over all subclasses.
      for (SgClassDefinitionPtrList::iterator subclassIt = subclasses.begin();
	   subclassIt != subclasses.end(); ++subclassIt) {

#ifdef DEBUG    
	std::cout << "subclass of " << classDefinition->get_qualified_name().str() << " got: " << std::endl;
#endif

      }
      }

      // First examine the class itself.
      // Iterate over all of the methods defined in this class.
      SgDeclarationStatementPtrList &decls =
	classDefinition->get_members();
      for (SgDeclarationStatementPtrList::iterator declIter = decls.begin();
	   declIter != decls.end(); ++declIter) {
	
	SgDeclarationStatement *declStmt = *declIter;
	ROSE_ASSERT(declStmt != NULL);
	
	SgMemberFunctionDeclaration *method =
	  isSgMemberFunctionDeclaration(declStmt);
	if ( method == NULL ) {
	  continue;
	}
	
#ifdef DEBUG    
	std::cout << "checking overrides" << std::endl;
#endif
	// Determine whether subclass of the class defining this
	// method overrides the method.
#if 1
	if ( matchingMemberFunctions(method,
				     memberFunctionDeclaration) ) {
#ifdef DEBUG    
	  std::cout << "overries" << std::endl;
#endif
	  // Do not consider a pure virtual method to be an 
	  // overriding method (since it can not be invoked).
	  if ( !isPureVirtual(method) ) {
	    numResolutionsForMethod++;
	  }
	}
#else
	if ( methodOverridesVirtualMethod(method, 
					  memberFunctionDeclaration) ) {
#ifdef DEBUG    
	  std::cout << "overries" << std::endl;
#endif
	  numResolutionsForMethod++;
	}
#endif
      }

      // Now look within the sub classes.
      SgClassDefinitionPtrList subclasses = 
	classHierarchy.getSubclasses(classDefinition);

#ifdef DEBUG
      if ( classDefinition != NULL )
	std::cout << "Looking up subclasses of " << classDefinition << " " << classDefinition->get_qualified_name().str() << std::endl;
#endif

      // Iterate over all subclasses.
      for (SgClassDefinitionPtrList::iterator subclassIt = subclasses.begin();
	   subclassIt != subclasses.end(); ++subclassIt) {

	SgClassDefinition *subclass = *subclassIt;
	ROSE_ASSERT(subclass != NULL);

#ifdef DEBUG    
	std::cout << "subclass of " << classDefinition->get_qualified_name().str() << " got: " << subclass->get_qualified_name().str() << std::endl;
#endif


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

#ifdef DEBUG    
	  std::cout << "checking overrides" << std::endl;
#endif
	  // Determine whether subclass of the class defining this
	  // method overrides the method.
#if 1
	  if ( matchingMemberFunctions(method,
				       memberFunctionDeclaration) ) {
#ifdef DEBUG    
	    std::cout << "overries" << std::endl;
#endif
	    // Do not consider a pure virtual method to be an 
	    // overriding method (since it can not be invoked).
	    if ( !isPureVirtual(method) ) {
	      numResolutionsForMethod++;
	    }
	  }
#else
	  if ( methodOverridesVirtualMethod(method, 
					    memberFunctionDeclaration) ) {
#ifdef DEBUG    
	    std::cout << "overries" << std::endl;
#endif
	    numResolutionsForMethod++;
	  }
#endif
	}

      }

      if ( numResolutionsForMethod == 1 )
	numMonomorphicCallSites++;
      numPossibleResolutions += numResolutionsForMethod;

      // Now perform the alias-based virtual method resolution.
      // Logically, this looks like:
      // for all procs proc in SageIRProcIterator
      //   for all statements stmt in getStmtIterator(proc)
      //     for all call sites cs in getCallsites(stmt)
      //       for all call MREs callmre in getCAllMemRefExp(cs)
      //         OA_ptr<LocIterator> locIter =
      //           alias->getMayLocs(callmre, proc)
      //         cout << "callsite has " << locIter.size << "resolutions"

      // We already have a Sage handle to a callsite (and no handle
      // to the enclosing procedure, proc).
      // First, find the enclosing function for the call site.
      // Then convert the Sage handles for callsites and the
      // enclosing function definition to OA CallHandles and ProcHandles.
      SgFunctionDefinition *enclosingFuncDefn =
        getEnclosingFunction(functionCallExp);
      ROSE_ASSERT(enclosingFuncDefn != NULL);

      // Get an OA CallHandle.
      OA::CallHandle callHandle = irInterface->getProcExprHandle(functionCallExp);

      // Get an OA ProcHandle.
      OA::ProcHandle callee = irInterface->getProcHandle(enclosingFuncDefn);

      // Get all of the Call MemRefExprs at the callsite.  I think 
      // we are expecting only one?  Whoops ... interface only
      // allows one.
      OA::OA_ptr<OA::MemRefExpr> callMRE = irInterface->getCallMemRefExpr(callHandle);
      ROSE_ASSERT(!callMRE.ptrEqual(0));

      // How many locations has the alias analysis assigned to this
      // callMRE?
      OA::OA_ptr<OA::LocIterator> locIter =
        alias->getMayLocs(*callMRE, callee);
      unsigned int numAliasAnalysisResolutionsForMethod = 0;
      for (locIter->reset(); locIter->isValid(); (*locIter)++) {
        // Don't count invisible locations:
	OA::OA_ptr<OA::Location> loc = locIter->current();
        if ( !loc->isaInvisible() ) {
          ++numAliasAnalysisResolutionsForMethod;
	  std::cout << "Visible Location: " << std::endl;
        } else {
	  std::cout << "Invisible Location: " << std::endl;
        }
        loc->output(*irInterface);
      }

      if ( numAliasAnalysisResolutionsForMethod == 1 )
	numAliasAnalysisMonomorphicCallSites++;
      numAliasAnalysisPossibleResolutions += numAliasAnalysisResolutionsForMethod;

      if ( ( ( numResolutionsForMethod ) > 1 ) || 
           ( ( numAliasAnalysisResolutionsForMethod ) > 1 ) ) {
	ostr << "CHA:  Method invocation has " << numResolutionsForMethod << " possible resolutions " << std::endl;
	ostr << "Alias analysis:  Method invocation has " << numAliasAnalysisResolutionsForMethod << " possible resolutions " << std::endl;
	ostr << functionCallExp->unparseToCompleteString() << std::endl;
      }
    }

  }
#endif
#endif
  ostr << "Summary CHA monomorphic call sites: " << numMonomorphicCallSites << std::endl;
  ostr << "Summary Alias analysis monomorphic call sites: " << numAliasAnalysisMonomorphicCallSites << std::endl;
  ostr << "Summary CHA total (virtual) resolutions: " << numPossibleResolutions << std::endl;
  ostr << "Summary Alias analysis total (virtual) resolutions: " << numAliasAnalysisPossibleResolutions << std::endl;
  //  return 0;
  return backend(project);
}
