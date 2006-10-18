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

#include <sys/time.h>
#include <time.h>

double diffTime(struct timeval start, struct timeval stop)
{
  struct timeval diff;
  diff.tv_sec = stop.tv_sec - start.tv_sec;
  diff.tv_usec = stop.tv_usec - start.tv_usec;
  if ( diff.tv_usec < 0 ) {
    diff.tv_usec += 1000000;
    diff.tv_sec -= 1;
  }

  double secs = diff.tv_sec + ( diff.tv_usec / 1000000.0 );
  return secs;
}


#include <OpenAnalysis/XAIF/AliasMapXAIF.hpp>
#include<OpenAnalysis/XAIF/ManagerAliasMapXAIF.hpp>

#include <defaultFunctionGenerator.h>
#include <shortCircuitingTransformation.h>
#include <destructorCallAnnotator.h>

#include "CallGraph.h"
#include "common.h"

#include <ext/hash_set>

#define DEBUG

using namespace UseOA;

// Taken, slightly modified, from OATest.C
/*
OA::OA_ptr<OA::Alias::EquivSets>
DoFIAliasEquivSets(OA::OA_ptr<SageIRInterface> irInterface, SgProject * p)
{
  int returnvalue=FALSE;
  
  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasEquivSets> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasEquivSets(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, *irInterface);
  OA::OA_ptr<OA::Alias::EquivSets> alias = 
    fialiasman->performAnalysis(procIter);
  return alias;
}
*/

OA::OA_ptr<OA::Alias::InterAliasMap>
DoFIAliasAliasMap(OA::OA_ptr<SageIRInterface> irInterface, SgProject * p)
{
  int returnvalue=FALSE;
  
  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, *irInterface);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);
  //  interAlias->output(*irInterface);

  return interAlias;
}

/** \brief Returns true if the function/method within node
 *         is enclosed is a constructor or destructor.
 */
bool isParentCtorOrDtor(SgNode *node)
{
    SgFunctionDefinition *funcDefn =
        getEnclosingFunction(node);
    SgMemberFunctionDeclaration *decl =
        isSgMemberFunctionDeclaration(funcDefn->get_declaration());

    if ( ( decl != NULL ) &&
         ( isConstructor(decl) || isDestructor(decl) ) ) {
        return true;
    }
    return false;
}
SgMemberFunctionRefExp *
isMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp, bool &lhsIsRefOrPtr)
{
  ROSE_ASSERT(functionCall != NULL);

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  bool isMethod = false;
  isDotExp = false;
  lhsIsRefOrPtr = false;

  SgMemberFunctionRefExp *functionRef = NULL;

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

      lhsIsRefOrPtr = ( isSgReferenceType(lhs->get_type()) || isSgPointerDerefExp(lhs) );

      functionRef = isSgMemberFunctionRefExp(isSgBinaryOp(expression)->get_rhs_operand());
      ROSE_ASSERT(functionRef != NULL);

      break;
    }
  case V_SgArrowExp:
    {
      isMethod = true;
      lhsIsRefOrPtr = true;
      functionRef = isSgMemberFunctionRefExp(isSgBinaryOp(expression)->get_rhs_operand());
      ROSE_ASSERT(functionRef != NULL);
      break;
    }
  case V_SgMemberFunctionRefExp:
    {
      isMethod = true;
      functionRef = isSgMemberFunctionRefExp(expression);
      ROSE_ASSERT(functionRef != NULL);
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

  return functionRef;
}

SgFile *getDefiningFile(SgNode *node)
{
    ROSE_ASSERT(node != NULL);
    if ( isSgFile(node ) ) {
        return isSgFile(node);
    }
    return getDefiningFile(node->get_parent());
}

bool definedWithinUserFile(SgNode *node, SgProject *project)
{
    Sg_File_Info *fileInfo = node->get_file_info();
    ROSE_ASSERT(fileInfo != NULL);

    string definingFileName = StringUtility::stripPathFromFileName(fileInfo->get_filename());

    bool definedInUserFile = false;
    list<string> files = project->getFileNames();
    for (list<string>::iterator it = files.begin();
         it != files.end(); ++it) {
        string s = *it;
        string stripped = StringUtility::stripPathFromFileName(s);
        if ( stripped == definingFileName ) {
            definedInUserFile = true;

            break;
        }
    }

    return definedInUserFile;
}

