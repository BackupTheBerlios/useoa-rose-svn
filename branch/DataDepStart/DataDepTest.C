#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rose.h>
#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
#include <OpenAnalysis/ReachDefs/ManagerReachDefsStandard.hpp>
#include <OpenAnalysis/Alias/ManagerAliasMapBasic.hpp>
#include <OpenAnalysis/Alias/ManagerFIAlias.hpp>
#include <OpenAnalysis/CFG/ManagerCFGStandard.hpp>
#include <OpenAnalysis/CallGraph/ManagerCallGraphStandard.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilderDOT.hpp>
#include <OpenAnalysis/DataDep/ManagerGCD.hpp>
//#include "SageAttr.h"  // needed for findSymbolFromStmt

#include <string>
#include <iostream>

using namespace std;
using namespace OA;


int DoGCDAnalysis(SgFunctionDefinition* f, SgProject * p,
                  std::vector<SgNode*> * na, bool p_handle,
                  OA::OA_ptr<SageIRInterface> irInterface);



int main(char argc, char *argv[])
{
    SgProject * sageProject = frontend( (int)(argc-1),&argv[1]);
    int filenum = sageProject->numberOfFiles();

    AstPDFGeneration pdftest;
    pdftest.generateInputFiles(sageProject);
    AstDOTGeneration dottest;
    dottest.generateInputFiles(sageProject);

    bool p_h=FALSE; // for debugging only switch between persistent and
                    // "pointer" handles (pointers are faster, persistent are
                    // easier to debug
    std::vector<SgNode*> nodeArray;

    OA::OA_ptr<SageIRInterface> ir; 
    ir = new SageIRInterface(sageProject, &nodeArray, p_h);
    
    for (int i = 0; i < filenum; ++i) 
    {
        SgFile &sageFile = sageProject->get_file(i);
        SgGlobal *root = sageFile.get_root();
        SgDeclarationStatementPtrList& declList = root->get_declarations ();
        for (SgDeclarationStatementPtrList::iterator p = declList.begin();
             p != declList.end(); ++p) 
        {
            SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
            if (func == 0)
              continue;
            SgFunctionDefinition *defn = func->get_definition();
            if (defn == 0)
              continue;
            DoGCDAnalysis(defn, sageProject, &nodeArray, p_h, ir);
        }
    }
    
   return 1;
}
/*
bool isIdxExprAccess(OA_ptr<MemRefExpr> mre)
{
    // In order to insure that this an IdxExprAccess we must assure
    // that the MRE is:
    //  - a RefOp       and that, that RefOp is:
    //  - a SubSetRef   and that, that SubSetRef is:
    //  - an IdxExprAccess

    if(! mre->isaRefOp()) { return false; }
    OA_ptr<RefOp> refOp;
    refOp = mre.convert<RefOp>();
    if(! refOp->isaSubSetRef()) { return false; }
    OA_ptr<SubSetRef> subSetRef;
    subSetRef = refOp.convert<SubSetRef>();
    if(! subSetRef->isaIdxExprAccess()) { return false; }

    return true;
}

// Euclid's algorithm for finding the greatest common divisor between
// two integers: x and y
int gcd(int x, int y) {
    while(x > 0) {
        if(y > x) {
            // swap the values of x and y
            int tmp = x;
            x = y;
            y = tmp;
        }
        x = x - y;
    }
    return y;
}

// return true if overlap could potentially exist
bool gcdTest(OA_ptr<IdxExprAccess> a1, OA_ptr<IdxExprAccess> a2) {
    // Extract the coefficients (c) and offsets (k) from the two index
    // expression access objects.
    int c1 = a1->getCoefficient();
    int c2 = a2->getCoefficient();
    int k1 = a1->getOffset();
    int k2 = a2->getOffset();

    // GCD detects potential overlap of two array references by determining
    // whether the difference between the two offsets k1 and k2 is divisible
    // by the greatest common divisor of the two multipliers c1 and c2
    // (Wolfe86).  If the difference is not divisible then the two array
    // references could not possibly overlap
    return (k1 - k2) % gcd(c1, c2) != 0;
}
*/
int DoGCDAnalysis(SgFunctionDefinition* f, SgProject * p,
                  std::vector<SgNode*> * na, bool p_handle,
                  OA::OA_ptr<SageIRInterface> irInterface)
{
    // create manager and results objects
    OA::OA_ptr<OA::DataDep::ManagerGCD> manager;
    manager = new OA::DataDep::ManagerGCD(irInterface);
    OA::OA_ptr<OA::DataDep::GCDResults> results;
    results = new OA::DataDep::GCDResults();

    //irInterface->getMemRefExprIterator(OA::MemRefHandle h);

// THE FOLLOWING CODE WILL BE MOVED TO THE MANAGER CLASS ONCE I KNOW IT
// WORKS:
//    vector<OA_ptr<IdxExprAccess> > arrayReferences;

    // get a procedure iterator
    bool excludeInputFiles = true;
    OA::OA_ptr<SageIRProcIterator> procIter;
    procIter = new SageIRProcIterator(p, irInterface, excludeInputFiles);
    OA::ProcHandle currProc = procIter->current();

    results = manager->performGCDTest(currProc);
    results->output(irInterface);

/*
    // iterate over all statements in this procedure
    OA::ProcHandle currProc = procIter->current();
    OA::OA_ptr<OA::IRStmtIterator> stmtIter =
        irInterface->getStmtIterator(currProc);
    for(; stmtIter->isValid(); ++(*stmtIter)) {
        // get an iterator for all the memory reference expressions on
        // the current statement
        OA::StmtHandle stmt = stmtIter->current();
        OA::OA_ptr<OA::MemRefHandleIterator> memRefHandleIter =
            irInterface->getAllMemRefs(stmt);
        // iterate across all of the memory reference handles in this
        // statement
        for( ; memRefHandleIter->isValid(); ++(*memRefHandleIter)) {
            // for each mem-ref handle get an iterator for all the memory
            // references it refers to
            OA::MemRefHandle memref = memRefHandleIter->current();
            OA_ptr<OA::MemRefExprIterator> mreIter =
                irInterface->getMemRefExprIterator(memref);
            for( ; mreIter->isValid(); ++(*mreIter)) {
                OA_ptr<MemRefExpr> mre = mreIter->current();

                if(isIdxExprAccess(mre)) {
                    OA_ptr<IdxExprAccess> idxExprAccess =
                        mre.convert<IdxExprAccess>();
                    arrayReferences.push_back(idxExprAccess);
                }
            }
        }
//    OA::OA_ptr<OA::DataDep::GCDResults> cfg =
//      manager->performGCDTest();
    }

    // Iterate through all combinations of idxExprAccess
    for(int i = 0; i < arrayReferences.size(); i++) {
        for(int j = 0; j < arrayReferences.size(); j++) {
            if(i == j) continue;
            if(arrayReferences[i]->getMemRefExpr() !=
               arrayReferences[j]->getMemRefExpr()) continue;
            if(! gcdTest(arrayReferences[i], arrayReferences[j])) {
                cout << "Potential overlap detected\n";
            }
        }
    }*/
}


