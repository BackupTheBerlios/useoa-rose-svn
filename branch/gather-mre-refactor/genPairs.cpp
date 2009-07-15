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

//! print usage information to stdout
void usage(char *programName);

int main ( unsigned argc,  char * argv[] )
{
    string sInputFile, sOutputFile;
    ofstream outputFile;

    // for debugging only switch between persistentand "pointer" handles
    // (pointers are faster, persistent are easier to debug
    bool p_h=FALSE; 
    //    p_h = TRUE;

    CmdOptions *cmds = CmdOptions::GetInstance();
    cmds->SetOptions(argc, argv);
    bool useVtableOpt = true;

    // read in command line arguments
    // usage: CtoOA inputFile
    if(argc < 2) { usage(argv[0]); return 1; }
    sInputFile  = argv[1];
    //sOutputFile = argv[2];

    // load the Sage project, open the output file, and construct the
    // code generator (which outputs .oa notation to the file)
    SgProject * sageProject = frontend((int)argc, &argv[0]);
    //outputFile.open(sOutputFile.c_str());

    // Loop over every file.   BW 4/13/06
    int filenum = sageProject->numberOfFiles();
    for (int i = 0; i < filenum; ++i) 
    {

        SgFile &sageFile = sageProject->get_file(i);
        SgGlobal *root = sageFile.get_root();

        // Loop through every function in the file of this project.
        std::vector<SgNode*> nodeArray;
        OA::OA_ptr<SageIRInterface> irInterface; 
        bool excludeInputFiles = true;
        irInterface = new SageIRInterface(sageProject, &nodeArray, p_h, useVtableOpt, excludeInputFiles);
        OA::OA_ptr<SageIRProcIterator> procIter;
        procIter = new SageIRProcIterator(sageProject, *irInterface);

        for (; procIter->isValid(); ++(*procIter) ) 
	{
	  OA::OA_ptr<OA::IRStmtIterator> stmtIter = irInterface->getStmtIterator(procIter->current());
          for(; stmtIter->isValid(); ++(*stmtIter)) {
            StmtHandle stmt = stmtIter->current();

	    std::cout << "Stmt: " << irInterface->toString(stmt) << "\n";

	    std::cout << "Results from getAssignPairIterator:" << std::endl;
	    OA::OA_ptr<OA::AssignPairIterator> pairIter = 
              irInterface->getAssignPairIterator(stmt);
            for(; pairIter->isValid(); ++(*pairIter)) {
	      OA::MemRefHandle memRef = pairIter->currentTarget();
	      OA::ExprHandle expr = pairIter->currentSource();
	      std::cout << "   memRef: " << irInterface->toString(memRef)
                        << "   expr: "   << irInterface->toString(expr)
                        << std::endl;

            }

	    std::cout << "Results from getPtrAssignStmtPairIterator:" << std::endl;
	    OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator> ptrPairIter = 
              irInterface->getPtrAssignStmtPairIterator(stmt);
            for(; ptrPairIter->isValid(); ++(*ptrPairIter)) {
	      OA::OA_ptr<OA::MemRefExpr> lhs = ptrPairIter->currentTarget();
	      OA::OA_ptr<OA::MemRefExpr> rhs = ptrPairIter->currentSource();
	      std::cout << "   lhs: ";
	      lhs->output(*irInterface);
	      std::cout << "   rhs: ";
              rhs->output(*irInterface);
	      std::cout << std::endl;

            }
          }
	}
    }     

    return 0;
}

void usage(char *programName)
{
    cout << programName << ": missing operand" << endl;
    cout << "Usage: CtoOA source target" << endl;
}