void countFunctions(int &definedFunctions, int &definedVirtualFunctions, 
                    int &invokedVirtualFunctions,
                    int &numStatements,
                    int &numMemRefs,
                    OA::OA_ptr<SageIRInterface> irInterface, SgProject * p)
{
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, *irInterface);
  definedFunctions = 0;
  definedVirtualFunctions = 0;
  invokedVirtualFunctions = 0;
  numStatements = 0;
  numMemRefs = 0;
  for (procIter->reset() ; procIter->isValid(); ++(*procIter)) {
      OA::ProcHandle currProc = procIter->current();

      {

          OA::OA_ptr<OA::IRStmtIterator> stmtIterPtr = irInterface->getStmtIterator(currProc);
          for ( ; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
              ++numStatements;
              OA::StmtHandle stmt = stmtIterPtr->current();
    
              OA::OA_ptr<OA::MemRefHandleIterator> mrIterPtr = 
                  irInterface->getAllMemRefs(stmt); 
              for (; mrIterPtr->isValid(); (*mrIterPtr)++ ) { 
                  OA::MemRefHandle memref = mrIterPtr->current(); 

                  // get the memory reference expressions for this handle 
                  OA::OA_ptr<OA::MemRefExprIterator> mreIterPtr = 
                      irInterface->getMemRefExprIterator(memref); 
       
                  // for each mem-ref-expr associated with this memref 
                  for (; mreIterPtr->isValid(); (*mreIterPtr)++) { 
                      numMemRefs++;
                  }
              }
          }
      }


      SgFunctionDefinition *funcDefn = isSgFunctionDefinition(irInterface->getNodePtr(currProc));
      ROSE_ASSERT(funcDefn != NULL);
      SgFunctionDeclaration *funcDecl = 
	isSgFunctionDeclaration(funcDefn->get_declaration());
      ROSE_ASSERT(funcDecl != NULL);

      if ( !definedWithinUserFile(funcDefn, p) ) {
          continue;
      }

      ++definedFunctions;

      if ( isVirtual(funcDecl) ) {
	std::cout << "virtual function defn: " << funcDecl->unparseToString() << std::endl;
        ++definedVirtualFunctions;
      }

      OA::OA_ptr<OA::IRStmtIterator> stmtIterPtr = irInterface->getStmtIterator(currProc);
      // Iterate over the statements of this block adding procedure references
      for ( ; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {

	OA::StmtHandle stmt = stmtIterPtr->current();

	OA::OA_ptr<OA::IRCallsiteIterator> callIter = irInterface->getCallsites(stmt);
        for ( ; callIter->isValid(); (*callIter)++ ) {
	  OA::CallHandle call = callIter->current();

          SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(irInterface->getNodePtr(call));
          if ( functionCallExp != NULL ) {
              SgFunctionDeclaration *functionDeclaration = 
                  getFunctionDeclaration(functionCallExp);

	      bool isDotExp = false;
	      bool isLhsRefOrPtr = false;
	      SgMemberFunctionRefExp *functionRefExp = NULL;
	      if ( ( functionRefExp = isMethodCall(functionCallExp, isDotExp, isLhsRefOrPtr) ) != NULL ) {
		// If this is a qualified method (e.g., Parent::answerName),
                // then it is not virtual.
                if ( !functionRefExp->get_need_qualifier() && !isParentCtorOrDtor(functionCallExp) && isVirtual(functionDeclaration) ) {
		  if ( ( !isDotExp || isLhsRefOrPtr ) ) {
		    //		    std::cout << "virtual callsite: " << functionCallExp->unparseToString() << std::endl;
		    ++invokedVirtualFunctions;
		  }
		}
	      }
          }
        }
      }
  }
}

void usage(char **argv)
{
  std::cerr << "usage: " << argv[0] << " -out:outputFile < compiler command line > " << std::endl;
  exit(-1);
}


/** \brief  Hash a SgNode to a number.
 *  \param  node  A Sage node.
 *  \return  A hash value derived from the node.
 *
 *  hashSgNode is used by abstractionToImplementationSurjection.
 *  In theory, node could be any type of SgNode.  In practice,
 *  an abstractionToImplementationSurjection only currently
 *  allows procedure abstractions.  Thus, node represents
 *  a procedure abstraction and is a SgFunctionDeclaration.
 */
class hashSgNode
{
  public:
  size_t operator()(SgNode *node) const;
};

/** \brief  Determine whether two Sage nodes are "equal."
 *  \param  node1  A Sage node.
 *  \param  node2  Anoter Sage node.
 *  \return  Boolean indicating whether node1 and node2 are "equal."
 *
 *  Equal does not mean address-equality!  Equality has semantic
 *  connotations-- that is, it is defined on a per-SgNode type basis.
 *  In theory, node1 and node2 could have any type.
 *  However, eqSgNodes is intended for use by 
 *  abstractionToImplementationSurjection. In practice,
 *  an abstractionToImplementationSurjection only currently
 *  allows procedure abstractions.  Thus, node1 and node2 represent
 *  a procedure abstraction and is a SgFunctionDeclaration.
 *  SgFunctionDeclaration equality means that the functions/methods
 *  have the same name and have the same type signature.
 */
class eqSgNodes
{
 public:
  bool operator()(const SgNode *node1, const SgNode *node2) const;
};

size_t hashSgNode::operator()(SgNode *node) const
{
  hash<char*> hasher;
  
  ROSE_ASSERT(node != NULL);
  std::string str = node->unparseToString();
  
  //  std::cout << "str in hash is: " << str << std::endl;

  return hasher(str.c_str());
};

bool eqSgNodes::operator()(const SgNode *node1, const SgNode *node2) const
{
  if ( node1->variantT() != node2->variantT() )
    return false;

  switch(node1->variantT()) {
  case V_SgFunctionDefinition:
    {
      const SgFunctionDefinition *defn1 = isSgFunctionDefinition(node1);
      ROSE_ASSERT(defn1 != NULL);

      Sg_File_Info *fileInfo1 = defn1->get_file_info();
      ROSE_ASSERT(fileInfo1 != NULL);

      const SgFunctionDefinition *defn2 = isSgFunctionDefinition(node2);
      ROSE_ASSERT(defn2 != NULL);

      Sg_File_Info *fileInfo2 = defn2->get_file_info();
      ROSE_ASSERT(fileInfo2 != NULL);

      if ( *fileInfo1 == *fileInfo2 )
        return true;           

      break;
    }
  default:
    {
      ROSE_ABORT();
      break;
    }
  }

  return false;
}

/** Find main within a program. */
SgFunctionDeclaration *findMain(SgProject *project)
{
    ROSE_ASSERT(project != NULL);
    std::list<SgNode *> nodes = NodeQuery::querySubTree(project,
                                                        V_SgFunctionDeclaration);

    SgFunctionDeclaration *mainFunc = NULL;
    for (std::list<SgNode *>::iterator it = nodes.begin();
         it != nodes.end(); ++it ) {
        SgFunctionDeclaration *decl = isSgFunctionDeclaration(*it);      
        ROSE_ASSERT(decl != NULL);

        SgName name = decl->get_name();
        if ( !strcmp("main", name.str() ) ) {
            mainFunc = decl;
            break;
        }
    }
    return mainFunc;
}
      
