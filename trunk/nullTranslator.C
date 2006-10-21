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

int main ( unsigned argc,  char * argv[] )
{
    // load the Sage project, open the output file, and construct the
    // code generator (which outputs .oa notation to the file)
    SgProject * sageProject = frontend((int)argc, &argv[0]);

    return backend(sageProject);
}
