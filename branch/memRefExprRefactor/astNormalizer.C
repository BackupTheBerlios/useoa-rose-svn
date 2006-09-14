#ifdef HAVE_CONFIG_H
// #include <config.h>
#endif
#include <rose.h>
#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include <string>
#include <iostream>
#include <OpenAnalysis/Alias/NotationGenerator.hpp>
#include <CommandOptions.h>

#include <defaultFunctionGenerator.h>
#include <shortCircuitingTransformation.h>
#include <destructorCallAnnotator.h>

int main ( unsigned argc,  char * argv[] )
{
    // load the Sage project, open the output file, and construct the
    // code generator (which outputs .oa notation to the file)
    SgProject * sageProject = frontend((int)argc, &argv[0]);

    // Perform the AST normalization.
    //    DefaultFunctionGenerator dfg;
    //    dfg.traverse(sageProject, preorder);
    defaultFunctionGenerator(sageProject);
    AstPostProcessing(sageProject);
    shortCircuitingTransformation(sageProject);
    destructorCallAnnotator(sageProject);

    AstPDFGeneration pdftest;
    pdftest.generateInputFiles(sageProject);
    AstDOTGeneration dottest;
    dottest.generateInputFiles(sageProject);

    return backend(sageProject);
}