int main(int argc, char **argv)
{
    int modifiedArgc; 
    char **modifiedArgv = NULL; 

    std::list<std::string> argList =  
        CommandlineProcessing::generateArgListFromArgcArgv(argc, argv); 

    // Get the path to the output file. 
    const std::string outputFilePrefix = std::string("-out:"); 
    std::list<std::string> tmp; 

    tmp = CommandlineProcessing::generateOptionList(argList, 
                                                    outputFilePrefix); 
    if (tmp.size() != 1) { 
        usage(argv);
    }
    std::string outputFile = *(tmp.begin());

    std::ofstream ostr(outputFile.c_str(), std::ios::out | std::ios::app); 
    ROSE_ASSERT(ostr.is_open()); 

    // Generate a command line to pass to ROSE, stripped of the
    // output file name specified via -out:.
    CommandlineProcessing::generateArgcArgvFromList(argList,  
                                                    modifiedArgc, 
                                                    modifiedArgv); 
   
    struct timeval beginFrontend;
    struct timeval endFrontend;
    struct timeval beginNormalize;
    struct timeval endNormalize;
    struct timeval beginCHA;
    struct timeval endCHA;
    struct timeval beginFIAlias;
    struct timeval endFIAlias;
    struct timeval beginFIAliasResolution;
    struct timeval endFIAliasResolution;


    gettimeofday(&beginFrontend, NULL);

    // Parse the input files.
    SgProject *project = frontend(modifiedArgc, modifiedArgv);
  
    gettimeofday(&endFrontend, NULL);

    gettimeofday(&beginNormalize, NULL);

#if 1
    // Perform the AST normalization.
    //    DefaultFunctionGenerator dfg;
    //    dfg.traverse(project, preorder);
    defaultFunctionGenerator(project);
    AstPostProcessing(project);
    shortCircuitingTransformation(project);
    //    destructorCallAnnotator(project);
#endif

    gettimeofday(&endNormalize, NULL);

#if 0
    // debug output
    AstPDFGeneration pdftest;
    pdftest.generateInputFiles(project);
    AstDOTGeneration dottest;
    dottest.generateInputFiles(project);
#endif
    int definedFunctions = 0;
    int definedVirtualFunctions = 0;
    int invokedVirtualFunctions = 0;
    int numStatements = 0;
    int numMemRefs = 0;  

    OA::OA_ptr<SageIRInterface> irInterface;
    std::vector<SgNode*> nodeArray;
    bool p_h=FALSE; //for debugging only switch between persistent and "pointer" handles (pointers are faster, persistent are easier to debug

    irInterface = new SageIRInterface(project, &nodeArray, p_h);

    countFunctions(definedFunctions, definedVirtualFunctions, 
                   invokedVirtualFunctions,
                   numStatements,
                   numMemRefs,
                   irInterface, project);

    gettimeofday(&beginCHA, NULL);

    // Do not _statically_ count virtual function invocations,
    // instead do so through a proper call graph analysis.
    // Further we need to do this for each analysis, as they
    // leave of conservatism may yield different results.
    invokedVirtualFunctions = 0;

    // The worklist holds those functions that need to be processed.
    std::vector<SgFunctionDefinition *> worklist;
  
    // processedFunctions holds those functions that have already been
    // processed.
    hash_set<SgFunctionDefinition *, hashSgNode, eqSgNodes> processedFunctions;

    // Get the main function.
    SgFunctionDeclaration *mainFunc = findMain(project);
    ROSE_ASSERT(mainFunc != NULL);  

    mainFunc = getDefiningDeclaration(mainFunc);
    ROSE_ASSERT(mainFunc != NULL);  

    SgFunctionDefinition *mainDefn = mainFunc->get_definition();
    ROSE_ASSERT(mainDefn != NULL);

    worklist.push_back(mainDefn);
    processedFunctions.insert(mainDefn);

    // First, perform CHA.
    unsigned int numCHACallSites = 0;
    unsigned int numCHAMonomorphicCallSites = 0;
    unsigned int numCHAPossibleResolutions = 0;
    unsigned int numCHAInvokedVirtualFunctions = 0;

    // Instantiate a class hierarchy wrapper.
    ClassHierarchyWrapper classHierarchy( project );

    // We will only report results for those functions that are
    // actually invoked-- i.e., are reachable.  To do this, we keep
    // a worklist and initialize it with main.  Since CHA and
    // alias analysis could potentially reach different functions,
    // we will need to do a separate pass for each of them.

    // Iterate over the worklist until it is empty.
    // Note that worklist.size() is modified within this loop.  Do not
    // remove worklist.size() from conditional.
    for (int i = 0; i < worklist.size(); ++i) {

        SgFunctionDefinition *defn = worklist[i];
        ROSE_ASSERT(defn != NULL);

	std::cout << "CHA examining: " << defn->get_mangled_name().str() << std::endl;

        // Visit each call site in this function.
        std::list<SgNode *> callsites;
        VariantVector v(V_SgFunctionCallExp);
        v = v + V_SgConstructorInitializer;
        v = v + V_SgDeleteExp;
        callsites = NodeQuery::querySubTree(defn, v);

        for (std::list<SgNode *>::iterator funcCallIt = callsites.begin();
             funcCallIt != callsites.end(); ++funcCallIt ) {

            numCHACallSites++;

            SgNode *n = *funcCallIt;
            ROSE_ASSERT(n != NULL);

            SgConstructorInitializer *ctorIniter = 
                isSgConstructorInitializer(n);
            if ( ctorIniter != NULL ) {
                // Constructor invocations are not indirect/virtual calls.
                // Add the constructor definition to the worklist.

                // Get the declaration of the function.
                SgFunctionDeclaration *decl = ctorIniter->get_declaration();
                ROSE_ASSERT(decl != NULL);

                decl = getDefiningDeclaration(decl);
                ROSE_ASSERT(decl != NULL);

                SgFunctionDefinition *defn = decl->get_definition();
                ROSE_ASSERT(defn != NULL);

                if ( processedFunctions.find(defn) ==
                     processedFunctions.end() ) {
                    worklist.push_back(defn);
                    processedFunctions.insert(defn);
                }
                continue;
            }

            SgMemberFunctionDeclaration *memberFunctionDeclaration = NULL;

            SgDeleteExp *deleteExp = isSgDeleteExp(n);
            if ( deleteExp != NULL ) {

                SgExpression *receiver = deleteExp->get_variable();
                ROSE_ASSERT(receiver != NULL);

                SgType *type = receiver->get_type(); 
                ROSE_ASSERT(type != NULL);

                SgPointerType *ptrType = isSgPointerType(type);
                ROSE_ASSERT(ptrType != NULL);
                type = getBaseType(ptrType->get_base_type());

                ROSE_ASSERT(type != NULL);

                // If this is a base type, then do nothing more.
                if ( !isSgClassType(type) ) {
                    continue;
                }

                SgClassDefinition *classDefn = getClassDefinition(type);
                ROSE_ASSERT(classDefn != NULL);
    
                memberFunctionDeclaration =
                    lookupDestructorInClass(classDefn); 
                ROSE_ASSERT(memberFunctionDeclaration != NULL); 
    
                // If the destructor is virtual, we need to create
                // an invocation through a field access.
                if ( !isVirtual(memberFunctionDeclaration) || isParentCtorOrDtor(deleteExp) ) {
                    memberFunctionDeclaration = 
                        isSgMemberFunctionDeclaration(getDefiningDeclaration(memberFunctionDeclaration));
                    ROSE_ASSERT(memberFunctionDeclaration != NULL);
                    SgFunctionDefinition *defn = 
                        memberFunctionDeclaration->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }
                    continue;   
                }
            }

            SgFunctionCallExp *functionCallExp =
                isSgFunctionCallExp(n);
            if (functionCallExp != NULL) {

                // We are only interested in examining method invocations.
                bool isDotExp = false;
                bool isLhsRefOrPtr = false;

#ifdef DEBUG    
                std::cout << "method?: " 
                          << functionCallExp->unparseToCompleteString() 
                          << std::endl;
#endif

                SgMemberFunctionRefExp *functionRefExp = 
                    isMethodCall(functionCallExp, isDotExp, isLhsRefOrPtr);
                if ( functionRefExp == NULL ) {
                    // This is not a method call, but we still need to 
                    // visit it.  Add to the worklist.
                    SgFunctionDeclaration *decl = getFunctionDeclaration(functionCallExp);
                    decl = getDefiningDeclaration(decl);
                    if ( decl == NULL ) {
                        std::cout << "NULL decl for " << functionCallExp->unparseToString() << std::endl;
                        continue;
                    } 

                    SgFunctionDefinition *defn = decl->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }
                    continue;
                }
    
#ifdef DEBUG    
                std::cout << "method: " 
                          << functionCallExp->unparseToCompleteString() 
                          << std::endl;
#endif

                if ( isDotExp && !isLhsRefOrPtr ) {
                    // If this is a dot expression (i.e., a.foo()), we can
                    // statically determine its type-- unless the left-hand
                    // side is a reference type.
#ifdef DEBUG    
                    std::cout << "dot: " 
                              << functionCallExp->unparseToCompleteString() 
                              << std::endl;
#endif
                    // This is not a method call, but we still need to 
                    // visit it.  Add to the worklist.
	            SgFunctionDeclaration *decl = getFunctionDeclaration(functionCallExp);
                    decl = getDefiningDeclaration(decl);
                    if ( decl == NULL ) {
                        std::cout << "NULL decl for " << functionCallExp->unparseToString() << std::endl;
                        continue;
                    } 
                    SgFunctionDefinition *defn = decl->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }

                    continue;
                }

                // Retrieve the static function declaration.
                memberFunctionDeclaration = 
                    isSgMemberFunctionDeclaration(getFunctionDeclaration(functionCallExp));

                if ( ( functionRefExp->get_need_qualifier() ||
                       isParentCtorOrDtor(functionCallExp) || 
                       !isVirtual(memberFunctionDeclaration) ) ) {

                    SgFunctionDeclaration *decl = 
                        getFunctionDeclaration(functionCallExp);
                    decl = getDefiningDeclaration(decl);
                    if ( decl == NULL ) {
                        std::cout << "NULL decl for " << functionCallExp->unparseToString() << std::endl;
                        continue;
                    } 

                    SgFunctionDefinition *defn = decl->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }
                    continue;
                }

            }
            ROSE_ASSERT(memberFunctionDeclaration != NULL);

