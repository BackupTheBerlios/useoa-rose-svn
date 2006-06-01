/*! \ file

  \brief Tool to convert .c files into .oa notation files.

  \authors Andy Stone
*/

#include <rose.h>
#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include <string>
#include <iostream>
#include <OpenAnalysis/Alias/NotationGenerator.hpp>

//! print usage information to stdout
void usage(char *programName);

//! output notation information into an open file
void outputNotation(OA::ProcHandle proc,
    OA::OA_ptr<SageIRInterface> sageIRInterface,
    std::ostream &outFile);




int main ( unsigned argc,  char * argv[] )
{
    string sInputFile, sOutputFile;
    ofstream outputFile;

    // for debugging only switch between persistentand "pointer" handles
    // (pointers are faster, persistent are easier to debug
    bool p_h=FALSE; 
    p_h = TRUE;

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
        irInterface = new SageIRInterface(sageProject, &nodeArray, p_h);
        OA::OA_ptr<SageIRProcIterator> procIter;
        procIter = new SageIRProcIterator(sageProject, irInterface);

        for (; procIter->isValid(); ++(*procIter) ) 
	{
            // output notation for this function
	    outputNotation(procIter->current(), irInterface, std::cout);
	}
    }     

    return 0;
}

void usage(char *programName)
{
    cout << programName << ": missing operand" << endl;
    cout << "Usage: CtoOA source target" << endl;
}

void outputNotation(OA::ProcHandle proc,
    OA::OA_ptr<SageIRInterface> sageIRInterface,
    std::ostream &outFile)
{
    NotationGenerator codeGen(sageIRInterface, outFile);
    codeGen.generate(proc);
}

