/**
 *  Invoke OpenAnalysis alias analysis on a ROSE AST.
 *  This is based on OATest, which is more sophisticated
 *  and accomodates all analyses supported by
 *  ROSE/OpenAnalysis.
 **/

#include <rose.h>

// Pull in the Sage2OA interface, which provides the ROSE
// implementation of the OpenAnalysis interface.
#include "Sage2OA.h"

// Pull in the OpenAnalysis alias interface.
#include <OpenAnalysis/Alias/ManagerFIAliasAliasMap.hpp>

// The following are required for AST normalization.
#include <defaultFunctionGenerator.h>
#include <shortCircuitingTransformation.h>
#include <destructorCallAnnotator.h>

/* Return the function declaration of main within the AST */
SgFunctionDeclaration *findMain(SgProject *project)
{
    ROSE_ASSERT(project != NULL);
    NodeQuerySynthesizedAttributeType nodes =
        NodeQuery::querySubTree(project,
                                V_SgFunctionDeclaration);

    SgFunctionDeclaration *mainFunc = NULL;
    for (NodeQuerySynthesizedAttributeType::iterator it = nodes.begin();
         it != nodes.end(); ++it ) 
    {
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

void usage(char **argv)
{
    cerr << "usage: " << argv[0] << " < compiler command line >" << endl;
    exit(-1);
}

int main(int argc, char **argv)
{
    if(argc < 2) {
        usage(argv);
    }

    // Parse the program.
    SgProject *sageProject = frontend(argc, argv);

    // Unfortunately, AST normalization is not comnpletely
    // implemented.
#if 0
    // Perform the AST normalization.
    //    DefaultFunctionGenerator dfg;
    //    dfg.traverse(sageProject, preorder);
    defaultFunctionGenerator(sageProject);
    AstPostProcessing(sageProject);
    shortCircuitingTransformation(sageProject);
    destructorCallAnnotator(sageProject);
#endif
    
    // Create the OpenAnalysis interface.
    OA::OA_ptr<SageIRInterface> irInterface;
  
    // The nodeArray and usePersistentHandles arguments
    // control whether OpenAnalysis uses pointer-based handles
    // (for efficiency) or persistent handles (for debugging).
    // In general, set usePersistentHandles = false;
    std::vector<SgNode*> nodeArray;
    bool usePersistentHandles = false; 
  
    irInterface = 
        new SageIRInterface(sageProject, &nodeArray, usePersistentHandles); 

    // OpenAnalysis implements two types of alias analysis
    // based on FIAlias, which was described by 
    // Ryder, Landi, Stocks, Zhang, and Altucher in
    // "A Schema for Interprocedural Side Effect Analysis with Pointer Aliasing"
    // TOPLAS, Vol 23, pages 105-186, March 2001.
    // FIAliasAliasMap is a straightforward implementation of the
    // original algorithm.
    // FIAliasReachableAliasMap is an optimized, iterative version that only
    // analyzes procedures that are (conservatively) reachable within
    // the program.

    // Perform the alias analysis.
    OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;

    // Create an FIAlias Manager to orchestrate the analysis.
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasManager;
    fialiasManager = new OA::Alias::ManagerFIAliasAliasMap(irInterface);

    bool performReachableAnalysis = false;
    if ( !performReachableAnalysis ) {
      
        // Create an iterator over all procedures in the AST.
        OA::OA_ptr<SageIRProcIterator> procIter;
        procIter = new SageIRProcIterator(sageProject, *irInterface);
  
        // Invoke the analysis.
        interAlias = fialiasManager->performAnalysis(procIter);

    } else {

        // Create an iterator that only holds the main procedure.
        OA::OA_ptr<SageIRProcIterator> procIter;
        procIter = new SageIRProcIterator(findMain(sageProject), *irInterface);
  
        // Specify that we should use the FIAliasREACHABLE implementation
        // of the analysis.
        OA::Alias::FIAliasImplement implement = OA::Alias::REACHABLE_PROCS;
        interAlias = fialiasManager->performAnalysis(procIter, implement);
    }

    // Output the alias results.
    interAlias->output(*irInterface);

    return backend(sageProject);
    // return 0;
}