#ifdef DEBUG    
            std::cout << "methodPtr: " 
                      << n->unparseToCompleteString() 
                      << std::endl;
#endif
            unsigned int numCHAResolutionsForMethod = 0;
	    std::vector <SgFunctionDeclaration *> resolutions;

#ifdef DEBUG    
            std::cout << "tracking: " 
                      << memberFunctionDeclaration->unparseToString() 
                      << std::endl;
#endif
            numCHAInvokedVirtualFunctions++;

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
      
            // We do not have to recurse through the class hierarchy
            // (as findResolutionsForMethodInTypeHierarchy
            // does).  In fact, we _should_ not since getSubClasses(cls)
            // returns _all_ of the subclasses of cls, not just its
            // immediate subclasses.
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
                if ( matchingFunctions(method,
                                       memberFunctionDeclaration) ) {
#ifdef DEBUG    
                    std::cout << "overrides" << std::endl;
#endif
                    // Do not consider a pure virtual method to be an 
                    // overriding method (since it can not be invoked).
                    if ( !isPureVirtual(method) || hasDefinition(method) ) {
                        numCHAResolutionsForMethod++;
                        // CHA believes this method may be invoked
                        // so add it to the work list.
                        method = isSgMemberFunctionDeclaration(getDefiningDeclaration(method));
                        ROSE_ASSERT(method != NULL);  

                        SgFunctionDefinition *enclosingFuncDefn =
                            getEnclosingFunction(functionCallExp); 
                        ROSE_ASSERT(enclosingFuncDefn != NULL);

                        Sg_File_Info *fileInfo = functionCallExp->get_file_info();
                        ROSE_ASSERT(fileInfo != NULL);

                        resolutions.push_back(method);
                        string methodName = method->get_name().str();
                        if ( isSgMemberFunctionDeclaration(method) ) {
                            SgClassDefinition *clsDefn = 
                                isSgClassDefinition(isSgMemberFunctionDeclaration(method)->get_scope());
                            ROSE_ASSERT(clsDefn != NULL);
                            methodName = clsDefn->get_qualified_name().str();
                            methodName += "::";
                            methodName += method->get_name().str(); 
                        }
		        std::cout << "CHA resolution for call site: "
                                  << functionCallExp->unparseToString() 
                                  << " in caller "
                                  << enclosingFuncDefn->get_mangled_name().str()
			          << " at line "
                                  << StringUtility::stripPathFromFileName(fileInfo->get_filename())
                                  << ":"
                                  << fileInfo->get_line()
                                  << ": " 

			  //                                  << method->unparseToString()
                                  << methodName 
                                  << std::endl;
                        SgFunctionDefinition *defn = 
                            method->get_definition();
                        ROSE_ASSERT(defn != NULL);

                        if ( processedFunctions.find(defn) ==
                             processedFunctions.end() ) {
                            worklist.push_back(defn);
                            processedFunctions.insert(defn);
                        }
                    }
                }
            }

            // Now look within the sub classes.
            SgClassDefinitionPtrList subclasses = 
                classHierarchy.getSubclasses(classDefinition);

#ifdef DEBUG
            if ( classDefinition != NULL ) {
                std::cout << "Looking up subclasses of " 
                          << classDefinition << " " 
                          << classDefinition->get_qualified_name().str() 
                          << std::endl;
            }
#endif

            // XXX: this is very repetitive with the above.
            // We should just throw the original class and its
            // subclasses into a list and then iterate over all of them
            // to avoid redundant code.
            // Iterate over all subclasses.
            for (SgClassDefinitionPtrList::iterator subclassIt = subclasses.begin();
                 subclassIt != subclasses.end(); ++subclassIt) {

                SgClassDefinition *subclass = *subclassIt;
                ROSE_ASSERT(subclass != NULL);

#ifdef DEBUG    
                std::cout << "subclass of " 
                          << classDefinition->get_qualified_name().str() 
                          << " got: " 
                          << subclass->get_qualified_name().str() 
                          << std::endl;
#endif

                // Iterate over all of the methods defined in the subclass.
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
                    // Determine whether subclass of the class defining 
                    // this method overrides the method.
                    if ( matchingFunctions(method,
                                           memberFunctionDeclaration) ) {
#ifdef DEBUG    
                        std::cout << "overrides" << std::endl;
#endif
                        // Do not consider a pure virtual method to be an 
                        // overriding method (since it can not be invoked).
                        if ( !isPureVirtual(method) || 
                             hasDefinition(method) ) {
 
                            numCHAResolutionsForMethod++;


                            // CHA believes this method may be invoked
                            // so add it to the work list.
                            method = isSgMemberFunctionDeclaration(getDefiningDeclaration(method));
                            ROSE_ASSERT(method != NULL);  

                            SgFunctionDefinition *enclosingFuncDefn =
                                getEnclosingFunction(functionCallExp); 
                            ROSE_ASSERT(enclosingFuncDefn != NULL);

                            Sg_File_Info *fileInfo = functionCallExp->get_file_info();
                            ROSE_ASSERT(fileInfo != NULL);

 		            std::cout << "CHA resolution for call site: "
                                      << n->unparseToString() 
                                      << " in caller "
                                      << enclosingFuncDefn->get_mangled_name().str()
	    		              << " at line "
                                      << StringUtility::stripPathFromFileName(fileInfo->get_filename())
                                      << ":"
                                      << fileInfo->get_line()
                                      << ": " 
                                      << method->unparseToString()
                                      << std::endl;

                            resolutions.push_back(method);

                            SgFunctionDefinition *defn = 
                                method->get_definition();
                            ROSE_ASSERT(defn != NULL);

                            if ( processedFunctions.find(defn) ==
                                 processedFunctions.end() ) {
                                worklist.push_back(defn);
                                processedFunctions.insert(defn);
                            }
                        }
                    }
                }
	    }
	    
            if ( numCHAResolutionsForMethod == 1 ) {
                numCHAMonomorphicCallSites++;
            }
            numCHAPossibleResolutions += numCHAResolutionsForMethod;

            if ( numCHAResolutionsForMethod >= 1 ) {
                SgFunctionDefinition *enclosingFuncDefn =
                    getEnclosingFunction(n);
                ROSE_ASSERT(enclosingFuncDefn != NULL);

                Sg_File_Info *fileInfo = n->get_file_info();
                ROSE_ASSERT(fileInfo != NULL);

                SgFunctionDeclaration *callerDecl =
                    enclosingFuncDefn->get_declaration();
                ROSE_ASSERT(callerDecl != NULL);
                string callerName = callerDecl->get_name().str();
                if ( isSgMemberFunctionDeclaration(callerDecl) ) {
                    SgClassDefinition *clsDefn = 
                        isSgClassDefinition(isSgMemberFunctionDeclaration(callerDecl)->get_scope());
                    ROSE_ASSERT(clsDefn != NULL);
                    callerName = clsDefn->get_qualified_name().str();
                    callerName += "::";
                    callerName += callerDecl->get_name().str(); 
                }

                ostr << "CHA virtual call site: " 
                     << n->unparseToCompleteString() 
                     << " from caller: " 
		  //                     << enclosingFuncDefn->get_mangled_name().str()
                     << callerName
	    	     << " at line "
                     << StringUtility::stripPathFromFileName(fileInfo->get_filename())
                     << ":"
                     << fileInfo->get_line()
                     << std::endl;
                ostr << "\t CHA:  Method invocation has " 
                     << numCHAResolutionsForMethod 
                     << " possible resolutions " 
                     << std::endl;

                for (int i = 0; i < resolutions.size(); ++i) {
                    SgFunctionDeclaration *decl = resolutions[i];
                    string methodName = decl->get_name().str();
                    if ( isSgMemberFunctionDeclaration(decl) ) {
                        SgClassDefinition *clsDefn = 
                            isSgClassDefinition(isSgMemberFunctionDeclaration(decl)->get_scope());
                        ROSE_ASSERT(clsDefn != NULL);
                        methodName = clsDefn->get_qualified_name().str();
                        methodName += "::";
                        methodName += decl->get_name().str(); 
                    }
                    ostr << "\t\t" << methodName << std::endl;
                }
                ostr << std::endl;
            }
        }
    }

    gettimeofday(&endCHA, NULL);

    unsigned int numAliasAnalysisCallSites = 0;
    unsigned int numAliasAnalysisMonomorphicCallSites = 0;
    unsigned int numAliasAnalysisPossibleResolutions = 0;
    unsigned int numAliasAnalysisInvokedVirtualFunctions = 0;

#if 1
    gettimeofday(&beginFIAlias, NULL);

    // Perform the alias analysis.
    OA::OA_ptr<OA::Alias::InterAliasMap> interAlias =
        DoFIAliasAliasMap(irInterface, project);

    gettimeofday(&endFIAlias, NULL);

    gettimeofday(&beginFIAliasResolution, NULL);

    SgNode *root = project;

    // Reset the worklist and set of processed functions.
    worklist.clear();
    processedFunctions.clear();

    // Initialize the worklist.
    worklist.push_back(mainDefn);
    processedFunctions.insert(mainDefn);

    // Now perform the alias-based virtual method resolution.
    // Logically, this looks like:
    // for all procs proc in SageIRProcIterator
    //   for all statements stmt in getStmtIterator(proc)
    //     for all call sites cs in getCallsites(stmt)
    //       for all call MREs callmre in getCAllMemRefExp(cs)
    //         OA_ptr<LocIterator> locIter =
    //           alias->getMayLocs(callmre, proc)
    //         cout << "callsite has " << locIter.size << "resolutions"

    // Iterate over the worklist until it is empty.
    // Note that worklist.size() is modified within this loop.  Do not
    // remove worklist.size() from conditional.
    for (int i = 0; i < worklist.size(); ++i) {

        SgFunctionDefinition *defn = worklist[i];
        ROSE_ASSERT(defn != NULL);

	std::cout << "Alias analysis examining: " << defn->get_mangled_name().str() << std::endl;

        // Visit each call site in this function.
        std::list<SgNode *> callsites;
        VariantVector v(V_SgFunctionCallExp);
        v = v + V_SgConstructorInitializer;
        v = v + V_SgDeleteExp;
        callsites = NodeQuery::querySubTree(defn, v);

        for (std::list<SgNode *>::iterator funcCallIt = callsites.begin();
             funcCallIt != callsites.end(); ++funcCallIt ) {

            numAliasAnalysisCallSites++;

            SgNode *n = *funcCallIt;
            ROSE_ASSERT(n != NULL);

            SgConstructorInitializer *ctorIniter = 
                isSgConstructorInitializer(n);
            if ( ctorIniter != NULL ) {
                // Constructor invocations are not indirect/virtual calls.
                // Add the constructor definition to the worklist.

                // Get the declaration of the function.
                SgFunctionDeclaration *decl = ctorIniter->get_declaration();
                ROSE_ASSERT(decl != NULL);

                decl = getDefiningDeclaration(decl);
                ROSE_ASSERT(decl != NULL);

                SgFunctionDefinition *defn = decl->get_definition();
                ROSE_ASSERT(defn != NULL);

                if ( processedFunctions.find(defn) ==
                     processedFunctions.end() ) {
                    worklist.push_back(defn);
                    processedFunctions.insert(defn);
                }
                continue;
            }

            SgMemberFunctionDeclaration *memberFunctionDeclaration = NULL;

            SgDeleteExp *deleteExp = isSgDeleteExp(n);
            if ( deleteExp != NULL ) {

                SgExpression *receiver = deleteExp->get_variable();
                ROSE_ASSERT(receiver != NULL);

                SgType *type = receiver->get_type(); 
                ROSE_ASSERT(type != NULL);

                SgPointerType *ptrType = isSgPointerType(type);
                ROSE_ASSERT(ptrType != NULL);
                type = getBaseType(ptrType->get_base_type());

                ROSE_ASSERT(type != NULL);

                // If this is a base type, then do nothing more.
                if ( !isSgClassType(type) ) {
                    continue;
                }

                SgClassDefinition *classDefn = getClassDefinition(type);
                ROSE_ASSERT(classDefn != NULL);
    
                memberFunctionDeclaration =
                    lookupDestructorInClass(classDefn); 
                ROSE_ASSERT(memberFunctionDeclaration != NULL); 
    
                // If the destructor is virtual, we need to create
                // an invocation through a field access.
                if ( !isVirtual(memberFunctionDeclaration) || isParentCtorOrDtor(deleteExp)) {
                    memberFunctionDeclaration = 
                        isSgMemberFunctionDeclaration(getDefiningDeclaration(memberFunctionDeclaration));
                    ROSE_ASSERT(memberFunctionDeclaration != NULL);
                    SgFunctionDefinition *defn = 
                        memberFunctionDeclaration->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }
                    continue;   
                }
            }

            SgFunctionCallExp *functionCallExp =
                isSgFunctionCallExp(n);
            if (functionCallExp != NULL) {

                // We are only interested in examining method invocations.
                bool isDotExp = false;
                bool isLhsRefOrPtr = false;

#ifdef DEBUG    
                std::cout << "method?: " 
                          << functionCallExp->unparseToCompleteString() 
                          << std::endl;
#endif

                SgMemberFunctionRefExp *functionRefExp = 
                    isMethodCall(functionCallExp, isDotExp, isLhsRefOrPtr);
                if ( functionRefExp == NULL ) {
                    // This is not a method call, but we still need to 
                    // visit it.  Add to the worklist.
                    SgFunctionDeclaration *decl = getFunctionDeclaration(functionCallExp);
                    decl = getDefiningDeclaration(decl);
                    if ( decl == NULL ) {
                        std::cout << "NULL decl for " << functionCallExp->unparseToString() << std::endl;
                        continue;
                    } 

                    SgFunctionDefinition *defn = decl->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }
                    continue;
                }
    
#ifdef DEBUG    
                std::cout << "method: " 
                          << functionCallExp->unparseToCompleteString() 
                          << std::endl;
#endif

                if ( isDotExp && !isLhsRefOrPtr ) {
                    // If this is a dot expression (i.e., a.foo()), we can
                    // statically determine its type-- unless the left-hand
                    // side is a reference type.
#ifdef DEBUG    
                    std::cout << "dot: " 
                              << functionCallExp->unparseToCompleteString() 
                              << std::endl;
#endif
                    // This is not a method call, but we still need to 
                    // visit it.  Add to the worklist.
	            SgFunctionDeclaration *decl = getFunctionDeclaration(functionCallExp);
                    decl = getDefiningDeclaration(decl);
                    if ( decl == NULL ) {
                        std::cout << "NULL decl for " << functionCallExp->unparseToString() << std::endl;
                        continue;
                    } 
                    SgFunctionDefinition *defn = decl->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }

                    continue;
                }

                // Retrieve the static function declaration.
                memberFunctionDeclaration = 
                    isSgMemberFunctionDeclaration(getFunctionDeclaration(functionCallExp));

                if ( ( functionRefExp->get_need_qualifier() ||
                       isParentCtorOrDtor(functionCallExp) || 
                       !isVirtual(memberFunctionDeclaration) ) ) {

                    SgFunctionDeclaration *decl = 
                        getFunctionDeclaration(functionCallExp);
                    decl = getDefiningDeclaration(decl);
                    if ( decl == NULL ) {
                        std::cout << "NULL decl for " << functionCallExp->unparseToString() << std::endl;
                        continue;
                    } 

                    SgFunctionDefinition *defn = decl->get_definition();
                    ROSE_ASSERT(defn != NULL);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }
                    continue;
                }

            }
            ROSE_ASSERT(memberFunctionDeclaration != NULL);

#ifdef DEBUG    
            std::cout << "methodPtr: " 
                      << n->unparseToCompleteString() 
                      << std::endl;
#endif
            unsigned int numAliasAnalysisResolutionsForMethod = 0;
	    std::vector <SgFunctionDeclaration *> resolutions;

#ifdef DEBUG    
            std::cout << "tracking: " 
                      << memberFunctionDeclaration->unparseToString() 
                      << std::endl;
#endif
            numAliasAnalysisInvokedVirtualFunctions++;

            // We already have a Sage handle to a callsite (and no handle
            // to the enclosing procedure, proc).
            // First, find the enclosing function for the call site.
            // Then convert the Sage handles for callsites and the
            // enclosing function definition to OA CallHandles 
            // and ProcHandles.
            SgFunctionDefinition *enclosingFuncDefn =
                getEnclosingFunction(functionCallExp);
            ROSE_ASSERT(enclosingFuncDefn != NULL);

            // Get an OA CallHandle.
            OA::CallHandle callHandle = 
                irInterface->getProcExprHandle(functionCallExp);

            // Get an OA ProcHandle.
            OA::ProcHandle caller = 
                irInterface->getProcHandle(enclosingFuncDefn);

            // Get all of the Call MemRefExprs at the callsite.  I think 
            // we are expecting only one?  Whoops ... interface only
            // allows one.
            OA::OA_ptr<OA::MemRefExpr> callMRE = 
                irInterface->getCallMemRefExpr(callHandle);
            ROSE_ASSERT(!callMRE.ptrEqual(0));

            // How many locations has the alias analysis assigned to this
            // callMRE?
            OA::OA_ptr<OA::Alias::AliasMap> alias =
                interAlias->getAliasMapResults(caller);
            OA::OA_ptr<OA::LocIterator> locIter =
                alias->getMayLocs(*callMRE, caller);

            for (locIter->reset(); locIter->isValid(); (*locIter)++) {
                // Don't count invisible locations:
                OA::OA_ptr<OA::Location> loc = locIter->current();
                if ( !loc->isaInvisible() ) {
                    ++numAliasAnalysisResolutionsForMethod;
                    SgFunctionDefinition *enclosingFuncDefn =
                        getEnclosingFunction(functionCallExp); 
                    ROSE_ASSERT(enclosingFuncDefn != NULL);

                    Sg_File_Info *fileInfo = functionCallExp->get_file_info();
                    ROSE_ASSERT(fileInfo != NULL);

                    std::cout << "Alias analysis resolution for call site " 
                              << functionCallExp->unparseToString() 
                              << " in caller "
                              << enclosingFuncDefn->get_mangled_name().str()
	    		      << " at line "
                              << StringUtility::stripPathFromFileName(fileInfo->get_filename())
                              << ":"
                              << fileInfo->get_line()
                              << ": "; 


                    // Need to convert this OA visible location
                    // to a Sage function definition, so that
                    // we may add it to the work list.

                    ROSE_ASSERT(loc->isaNamed());
 
                    OA::OA_ptr<OA::NamedLoc> namedLoc = 
                        loc.convert<OA::NamedLoc>();
                    ROSE_ASSERT(!namedLoc.ptrEqual(0));

                    OA::SymHandle symHandle = namedLoc->getSymHandle();
		    OA::ProcHandle procHandle = 
                        irInterface->getProcHandle(symHandle);

                    SgFunctionDefinition *defn = 
                        irInterface->getSgNode(procHandle);
                    ROSE_ASSERT(defn != NULL);

                    SgFunctionDeclaration *decl = 
                        defn->get_declaration();
                    ROSE_ASSERT(decl != NULL);
                    resolutions.push_back(decl);

                    if ( processedFunctions.find(defn) ==
                         processedFunctions.end() ) {
                        worklist.push_back(defn);
                        processedFunctions.insert(defn);
                    }

                } else {
                    std::cout << "Invisible Location: " << std::endl;
                    std::cout << "with respect to caller: " 
                              << irInterface->toString(caller) 
                              << std::endl;
                    std::cout << "call site: ";
                    irInterface->dump(callMRE, std::cout);
                    std::cout << std::endl;
                }
                loc->output(*irInterface);
            }
	    
            if ( numAliasAnalysisResolutionsForMethod == 1 ) {
                numAliasAnalysisMonomorphicCallSites++;
            }
            numAliasAnalysisPossibleResolutions += numAliasAnalysisResolutionsForMethod;

            if ( numAliasAnalysisResolutionsForMethod >= 1 ) {
                SgFunctionDefinition *enclosingFuncDefn =
                    getEnclosingFunction(n);
                ROSE_ASSERT(enclosingFuncDefn != NULL);

                Sg_File_Info *fileInfo = n->get_file_info();
                ROSE_ASSERT(fileInfo != NULL);

                SgFunctionDeclaration *callerDecl =
                    enclosingFuncDefn->get_declaration();
                ROSE_ASSERT(callerDecl != NULL);
                string callerName = callerDecl->get_name().str();
                if ( isSgMemberFunctionDeclaration(callerDecl) ) {
                    SgClassDefinition *clsDefn = 
                        isSgClassDefinition(isSgMemberFunctionDeclaration(callerDecl)->get_scope());
                    ROSE_ASSERT(clsDefn != NULL);
                    callerName = clsDefn->get_qualified_name().str();
                    callerName += "::";
                    callerName += callerDecl->get_name().str(); 
                }

                ostr << "Alias analysis virtual call site: " 
                     << n->unparseToCompleteString() 
                     << " from caller: " 
		  //                     << enclosingFuncDefn->get_mangled_name().str()
                     << callerName
	    	     << " at line "
                     << StringUtility::stripPathFromFileName(fileInfo->get_filename())
                     << ":"
                     << fileInfo->get_line()
                     << std::endl;
                ostr << "\t Alias analysis:  Method invocation has " 
                     << numAliasAnalysisResolutionsForMethod 
                     << " possible resolutions " 
                     << std::endl;

                for (int i = 0; i < resolutions.size(); ++i) {
                    SgFunctionDeclaration *decl = resolutions[i];
                    string methodName = decl->get_name().str();
                    if ( isSgMemberFunctionDeclaration(decl) ) {
                        SgClassDefinition *clsDefn = 
                            isSgClassDefinition(isSgMemberFunctionDeclaration(decl)->get_scope());
                        ROSE_ASSERT(clsDefn != NULL);
                        methodName = clsDefn->get_qualified_name().str();
                        methodName += "::";
                        methodName += decl->get_name().str(); 
                    }
                    ostr << "\t\t" << methodName << std::endl;
                }
                ostr << std::endl;
            }
        }
    }

    gettimeofday(&endFIAliasResolution, NULL);
#endif
    ostr << "number of statements (as counted by OpenAnalysis): " 
         << numStatements 
         << std::endl;

    ostr << "number of memory references (as counted by OpenAnalysis): " 
         << numMemRefs 
         << std::endl;

    ostr << "definedFunctions: " 
         << definedFunctions 
         << std::endl;

    ostr << "definedVirtualFunctions: " 
         << definedVirtualFunctions 
         << std::endl;

    ostr << "CHA virtual method callsites: " 
         << numCHAInvokedVirtualFunctions 
         << std::endl;

    ostr << "Alias analysis virtual method callsites: " 
         << numAliasAnalysisInvokedVirtualFunctions 
         << std::endl;

    ostr << "Summary CHA monomorphic call sites: " 
         << numCHAMonomorphicCallSites 
         << std::endl;

    ostr << "Summary Alias analysis monomorphic call sites: " 
         << numAliasAnalysisMonomorphicCallSites 
         << std::endl;

    ostr << "Summary CHA total (virtual) resolutions: " 
         << numCHAPossibleResolutions 
         << std::endl;

    ostr << "Summary Alias analysis total (virtual) resolutions: " 
         << numAliasAnalysisPossibleResolutions 
         << std::endl;

    ostr << std::endl;
    ostr << "Execution times in seconds (avg of 1 run; all times mutually exclusive)" << std::endl;
    double frontEndTime = diffTime(beginFrontend, endFrontend);
    double normalizeTime = diffTime(beginNormalize, endNormalize);
    double CHATime = diffTime(beginCHA, endCHA);
    double FIAliasTime = diffTime(beginFIAlias, endFIAlias);
    double FIAliasResolutionTime = diffTime(beginFIAliasResolution, endFIAliasResolution);
    ostr << "Frontend time: " << frontEndTime << std::endl;
    ostr << "Normalization time: " << normalizeTime << std::endl;
    ostr << "CHA time: " << CHATime << std::endl;
    ostr << "FIAlias time: " << FIAliasTime << std::endl;
    ostr << "FIAlias resolution time: " << FIAliasResolutionTime << std::endl;
  
    //  return 0;
    return backend(project);
}
