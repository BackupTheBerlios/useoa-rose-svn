/* // testAll.cpp

  This is a test driver that calls various OA analyses  depending on the command line option

 */
 
 /*
  {  0 , "oa-CFG",            CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-MemRefExpr",     CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-Alias",          CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasMap",       CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasTag",       CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasTagReachable", CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-CallGraph",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ICFG",           CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ICFGDep",        CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ReachDefs",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-UDDUChains",     CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-UDDUChainsXAIF", CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-MPICFG",         CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ReachConsts",    CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ExprTree",       CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-Linearity",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasMapXAIF",   CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },  */
  

#ifdef HAVE_CONFIG_H
//#include <config.h>
#endif

#define TRUE 1
#define FALSE 0

#include <rose.h>
#include <unistd.h>
#include <time.h>
#include "common.h"
#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include "debug.h"
//<AIS|ATB>#include <OpenAnalysis/Alias/ManagerAliasMapBasic.hpp>
//<AIS|ATB>#include <OpenAnalysis/Alias/ManagerFIAliasEquivSets.hpp>
//<AIS|ATB>#include <OpenAnalysis/Alias/ManagerFIAliasAliasMap.hpp>
#include <OpenAnalysis/Alias/ManagerFIAliasAliasTag.hpp>
#include <OpenAnalysis/Alias/Interface.hpp>
#include <OpenAnalysis/CallGraph/ManagerCallGraph.hpp>
#include <OpenAnalysis/CFG/ManagerCFG.hpp>
#include <OpenAnalysis/CFG/EachCFGStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
//<AIS|ATB>#include <OpenAnalysis/DataDep/ManagerDataDepGCD.hpp>
#include <OpenAnalysis/ICFG/ManagerICFG.hpp>
//#include <OpenAnalysis/Activity/ManagerICFGDep.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/ReachDefs/ManagerReachDefsStandard.hpp>
#include <OpenAnalysis/Liveness/ManagerLivenessStandard.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include <OpenAnalysis/DFAGen/Liveness/auto_ManagerLiveness.hpp>
//#include <OpenAnalysis/DFAGen/LivenessBV/auto_ManagerLivenessBV.hpp>
#include <OpenAnalysis/DFAGen/ReachingDefs/auto_ManagerReachingDefs.hpp>
#include <OpenAnalysis/DFAGen/Vary/auto_ManagerVary.hpp>
#include <OpenAnalysis/DFAGen/Useful/auto_ManagerUseful.hpp>
//<AIS|ATB>#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilderDOT.hpp>
#include <OpenAnalysis/Utils/Util.hpp>
//<AIS|ATB>#include <OpenAnalysis/ReachConsts/ManagerReachConstsStandard.hpp>
//<AIS|ATB>#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
//<AIS|ATB>#include <OpenAnalysis/XAIF/UDDUChainsXAIF.hpp>
//<AIS|ATB>#include <OpenAnalysis/XAIF/ManagerUDDUChainsXAIF.hpp>


//<AIS|ATB>#include <OpenAnalysis/XAIF/AliasMapXAIF.hpp>
//<AIS|ATB>#include <OpenAnalysis/XAIF/ManagerAliasMapXAIF.hpp>
//<AIS|ATB>#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
//<AIS|ATB>#include <OpenAnalysis/XAIF/UDDUChainsXAIF.hpp>
//<AIS|ATB>#include <OpenAnalysis/XAIF/ManagerUDDUChainsXAIF.hpp>
//<AIS|ATB>#include <OpenAnalysis/ReachConsts/ManagerICFGReachConsts.hpp>
//<AIS|ATB>#include <OpenAnalysis/Activity/ManagerActiveStandard.hpp>
//<AIS|ATB>#include <OpenAnalysis/Activity/ManagerEachActive.hpp>
//<AIS|ATB>#include <OpenAnalysis/Activity/ManagerInterDep.hpp>
//<AIS|ATB>#include <OpenAnalysis/Activity/ManagerICFGActive.hpp>
//<AIS|ATB>#include <OpenAnalysis/Activity/ManagerICFGDep.hpp>
//<AIS|ATB>#include <OpenAnalysis/Activity/ManagerICFGUseful.hpp>
//<AIS|ATB>#include <OpenAnalysis/Activity/ManagerICFGVaryActive.hpp>
//<AIS|ATB>#include <OpenAnalysis/Linearity/ManagerLinearityStandard.hpp>

//<AIS|ATB>#include <OpenAnalysis/Loop/LoopManager.hpp>

//#include "SageAttr.h"  // needed for findSymbolFromStmt

#include <string>
#include <iostream>
#include <sstream>
#include <CommandOptions.h>

#include <defaultFunctionGenerator.h>
#include <shortCircuitingTransformation.h>
#include <destructorCallAnnotator.h>

// put in to help with g++ compiler bug
/*
#include <new>
#include <stdlib.h>
void* operator new(size_t sz)
{
    void* res = malloc(sz);
    printf("new(%d) - %p\n", sz, res);
    return res;
}

void operator delete(void* ptr)
{
    printf("delete(%p)\n", ptr);
    free(ptr);
}
*/

using namespace std;
using namespace OA;
//<AIS|ATB>using namespace OA::DataDep;
using namespace OA::Alias;
//<AIS|ATB>using namespace OA::AffineExpr;

int DoOpenAnalysis(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
int DoAlias(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
//<AIS|ATB>int DoFIAliasEquivSets(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
//<AIS|ATB>int DoFIAliasAliasMap(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoFIAliasTag(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoFIAliasTagReachable(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
//<AIS|ATB>int DoFIAliasReachableAliasMap(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoCFG(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoCallGraph(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoICFG(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoICFGDep(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoParamBinding(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle);
int DoUDDUChains(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
int DoDataDepGCD(SgProject * proj, std::vector<SgNode*> * na, bool p_handle,
    char argc, char *argv[]);
int DoUDDUChainsXAIF(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
void OutputMemRefInfo(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt);
void OutputMemRefInfoNoPointers(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt);
int DoReachDef(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoLiveness(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int Foo(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoSideEffect(SgProject* sgproject, std::vector<SgNode*> *na, bool p_handle);
int DoAliasMapXAIFFIAlias(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
void prettyPrintMemRefExp(OA::OA_ptr<OA::MemRefExpr> memRefExp, 
			  SageIRInterface *ir,
			  std::ostream &os);
int DoReachConsts(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoICFGReachConsts(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*>* na, bool p_handle);
int DoICFGActivity(SgProject * p, std::vector<SgNode*>* na, bool p_handle);
int DoExprTree(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoLoop(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoLinearity(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoAssignPairs(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoCompareExpressionTrees(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoDFAGenLiveness(SgProject *p, std::vector<SgNode*> *na, bool p_handle);
//int DoDFAGenLivenessBV(SgProject *p, std::vector<SgNode*> *na, bool p_handle);
int DoDFAGenReachDefs(SgProject *p, std::vector<SgNode*> *na, bool p_handle);
int DoDFAGenVary(SgProject *p, std::vector<SgNode*> *na, bool p_handle);
int DoDFAGenUseful(SgProject *p, std::vector<SgNode*> *na, bool p_handle);


/* Debug flags:

    turn this on:      If you:
    debug               Want debugging output.
    outputRose          
    outputAST           output .dot and .pdf files of the AST
    exitWithTop         Want to run the unix 'top' program on this process
                        immediatly before it exits. Q: Why would you want to do
                        this? A: It shows how much memory the process used.
    skipAnalysis        Have UseOA-ROSE construct the IR interface and do
                        everything it normally does except actually perform
                        the analysis.
    silent              Don't output the normal output.  Notice: this doesn't
                        silence debugging output, if you want that turn off
                        all the debugging flags.  Nor does this silence error
                        messages.
*/

bool debug = false;
bool outputRose = false;
bool outputAST = false;
bool exitWithTop = false;
bool skipAnalysis = false;
bool silent = false;

void readDebuggingFlags() {
    USEOA_DEBUG_CTRL_MACRO("DEBUG",         debug);
    USEOA_DEBUG_CTRL_MACRO("OUTPUT_ROSE",   outputRose);
    USEOA_DEBUG_CTRL_MACRO("OUTPUT_AST",    outputAST);
    USEOA_DEBUG_CTRL_MACRO("EXIT_WITH_TOP", exitWithTop);
    USEOA_DEBUG_CTRL_MACRO("SKIP_ANALYSIS", skipAnalysis);
    USEOA_DEBUG_CTRL_MACRO("SILENT", silent);
}



void usage(char **argv)
{
  cerr << "usage: " << argv[0] << " [ debugFlags ] opt filename" << endl;
  cerr << "     where debugFlags is one or more of: " << endl;
  cerr << "          --debug" << endl;
  cerr << "          --outputRose" << endl;
  cerr << "          --outputAST" << endl;
  cerr << "          --exitWithTop" << endl;
  cerr << "          --skipAnalysis" << endl;
  cerr << "          --silent" << endl;
  cerr << "     where opt is one of:" << endl;
  cerr << "          --oa-AliasMap" << endl;
  cerr << "          --oa-AliasTag" << endl;
  cerr << "          --oa-AliasTagReachable" << endl;
  cerr << "          --oa-AliasMapXAIF" << endl;
  cerr << "          --oa-CFG" << endl;
  cerr << "          --oa-CallGraph" << endl;
  cerr << "          --oa-DataDepGCD" << endl;
  cerr << "          --oa-ExprTree" << endl;
  cerr << "          --oa-FIAliasAliasMap" << endl;
  cerr << "          --oa-FIAliasReachableAliasMap" << endl;
  cerr << "          --oa-FIAliasEquivSets" << endl;
  cerr << "          --oa-ICFG" << endl;
  cerr << "          --oa-ICFGActivity" << endl;
  cerr << "          --oa-ICFGDep" << endl;
  cerr << "          --oa-ICFGReachConsts" << endl;
  cerr << "          --oa-Linearity" << endl;
  cerr << "          --oa-Liveness" << endl;
  cerr << "          --oa-Loop" << endl;
  cerr << "          --oa-MPICFG" << endl;
  cerr << "          --oa-MemRefExpr" << endl;
  cerr << "          --oa-ParamBindings" << endl;
  cerr << "          --oa-ReachConsts" << endl;
  cerr << "          --oa-ReachDefs" << endl;
  cerr << "          --oa-SideEffect" << endl;
  cerr << "          --oa-UDDUChains" << endl;
  cerr << "          --oa-UDDUChainsXAIF" << endl;
  cerr << "          --oa-CompareExpressionTrees" << endl;
  cerr << "          --oa-DFAGen-Liveness" << endl;
//  cerr << "          --oa-DFAGen-LivenessBV" << endl;
  cerr << "          --oa-DFAGen-ReachDefs" << endl;
  cerr << "          --oa-DFAGen-Vary" << endl;
  cerr << "          --oa-DFAGen-Useful" << endl;
  cerr << "          --oa-DFAGen-AvailExprs" << endl;

  exit(-1);
}

// Temp
class A {
  public:
    int val;
};



int
main ( int argc,  char * argv[] )
{

  readDebuggingFlags();


  //! ====================================================
  //! for debugging only switch between persistent and 
  //! "pointer" handles (pointers are faster, persistent 
  //! are easier to debug
  //! ====================================================

  bool p_h=FALSE; 
  //bool p_h = TRUE;
  
  std::vector<SgNode*> nodeArray;

  //! ============================================================
  //! Figure out which analysis to do based on command-line args.
  //! Now we use CmdOptions and don't require the analysis option
  //! to be the first specified.
  //! ===========================================================
  if(argc<3)
  {
    usage(argv);
  }
  else
  {
    double time = clock();
    SgProject * sageProject =frontend( (int)(argc-1),&argv[1]);
    printf("Frontend Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    int filenum = sageProject->numberOfFiles();

    CmdOptions *cmds = CmdOptions::GetInstance();
#ifdef ROSE_PRE_0_9_0B
    cmds->SetOptions(argc, argv);
#else
    vector<string> argvList(argv, argv + argc);
    cmds->SetOptions(argvList);
#endif

    //! ============================================
    //! debug flags, these can alternatively be set 
    //! with the USEOA_DEBUG environmental
    //! ============================================

    // variable.
    if ( cmds->HasOption("--debug") )        { debug = true; }
    if ( cmds->HasOption("--outputRose") )   { outputRose = true; }
    if ( cmds->HasOption("--outputDOT") )    { outputAST = true; }
    if ( cmds->HasOption("--exitWithTop") )  { exitWithTop = true; }
    if ( cmds->HasOption("--skipAnalysis") ) { skipAnalysis = true; }
    if ( cmds->HasOption("--silent") )       { silent = true; }




    //! ======== output AST if the flag to do so is set. ========
    if(outputAST) {
        AstPDFGeneration pdftest;
        pdftest.generateInputFiles(sageProject);
        AstDOTGeneration dottest;
        dottest.generateInputFiles(sageProject);
    }

#if 0 
    // Perform the AST normalization.
    //    DefaultFunctionGenerator dfg;
    //    dfg.traverse(sageProject, preorder);
    defaultFunctionGenerator(sageProject);
    AstPostProcessing(sageProject);
    shortCircuitingTransformation(sageProject);
    //    destructorCallAnnotator(sageProject);
#endif
   





    //! ===================================================
    //! Analysis Drivers
    //! ===================================================

 
    if( cmds->HasOption("--oa-CFG") )
    {
       //! ======= Control Flow Graph =========
       DoCFG(sageProject, &nodeArray, p_h);

    }
    else if( cmds->HasOption("--oa-MemRefExpr") )
    {

      //! ===== MemRefExpr Analysis =====
     
      OA::OA_ptr<SageIRInterface> ir; 
      ir = new SageIRInterface(sageProject, &nodeArray, p_h);

      for (int i = 0; i < filenum; ++i) 
      {

          SgFile &sageFile = sageProject->get_file(i);
          SgSourceFile *srcFile = isSgSourceFile(&sageFile);
          if(srcFile == NULL)
            continue;
          SgGlobal *root = srcFile->get_globalScope();


          //! FIXME
          //! PLM What is the Difference of ROSE 0_9_0B from other?
    
#ifdef ROSE_PRE_0_9_0B

          //! ===== get the ROSE file =====

	  const char *fileName = sageFile.getFileName();
          ROSE_ASSERT(fileName != NULL);
          ir->createNodeArray(root);


          //! == get all the procedures in the program ==

          list<SgNode *> nodes =
              NodeQuery::querySubTree(root,
                                      V_SgFunctionDefinition);
          for (list<SgNode *>::iterator it = nodes.begin();
               it != nodes.end(); ++it )
          {
               SgNode *n = *it;
               ROSE_ASSERT(n != NULL);

               //! ===== Check if the procedure is defined =====

               SgFunctionDefinition *defn = isSgFunctionDefinition(n);
               ROSE_ASSERT(defn != NULL);

           
               //! ===== Filter Out  ? ===== 
               if (!defn->get_file_info())  { continue; }

              
               //! ===== Filter Out ? =====
               if (!ROSE::isSameName(fileName,
                                    (defn->get_file_info())->get_filename()))
               {  continue; }


               //! ===== For each Procedure =====

               OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(defn)));


               //! === Iterator over all the Statements in the Procedure ===

               OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
               for ( ; sIt->isValid(); (*sIt)++) {

                    OA::StmtHandle stmt = sIt->current();
  
                    //! === Print MemRefHandles per Statement ===

                    if ( outputRose ) {
                         OutputMemRefInfoNoPointers(ir, stmt);
                    }
                    else {
                         OutputMemRefInfo(ir, stmt);
                    }

               } //! ===== end for Statement Iter =====

          } //! ===== end for SgNode Iter =====

#else

          //! ===== get the ROSE file

	  const char *fileName = sageFile.getFileName().c_str();
          ROSE_ASSERT(fileName != NULL);
          ir->createNodeArray(root);


          //! == get all the procedures in the program ==

          NodeQuerySynthesizedAttributeType nodes =
              NodeQuery::querySubTree(root,
                                      V_SgFunctionDefinition);

          for (NodeQuerySynthesizedAttributeType::iterator it = nodes.begin();
               it != nodes.end(); ++it )
          {

               SgNode *n = *it;
               ROSE_ASSERT(n != NULL);

               //! ===== Check if the procedure is defined =====

               SgFunctionDefinition *defn = isSgFunctionDefinition(n);
               ROSE_ASSERT(defn != NULL);


               //! ===== Filter Out ? =====

               if (!defn->get_file_info())  { continue; }
               // Don't output junk we pull in from header files.


               //! ===== Filter Out ? =====

               if (strcmp(fileName, (defn->get_file_info())->get_filename()))
               { continue; }


               //! ===== For each Procedure =====

               OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(defn)));

               //! ===== Iterator over all the Statements in the Procedure =====

               OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
               for ( ; sIt->isValid(); (*sIt)++)
               {
                    OA::StmtHandle stmt = sIt->current();

                    //! === Print MemRefHandles per Statement ===       

                    if ( outputRose ) {
                         OutputMemRefInfoNoPointers(ir, stmt);
                    }
                    else {
                         OutputMemRefInfo(ir, stmt);
                    }

               } //! ===== end for Statement Iter =====

          } //! ===== end for SgNode Iter =====

#endif

      } //! ===== end for file Iter =====

    } //! ===== end MemRefExpr driver =====




    //<AIS|ATB>
    #if 0
    else if ( cmds->HasOption("--oa-ExprTree") )
    {
      for (int i = 0; i < filenum; ++i) 
        {
            SgFile &sageFile = sageProject->get_file(i);
            SgGlobal *root = sageFile.get_root();
            SgDeclarationStatementPtrList& declList = root->get_declarations ();
            for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p) 
             {
               SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
               if (func == 0){
                   continue;
	       }
               SgFunctionDefinition *defn = func->get_definition();
               if (defn == 0){     
                 continue;
	       }
               DoExprTree(defn, sageProject, &nodeArray, p_h);
             }     
	 } 
    }
    #endif

    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-AssignPairs") ) {
      for (int i = 0; i < filenum; ++i)
      {
        SgFile &sageFile = sageProject->get_file(i);
        SgGlobal *root = sageFile.get_root();
        SgDeclarationStatementPtrList& declList = root->get_declarations ();
        for (SgDeclarationStatementPtrList::iterator p = declList.begin();         p != declList.end(); ++p)
        {
            SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
            if (func == 0){
                continue;
            }
            SgFunctionDefinition *defn = func->get_definition();
            if (defn == 0){
                continue;
            }

            DoAssignPairs(defn, sageProject, &nodeArray, p_h);
        }
      }
    }
    #endif


    //<AIS|ATB>
    #if 0
    else if( ( cmds->HasOption("--oa-FIAliasEquivSets") ) ||
	     ( cmds->HasOption("--oa-FIAlias") ) )
    {
      DoFIAliasEquivSets(sageProject, &nodeArray, p_h);
      //      DoFIAliasAliasMap(sageProject, &nodeArray, p_h);
      //      DoFIAlias(sageProject, &nodeArray, FALSE);
    }
    #endif
    else if( cmds->HasOption("--oa-AliasTagReachable") )
    {
      DoFIAliasTagReachable(sageProject, &nodeArray, p_h);
    }
    else if( cmds->HasOption("--oa-AliasTag") )
    {
      DoFIAliasTag(sageProject, &nodeArray, p_h);
    }
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-FIAliasAliasMap") )
    {
      DoFIAliasAliasMap(sageProject, &nodeArray, p_h);
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-FIAliasReachableAliasMap") )
    {
      DoFIAliasReachableAliasMap(sageProject, &nodeArray, p_h);
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-AliasMap") )
    {
      //printf("TO DO, implement alias analysis\n");
        for (int i = 0; i < filenum; ++i) 
          {
            SgFile &sageFile = sageProject->get_file(i);
            SgGlobal *root = sageFile.get_root();
            SgDeclarationStatementPtrList& declList = root->get_declarations ();
            for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p) 
              {
                SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
                if (func == 0)
                  continue;
                SgFunctionDefinition *defn = func->get_definition();
                if (defn == 0)
                  continue;
                // SgBasicBlock *stmts = defn->get_body();  
                // create a control flow graph and generate text output
                DoAlias(defn, sageProject, &nodeArray, p_h);
              }     
          }
    }
    #endif
    /*else if( cmds->HasOption("--oa-AliasMap") )
    {
      printf("TO DO, implement alias map analysis\n");
      return 1;
    }*/
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-CallGraph") )
    {
       DoCallGraph(sageProject, &nodeArray, p_h);
      return 1;
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-ICFG") )
    {
       DoICFG(sageProject, &nodeArray, p_h);
      return 1;
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-ICFGDep") )
    {
       DoICFGDep(sageProject, &nodeArray, p_h);
      return 1;
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-ICFGActivity") )
    {
       DoICFGActivity(sageProject, &nodeArray, p_h);
      return 1;
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-ParamBindings") )
    {
      DoParamBinding(sageProject, &nodeArray, p_h);
       return 1;
    }
    #endif
    else if( cmds->HasOption("--oa-SideEffect") )
    {
      DoSideEffect(sageProject, &nodeArray, p_h);
      return 1;
    }
    #if 0
    //<AIS|ATB>
    else if( cmds->HasOption("--oa-ReachDefs") )
    {
       DoReachDef(sageProject, &nodeArray, p_h);
    }
    else if( cmds->HasOption("--oa-UDDUChains") )
    {
      for (int i = 0; i < filenum; ++i) 
      {
        SgFile &sageFile = sageProject->get_file(i);
        SgGlobal *root = sageFile.get_root();
        SgDeclarationStatementPtrList& declList = root->get_declarations ();
        for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p) 
        {
          SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
          if (func == 0)
            continue;
          SgFunctionDefinition *defn = func->get_definition();
          if (defn == 0)
            continue;
          // SgBasicBlock *stmts = defn->get_body();  
          // create a control flow graph and generate text output
          DoUDDUChains(defn, sageProject, &nodeArray, p_h);
        }     
      }
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-UDDUChainsXAIF") )
    {
      for (int i = 0; i < filenum; ++i)
      {
        SgFile &sageFile = sageProject->get_file(i);         SgGlobal *root = sageFile.get_root();
        SgDeclarationStatementPtrList& declList = root->get_declarations ();
        for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p)
        {
          SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
          if (func == 0)
            continue;
          SgFunctionDefinition *defn = func->get_definition();
          if (defn == 0)
            continue;
          // SgBasicBlock *stmts = defn->get_body();
          // create a control flow graph and generate text output
          DoUDDUChainsXAIF(defn, sageProject, &nodeArray, p_h);
        }
      }

    }
    #endif
    else if( cmds->HasOption("--oa-MPICFG") )
    {
      printf("TO DO, implement MPICFG analysis\n");
      return 1;
    }
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-ReachConsts") )
    {
        
    //  printf("TO DO, implement ReachConsts analysis\n");
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
                                                                                             DoReachConsts(defn, sageProject, &nodeArray, p_h);
                                                                                           }
                                                                                        }
     
        return 1;
    } 
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-ICFGReachConsts") )
    {
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
             DoICFGReachConsts(defn, sageProject, &nodeArray, p_h);
           }
        }

        return 1;
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-AliasMapXAIF") )
    {
      for (int i = 0; i < filenum; ++i)
        {
            SgFile &sageFile = sageProject->get_file(i);
            SgGlobal *root = sageFile.get_root();
            SgDeclarationStatementPtrList& declList = root->get_declarations ();            for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p)              {
               SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
               if (func == 0){
                   continue;
           }
               SgFunctionDefinition *defn = func->get_definition();
               if (defn == 0){
                 continue;
           }
               // 10/26/06, This isn't defined in the in the merged OATest.C
               // Look for it in the trunk version before r239.
               //DoAliasMapXAIFFIAlias(defn, sageProject, &nodeArray, p_h);
             }
     }

    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-Linearity") )
    {
      printf("Linearity Analysis Start:\n");
      for (int i = 0; i < filenum; ++i)
        {
            SgFile &sageFile = sageProject->get_file(i);
            SgGlobal *root = sageFile.get_root();
            SgDeclarationStatementPtrList& declList = root->get_declarations ();
            for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p)
             {
               SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
               if (func == 0){
                   continue;
               }
               SgFunctionDefinition *defn = func->get_definition();
               if (defn == 0){
                 continue;
               }
               DoLinearity(defn, sageProject, &nodeArray, p_h);
             }
         }
      return 1;
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-Loop") )
    {
        printf("Loop Analysis Start:\n");
        DoLoop(sageProject, &nodeArray, p_h);
        return 1;
    }
    #endif

    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-DataDepGCD") )
    {
      DoDataDepGCD(sageProject, &nodeArray, p_h, argc, argv);
      return 1;
    }
    #endif
    //<AIS|ATB>
    #if 0
    else if( cmds->HasOption("--oa-CompareExpressionTrees") )
    {
        // iterate through files than functions, perform the analysis on each
        // function
        for (int i = 0; i < filenum; ++i) 
        {
            SgFile &sageFile = sageProject->get_file(i);
            SgGlobal *root = sageFile.get_root();
            SgDeclarationStatementPtrList& declList = root->get_declarations();
            for(SgDeclarationStatementPtrList::iterator p = declList.begin();
                p != declList.end(); ++p) 
            {
               SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
               if (func == 0){
                   continue;
	       }
               SgFunctionDefinition *defn = func->get_definition();
               if (defn == 0){     
                 continue;
	       }
               DoCompareExpressionTrees(defn, sageProject, &nodeArray, p_h);
            }     
	 } 


        return 1;
    }
    #endif
    else if( cmds->HasOption("--oa-Liveness") )
    {
        return DoLiveness(sageProject, &nodeArray, p_h);
    }
    else if( cmds->HasOption("--oa-DFAGen-Liveness") )
    {
        return DoDFAGenLiveness(sageProject, &nodeArray, p_h);
    }
/*    else if( cmds->HasOption("--oa-DFAGen-LivenessBV") )
    {
        return DoDFAGenLivenessBV(sageProject, &nodeArray, p_h);
    }*/
    else if( cmds->HasOption("--oa-DFAGen-ReachDefs") )
    {
        return DoDFAGenReachDefs(sageProject, &nodeArray, p_h);
    }
    else if( cmds->HasOption("--oa-DFAGen-Vary") )
    {
        return DoDFAGenVary(sageProject, &nodeArray, p_h);
    }
    else if( cmds->HasOption("--oa-DFAGen-Useful") )
    {
        return DoDFAGenUseful(sageProject, &nodeArray, p_h);
    }
    else if( cmds->HasOption("--oa-DFAGen-AvailExprs") )
    {
        // TODO: Implement
        assert(false);
    }
    else if(skipAnalysis == false)
    {
      printf("did not find any valid oa option on the command line\n");
      return 1;
    }
  }

  // if the exitWithTop debug flag was set then use top to display the memory
  // usage of this process immediatly prior to termination
  if(exitWithTop) {
    ostringstream sCmd;
    sCmd << "top p " << getpid();
    system(sCmd.str().c_str());
  }

   return 0;
}









//! ===============================================================
//! Driver for Control Flow Graph
//! ===============================================================

int DoCFG(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
    printf("===== start of CFG =====\n");
    
    //! ====== IRInterface ======= 
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle); 


    //! ====== Iterator over all the procedures =====
    OA::OA_ptr<SageIRProcIterator> procIter;
    procIter = new SageIRProcIterator(p, *irInterface);


    //! ====== Create Control Flow Graph Manager ======
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);


    //! ===== Get the CFG per procedure using CFG Manager  =====

    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);


    //! PLM : I think CFG Manager and EachCFG are duplicated
    //!       functionalities but according to Michelle the
    //!       we must keep them seperate because in order to
    //!       preserve the idea.

    for(; procIter->isValid(); ++(*procIter)) {

        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());

        if(!silent) {
            
            //! Text Output
            cfg->output(*irInterface);

           //! dot output
           OA::OA_ptr<OA::OutputBuilder> outBuild;
           outBuild = new OA::OutputBuilderDOT;
           cfg->configOutput(outBuild);
           cfg->output(*irInterface);

        }
    }

    printf("Terminated succesfully\n");
    return true; 
}





//! ======================================================================
//! Belong to MemRefExpr Driver
//! ======================================================================

void OutputMemRefInfo(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt)
{
    //! ========================================================
    //! adapted from Nathan's code --should be specific IR
    //! independent only OA interface used.
    //!
    //! - For each MemRefHandle in the statement
    //!    - Get MemRefExprs associated with the MemRefHandle
    //!      - Output each MemRefExpr
    //! ========================================================


    std::cout << "======================================" << std::endl;
    std::cout << "OA::StmtHandle: " << ir->toString(stmt) << std::endl;



    //! ===== Iterate over all MemRefHandles in the Statement =====

    OA::OA_ptr<OA::MemRefHandleIterator> mrIt = ir->getAllMemRefs(stmt);
    for ( ; mrIt->isValid(); (*mrIt)++)
    {

         //! === MemRefHandle ===
         OA::MemRefHandle memref = mrIt->current();
         if ( debug ) {
              std::cout << "  ===== OA::MemRefHandle: ===== ";
              fflush(stdout);
              ir->dump(memref, std::cout);
         }


         //! === Iterate over all MemRefExprs associated with MemRfHandle ===

         OA::OA_ptr<OA::MemRefExprIterator> mreIterPtr;
         mreIterPtr = ir->getMemRefExprIterator(memref);

         for (; mreIterPtr->isValid(); (*mreIterPtr)++) {

              //! === MemRefExpr ===
              OA::OA_ptr<OA::MemRefExpr::MemRefExpr> mre;
              mre = mreIterPtr->current();

              //! ===== Output the MemRefExpr
              std::cout << "\tmre = ";
              mre->output(*ir);
              std::cout << std::endl;

         } //! === end for ===

    } //! ===  end for ===

} //! === end OutputMemRefInfo ===







//! ======================================================================
//! Belong to MemRefExpr Driver
//! ======================================================================


void OutputMemRefInfoNoPointers(OA::OA_ptr<SageIRInterface> ir,
                                OA::StmtHandle stmt)
{

     //! FIXME:
     //! PLM, May 12th 2008
     //! I am not sure what this function is expected to do
     //! Need to talk to Michelle, Andy or Brian.

     std::cout << "======================================" << std::endl;
     std::cout << "OA::StmtHandle: " << ir->toString(stmt) << std::endl;

     //! =================================================================
     //! Get all mem ref handles corresponding to USEs in this statement.
     //! =================================================================

     OA::OA_ptr<OA::MemRefHandleIterator> useIter;
     useIter = ir->getUseMemRefs(stmt);

     for ( ; useIter->isValid(); (*useIter)++ ) {

          OA::MemRefHandle memRefHandle = useIter->current();

          //! =======================================================
          //! Finally, get all of the mem ref expressions associated
          //! with this memRefHandle.
          //! =======================================================
           OA::OA_ptr<OA::MemRefExprIterator> memRefExprIterator;
           memRefExprIterator = ir->getMemRefExprIterator(memRefHandle);

     }

     std::cout << std::endl;

     std::cout << "DEFs within statement: " << ir->toString(stmt) << std::endl;

     //! ================================================================
     //! Get all mem ref handles corresponding to DEFs in this statement.
     //! ================================================================
     OA::OA_ptr<OA::MemRefHandleIterator> defIter;
     defIter = ir->getDefMemRefs(stmt);

     for ( ; defIter->isValid(); (*defIter)++ ) {

          OA::MemRefHandle memRefHandle = defIter->current();

         //! =========================================================
         //! Finally, get all of the mem ref expressions associated
         //! with this memRefHandle.
         //! =========================================================
         OA::OA_ptr<OA::MemRefExprIterator> memRefExprIterator;
         memRefExprIterator = ir->getMemRefExprIterator(memRefHandle);

     }

     std::cout << std::endl;

}



















//<AIS|ATB>
#if 0
int DoCallGraph(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
{
	int returnvalue=FALSE;
        if ( debug ) 
           printf("*******start of DoCallGraph\n");
        OA::OA_ptr<SageIRInterface> irInterface;
        irInterface = new SageIRInterface(sgproject, na, p_handle);
  //irInterface->createNodeArray(sgproject); //what about global vars?
  
    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
    fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
    OA::OA_ptr<SageIRProcIterator> procIter;
    procIter = new SageIRProcIterator(sgproject, 
                                      *irInterface);
    OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
    interAlias = fialiasman->performAnalysis(procIter);

    // create CallGraph Manager and then Call Graph
    OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> callgraphmanstd;
    callgraphmanstd= new OA::CallGraph::ManagerCallGraphStandard(irInterface);
//    OA::OA_ptr<SageIRProcIterator> procIter;
//    procIter = new SageIRProcIterator(sgproject, irInterface);

    OA::OA_ptr<OA::CallGraph::CallGraph> callgraph
      = callgraphmanstd->performAnalysis(procIter,interAlias);
    callgraph->output(*irInterface);
    // dot output
    OA::OA_ptr<OA::OutputBuilder> outBuild;
    outBuild = new OA::OutputBuilderDOT;
    callgraph->configOutput(outBuild);
    callgraph->output(*irInterface);
	
	std::cout << "\n*******  end of DoCallGraph *********\n\n";
	return returnvalue;

}
#endif

//<AIS|ATB>
#if 0
int DoICFG(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) 
    printf("*******start of DoICFG\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(sgproject, na, p_handle);
  //irInterface->createNodeArray(sgproject); //what about global vars?
  
  // eachCFG 
  OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
  OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
  cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
  eachCFG = new OA::CFG::EachCFGStandard(cfgman);

  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(sgproject, 
                                    *irInterface);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);
  
  // create CallGraph Manager and then Call Graph
  OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> callgraphmanstd;
  callgraphmanstd= new OA::CallGraph::ManagerCallGraphStandard(irInterface);
  //    OA::OA_ptr<SageIRProcIterator> procIter;
  //    procIter = new SageIRProcIterator(sgproject, irInterface);
  
  OA::OA_ptr<OA::CallGraph::CallGraph> callgraph;
  callgraph = callgraphmanstd->performAnalysis(procIter,interAlias);
  
  // create ICFG Manager and then ICFG
    OA::OA_ptr<OA::ICFG::ICFG> icfg;
    icfg = new OA::ICFG::ICFG();
    OA::OA_ptr<OA::ICFG::ManagerICFGStandard> icfgman;
    icfgman = new OA::ICFG::ManagerICFGStandard(irInterface);
    icfg = icfgman->performAnalysis(procIter,eachCFG,callgraph);
  
  // output ICFG 
  icfg->output(*irInterface);
  // dot output
  OA::OA_ptr<OA::OutputBuilder> outBuild;
  outBuild = new OA::OutputBuilderDOT;
  icfg->configOutput(outBuild);
  icfg->output(*irInterface);

	std::cout << "\n*******  end of DoICFG *********\n\n";
	return returnvalue;

}
#endif

//<AIS|ATB>
#if 0
int DoICFGDep(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
{

   int returnvalue=FALSE;
   if ( debug ) printf("*******start of DoUDDUChains\n");
   OA::OA_ptr<SageIRInterface> irInterface;
   irInterface = new SageIRInterface(sgproject, na, p_handle);
 

   // eachCFG
   OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
   OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
   cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
   eachCFG = new OA::CFG::EachCFGStandard(cfgman);


   //FIAlias
   OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
   fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
   OA::OA_ptr<SageIRProcIterator> procIter;
   procIter = new SageIRProcIterator(sgproject, *irInterface);
   OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
   interAlias = fialiasman->performAnalysis(procIter);


   // CallGraph
   OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> cgraphman;
   cgraphman = new OA::CallGraph::ManagerCallGraphStandard(irInterface);
   OA::OA_ptr<OA::CallGraph::CallGraph> cgraph =
      cgraphman->performAnalysis(procIter, interAlias);

   // ParamBindings
   OA::OA_ptr<OA::DataFlow::ManagerParamBindings> parambindman;
   parambindman = new OA::DataFlow::ManagerParamBindings(irInterface);
   OA::OA_ptr<OA::DataFlow::ParamBindings> parambind
      = parambindman->performAnalysis(cgraph);

  // ICFG
   OA::OA_ptr<OA::ICFG::ManagerICFGStandard> icfgman;
   icfgman = new OA::ICFG::ManagerICFGStandard(irInterface);
   OA::OA_ptr<OA::ICFG::ICFG> icfg;
   icfg = icfgman->performAnalysis(procIter,eachCFG,cgraph);

  //ICFGDep
  OA::OA_ptr<OA::Activity::ManagerICFGDep> icfgdepman;
  icfgdepman = new OA::Activity::ManagerICFGDep(irInterface);
  OA::OA_ptr<OA::Activity::ICFGDep> icfgDep;
  icfgDep = icfgdepman->performAnalysis(icfg, parambind, interAlias,
                                        OA::DataFlow::ITERATIVE);

  icfgDep->output(*irInterface);

  std::cout << "\n*******  end of DoICFGDep *********\n\n";
  return returnvalue;




    
  // text output
  //OA::OA_ptr<OA::OutputBuilder> outBuild;
  
  /*
    outBuild = new OA::OutputBuilderText;
    icfg->configOutput(outBuild);
    icfg->output(*irInterface);
    
    outBuild = new OA::OutputBuilderDOT;
    icfg->configOutput(outBuild);
    icfg->output(*irInterface);
    
    outBuild = new OA::OutputBuilderText;
    icfgDep->configOutput(outBuild);
  */
 
}
#endif

int DoSideEffect(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
{

  int returnvalue=FALSE;
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(sgproject, na, p_handle);
  
  OA::OA_ptr<SageIRProcIterator> procIter;

  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
  fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
  procIter = new SageIRProcIterator(sgproject, *irInterface);
  OA::OA_ptr<OA::Alias::Interface> alias;
  alias = fialiasman->performAnalysis(procIter);
  OA::OA_ptr<OA::Alias::InterAliasResults> interAlias;
  interAlias = new InterAliasResults(alias);

  // call graph
  OA_ptr<CallGraph::ManagerCallGraphStandard> callgraphmanstd;
  callgraphmanstd = new CallGraph::ManagerCallGraphStandard(irInterface);
  OA_ptr<CallGraph::CallGraph> callgraph =
    callgraphmanstd->performAnalysis(procIter, alias);
 
  // eachCFG
  OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
  OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
  cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
  eachCFG = new OA::CFG::EachCFGStandard(cfgman);

  // param bindings
  OA_ptr<DataFlow::ManagerParamBindings> pbman;
  pbman = new DataFlow::ManagerParamBindings(irInterface);
  OA_ptr<DataFlow::ParamBindings> parambind;
  parambind = pbman->performAnalysis(callgraph);

  // Intra Side-Effect
  OA_ptr<SideEffect::ManagerSideEffectStandard> sideeffectman;
  sideeffectman = new SideEffect::ManagerSideEffectStandard(irInterface);  

  // InterSideEffect
  OA_ptr<SideEffect::ManagerInterSideEffectStandard> interSEman;
  interSEman = new SideEffect::ManagerInterSideEffectStandard(irInterface);

  OA_ptr<OA::SideEffect::InterSideEffectStandard> interSE;
  interSE = interSEman->performAnalysis(
    callgraph, parambind, interAlias, sideeffectman, OA::DataFlow::ITERATIVE);
    
  if(!silent) { interSE->output(*irInterface, *alias); }

  return returnvalue;
}

//<AIS|ATB>
#if 0
int DoParamBinding(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
{

	  int returnvalue=FALSE;
	 if ( debug )
        {
	      printf("*******start of ParamBinding \n");
         }
      OA::OA_ptr<SageIRInterface> irInterface;
      irInterface = new SageIRInterface(sgproject, na, p_handle);
   
      //FIAlias
        OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
        fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
        OA::OA_ptr<SageIRProcIterator> procIter;
        procIter = new SageIRProcIterator(sgproject,*irInterface);
        OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
        interAlias = fialiasman->performAnalysis(procIter);
		  
		  
        OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> callgraphmanstd;
        callgraphmanstd= new OA::CallGraph::ManagerCallGraphStandard(irInterface);
        OA::OA_ptr<OA::CallGraph::CallGraph> callgraph
	                          = callgraphmanstd->performAnalysis(procIter,interAlias);
		  
        OA::OA_ptr<OA::DataFlow::ManagerParamBindings> pbman;
        pbman = new OA::DataFlow::ManagerParamBindings(irInterface);
        OA::OA_ptr<OA::DataFlow::ParamBindings> parambind;
        parambind = pbman->performAnalysis(callgraph);
        parambind->output(*irInterface);

	
        return returnvalue;
 }
#endif
		                                        


//<AIS|ATB>
#if 0
int DoAlias(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
	int returnvalue=FALSE;
        if ( debug )
	   printf("*******start of DoCallGraph\n");
        OA::OA_ptr<SageIRInterface> irInterface;
        irInterface = new SageIRInterface(p, na, p_handle);

	//try
	//{
        // create CallGraph Manager and then Call Graph
        OA::OA_ptr<OA::Alias::ManagerAliasMapBasic> aliasmanstd;
        aliasmanstd= new OA::Alias::ManagerAliasMapBasic(irInterface);
        //OA::OA_ptr<SageIRProcIterator> procIter;
        //procIter = new SageIRProcIterator(sgproject, irInterface);

        //OA::OA_ptr<OA::CallGraph::CallGraphStandard> callgraph
         // = callgraphmanstd->performAnalysis(procIter);
        
        //callgraph->dump(std::cout, irInterface);
        OA::OA_ptr<OA::Alias::AliasMap> alias = 
      aliasmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
      alias->dump(std::cout, irInterface);

	//}
	//catch(Exception &e)
	//{
		printf("error in try\n");
		
//	}


	
	std::cout << "\n*******  end of DoAlias *********\n\n";
	return returnvalue;

}
#endif

//<AIS|ATB>
#if 0
int DoFIAliasEquivSets(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;

  /*! commented out by PLM 08/17/06
  if ( debug ) printf("*******start of FIAlias\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);
  
  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasEquivSets> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasEquivSets(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, irInterface);
  //#define BRIAN_ADDED_DEBUG_PARAM_TO_PERFORMANALYSIS
#ifdef BRIAN_ADDED_DEBUG_PARAM_TO_PERFORMANALYSIS
  OA::OA_ptr<OA::Alias::EquivSets> alias = 
    fialiasman->performAnalysis(procIter, debug);
#else
  OA::OA_ptr<OA::Alias::EquivSets> alias = 
    fialiasman->performAnalysis(procIter);
#endif
  //  alias->dump(std::cout, irInterface);
  alias->output(*irInterface);
  */
}
#endif

//<AIS|ATB>
#if 0
int DoFIAliasAliasMap(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) printf("*******start of FIAlias\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  //std::vector<SgNode*> nodeArray;
  //bool p_h=FALSE; //for debugging only switch between persistent and "pointer" handles (pointers are faster, persistent are easier to debug
  irInterface = new SageIRInterface(p, na, p_handle); 
  //irInterface = new SageIRInterface(p, &nodeArray, p_h); 
  
#if 0
  list<SgNode *> nodes = NodeQuery::querySubTree(p,
						 V_SgVariableDeclaration);
  for (list<SgNode *>::iterator it = nodes.begin();
       it != nodes.end(); ++it ) {
    
    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);
    
    SgVariableDeclaration *var = isSgVariableDeclaration(n);
    ROSE_ASSERT(var != NULL);

    cout << var->unparseToCompleteString() << endl;
  }
#endif

  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, *irInterface);
  
  //#define BRIAN_ADDED_DEBUG_PARAM_TO_PERFORMANALYSIS
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  
  if(!skipAnalysis) {
    interAlias = fialiasman->performAnalysis(procIter);
    if(!silent) { interAlias->output(*irInterface); }
  }
}
#endif

int DoFIAliasTag(SgProject * p, std::vector<SgNode*> * na, bool p_handle) {
  int returnvalue = FALSE;
  if ( debug ) printf("*******start of FIAlias\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle); 

  //FIAlias
  double time = clock();
 
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
  fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, *irInterface);
  
  OA::OA_ptr<OA::Alias::Interface> results;
  results = fialiasman->performAnalysis(procIter);

  printf("FIAliasTag Time: %lf\n",
    (clock() - time) / (1.0 * CLOCKS_PER_SEC));
 
  if(!skipAnalysis) {
    if(!silent) { results->output(*irInterface); }
  }
}

/** Find main within a program. */
SgFunctionDeclaration *findMain(SgProject *project)
{
    ROSE_ASSERT(project != NULL);
#ifdef PRE_ROSE_0_9_0B
    std::list<SgNode *> nodes = 
#else
    NodeQuerySynthesizedAttributeType nodes =
#endif
        NodeQuery::querySubTree(project,
                                V_SgFunctionDeclaration);

    SgFunctionDeclaration *mainFunc = NULL;
#ifdef PRE_ROSE_0_9_0B
    for (std::list<SgNode *>::iterator it = nodes.begin();
         it != nodes.end(); ++it ) 
#else
    for (NodeQuerySynthesizedAttributeType::iterator it = nodes.begin();
         it != nodes.end(); ++it ) 
#endif
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

int DoFIAliasTagReachable(SgProject * p, std::vector<SgNode*> * na, bool p_handle) {
  int returnvalue = FALSE;
  if ( debug ) printf("*******start of FIAlias\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle); 

  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
  fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(findMain(p), *irInterface);
  
  OA::OA_ptr<OA::Alias::Interface> results;
  
  if(!skipAnalysis) {
    OA::Alias::FIAliasImplement implement = OA::Alias::REACHABLE_PROCS;
    results = fialiasman->performAnalysis(procIter, implement);
    if(!silent) { results->output(*irInterface); }
  }
}

//<AIS|ATB>
#if 0
int DoFIAliasReachableAliasMap(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) printf("*******start of FIAlias\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  //std::vector<SgNode*> nodeArray;
  //bool p_h=FALSE; //for debugging only switch between persistent and "pointer" handles (pointers are faster, persistent are easier to debug
  irInterface = new SageIRInterface(p, na, p_handle); 
  //irInterface = new SageIRInterface(p, &nodeArray, p_h); 
  
#if 0
  list<SgNode *> nodes = NodeQuery::querySubTree(p,
						 V_SgVariableDeclaration);
  for (list<SgNode *>::iterator it = nodes.begin();
       it != nodes.end(); ++it ) {
    
    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);
    
    SgVariableDeclaration *var = isSgVariableDeclaration(n);
    ROSE_ASSERT(var != NULL);

    cout << var->unparseToCompleteString() << endl;
  }
#endif

  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(findMain(p), *irInterface);
  
  //#define BRIAN_ADDED_DEBUG_PARAM_TO_PERFORMANALYSIS
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  
  if(!skipAnalysis) {
    OA::Alias::FIAliasImplement implement = OA::Alias::REACHABLE_PROCS;
    interAlias = fialiasman->performAnalysis(procIter, implement);
    if(!silent) { interAlias->output(*irInterface); }
  }
}
#endif

int DoReachDef(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
    int returnvalue=FALSE;
    double time;

    if ( debug ) printf("*******start of DoReachDef\n");

    // contruct IR
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);

    // CFG
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
    cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

    // eachCFG
    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);

    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
    fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
    OA::OA_ptr<SageIRProcIterator> procIter;
    procIter = new SageIRProcIterator(p, *irInterface);
    OA::OA_ptr<OA::Alias::Interface> alias;
    alias = fialiasman->performAnalysis(procIter);

    if(!silent) { alias->output(*irInterface); }

    // force a CFG analysis on all procedures and time it
    procIter = new SageIRProcIterator(p, *irInterface);
    int count = 0;
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());
        count++;
    }

    //TODO: Need to create interprocedural alias to pass to side effects
    OA::OA_ptr<OA::Alias::InterAliasInterface> interAlias;

    // Interprocedural Side-Effect Analysis
    // for now generate default conservative interprocedural side-effect results
    OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
    interSideEffect = new OA::SideEffect::InterSideEffectStandard(interAlias);
    
    // Construct ReachDefs manager
    OA::OA_ptr<OA::ReachDefs::ManagerReachDefsStandard> rdman;
    rdman = new OA::ReachDefs::ManagerReachDefsStandard(irInterface);

    // Iterate through each proc doing ReachDefs
    time = clock();
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::ReachDefs::ReachDefsStandard> rds;
        rds = rdman->performAnalysis(
            procIter->current(),
            eachCFG->getCFGResults(procIter->current()),
            alias,
            interSideEffect,
            OA::DataFlow::ITERATIVE); 
        
        if(!silent) { rds->output(*irInterface); }
    }

    printf("Reachdef Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    std::cout << "\n*******  end of DoReachDef *********\n\n";

    return returnvalue;
}

int DoLiveness(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
    int returnvalue=FALSE;
    OA::OA_ptr<SageIRProcIterator> procIter;

    if ( debug ) printf("*******start of hand written Liveness analysis\n");

    double time = clock();

    // contruct IR
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);
    printf("Construct IR Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    // CFG
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
    cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

    // eachCFG
    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);

    // force a CFG analysis on all procedures and time it
    time = clock();
    procIter = new SageIRProcIterator(p, *irInterface);
    int count = 0;
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());
        count++;
    }
    printf("CFG Time: %lf, %d procs analyzed\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC), count);

    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
    fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
    procIter = new SageIRProcIterator(p, *irInterface);
    OA::OA_ptr<OA::Alias::Interface> alias;
    alias = fialiasman->performAnalysis(procIter);
    printf("FIAlias Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    //TODO: Need to create interprocedural alias to pass to side effects
    OA::OA_ptr<OA::Alias::InterAliasInterface> interAlias;

    // Interprocedural Side-Effect Analysis
    // for now generate default conservative interprocedural side-effect results
    OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
    interSideEffect = new OA::SideEffect::InterSideEffectStandard(interAlias);
    printf("Side-effects Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();
    
    // Construct Liveness manager
    OA::OA_ptr<OA::Liveness::ManagerLivenessStandard> liveMgr;
    liveMgr = new OA::Liveness::ManagerLivenessStandard(irInterface);

    // Iterate through each proc doing Liveness
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::Liveness::LivenessStandard> results;
        results = liveMgr->performAnalysis(
            procIter->current(),
            eachCFG->getCFGResults(procIter->current()),
            alias,
            interSideEffect,
            OA::DataFlow::ITERATIVE);

        //if(!silent) { results->output(*irInterface); }
        //results->output(*irInterface);
    }
    printf("Liveness Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    std::cout << "\n*******end of hand written Liveness analysis *********\n\n";

    return returnvalue;
}



//<AIS|ATB>
#if 0
int DoReachConsts(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
   int returnvalue=FALSE;
   if ( debug ) printf("*******start of DoUDDUChains\n");
   OA::OA_ptr<SageIRInterface> irInterface;
   irInterface = new SageIRInterface(p, na, p_handle);
           
    
   //CFG
   OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
   cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);
   OA::OA_ptr<OA::CFG::CFG> cfg=
   cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));

   //Alias
   OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
   fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
   OA::OA_ptr<SageIRProcIterator> procIter;
   procIter = new SageIRProcIterator(p,*irInterface);
   OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
   interAlias = fialiasman->performAnalysis(procIter);
   OA::ProcHandle proc((OA::irhandle_t)(irInterface->getNodeNumber(f)));
   OA::OA_ptr<OA::Alias::Interface> alias = interAlias->getAliasResults(proc);
  
   // CallGraph
  OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> cgraphman;
  cgraphman = new OA::CallGraph::ManagerCallGraphStandard(irInterface);
  OA::OA_ptr<OA::CallGraph::CallGraph> cgraph =
      cgraphman->performAnalysis(procIter, interAlias);
   
   // ParamBindings
   OA::OA_ptr<OA::DataFlow::ManagerParamBindings> parambindman;
   parambindman = new OA::DataFlow::ManagerParamBindings(irInterface);
   OA::OA_ptr<OA::DataFlow::ParamBindings> parambind
      = parambindman->performAnalysis(cgraph);


   // Intra Side-Effect
   OA::OA_ptr<OA::SideEffect::ManagerSideEffectStandard> sideeffectman;
   sideeffectman = new OA::SideEffect::ManagerSideEffectStandard(irInterface);

   // InterSideEffect
   OA::OA_ptr<OA::SideEffect::ManagerInterSideEffectStandard> interSEman;
   interSEman = new OA::SideEffect::ManagerInterSideEffectStandard(irInterface);

   OA::OA_ptr<OA::SideEffect::InterSideEffectStandard> interSE;
   interSE = interSEman->performAnalysis(cgraph, parambind,
                                        interAlias, sideeffectman,
                                        OA::DataFlow::ITERATIVE);
  /*

   // Interprocedural Side-Effect Analysis
   // for now generate default conservative interprocedural side-effect results
   OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
   interSideEffect = new OA::SideEffect::InterSideEffectStandard;
  */       
   
   //ReachConsts        
   OA::OA_ptr<OA::ReachConsts::ManagerReachConstsStandard> rcman;
   rcman = new OA::ReachConsts::ManagerReachConstsStandard(irInterface);
   OA::OA_ptr<OA::ReachConsts::ReachConstsStandard> reachConsts=
          rcman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),
                                 cfg,alias,interSE,
                                 OA::DataFlow::ITERATIVE); 
 
   reachConsts->output(*irInterface); 
   std::cout << "\n*******  end of DoUDDUChains *********\n\n";
   return returnvalue;
}
#endif

//<AIS|ATB>
#if 0
int DoICFGActivity(SgProject * p, std::vector<SgNode*>* na, bool p_handle)
{
   int returnvalue=FALSE;
   if ( debug ) printf("******* start of DoICFGActivity *******\n");
   OA::OA_ptr<SageIRInterface> irInterface;
   irInterface = new SageIRInterface(p, na, p_handle);


   /*
   //CFG
   OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
   cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);
   OA::OA_ptr<OA::CFG::CFG> cfg=
   cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
   */

   // eachCFG
   OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
   OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
   cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
   eachCFG = new OA::CFG::EachCFGStandard(cfgman);


   //FIAlias
   OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
   fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
   OA::OA_ptr<SageIRProcIterator> procIter;
   procIter = new SageIRProcIterator(p, *irInterface);
   OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
   interAlias = fialiasman->performAnalysis(procIter);

   
   // CallGraph
   OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> cgraphman;
   cgraphman = new OA::CallGraph::ManagerCallGraphStandard(irInterface);
   OA::OA_ptr<OA::CallGraph::CallGraph> cgraph =
      cgraphman->performAnalysis(procIter, interAlias);

   // ParamBindings
   OA::OA_ptr<OA::DataFlow::ManagerParamBindings> parambindman;
   parambindman = new OA::DataFlow::ManagerParamBindings(irInterface);
   OA::OA_ptr<OA::DataFlow::ParamBindings> parambind
      = parambindman->performAnalysis(cgraph);

   // Intra Side-Effect
   OA::OA_ptr<OA::SideEffect::ManagerSideEffectStandard> sideeffectman;
   sideeffectman = new OA::SideEffect::ManagerSideEffectStandard(irInterface);

   // InterSideEffect
   OA::OA_ptr<OA::SideEffect::ManagerInterSideEffectStandard> interSEman;
   interSEman = new OA::SideEffect::ManagerInterSideEffectStandard(irInterface);

   OA::OA_ptr<OA::SideEffect::InterSideEffectStandard> interSE;
   interSE = interSEman->performAnalysis(cgraph, parambind,
                                        interAlias, sideeffectman,
                                        OA::DataFlow::ITERATIVE);

   // ICFG
   OA::OA_ptr<OA::ICFG::ManagerICFGStandard> icfgman;
   icfgman = new OA::ICFG::ManagerICFGStandard(irInterface);
   OA::OA_ptr<OA::ICFG::ICFG> icfg;
   icfg = icfgman->performAnalysis(procIter,eachCFG,cgraph);

   // ICFGActive
   OA::OA_ptr<OA::Activity::ManagerICFGActive> activeman;
   activeman = new OA::Activity::ManagerICFGActive(irInterface);
   OA::OA_ptr<OA::Activity::InterActive> active;
   active = activeman->performAnalysis(icfg, parambind,
                                        interAlias, interSE,
                                        OA::DataFlow::ITERATIVE);

   active->output(*irInterface);

   int numIterDep = 1;
   int numIterActive = 1;
   int numTotal = numIterDep + active->getNumIterUseful()
     + active->getNumIterVary() + numIterActive;
   std::cout << "\n\nTotal Iters: " << numTotal << std::endl;

   std::cout << "\n*******  end of DoICFGActivity *********\n\n";
   return 0;
}
#endif



//<AIS|ATB>
#if 0
int DoICFGReachConsts(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*>* na, bool p_handle)
{
   int returnvalue=FALSE;
   if ( debug )   printf("*******start of DoICFGReachConsts **********\n");
   OA::OA_ptr<SageIRInterface> irInterface;
   irInterface = new SageIRInterface(p, na, p_handle);
   //irInterface->createNodeArray(sgproject); //what about global vars?

   // eachCFG
   OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
   OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
   cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
   eachCFG = new OA::CFG::EachCFGStandard(cfgman);

   //FIAlias
   OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
   fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
   OA::OA_ptr<SageIRProcIterator> procIter;
   procIter = new SageIRProcIterator(p, *irInterface);
   OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
   interAlias = fialiasman->performAnalysis(procIter);

   // create CallGraph Manager and then Call Graph
   OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> callgraphmanstd;
   callgraphmanstd= new OA::CallGraph::ManagerCallGraphStandard(irInterface);
   //    OA::OA_ptr<SageIRProcIterator> procIter;
   //    procIter = new SageIRProcIterator(sgproject, irInterface);

   OA::OA_ptr<OA::CallGraph::CallGraph> callgraph;
   callgraph = callgraphmanstd->performAnalysis(procIter,interAlias);

   // create ICFG Manager and then ICFG
   OA::OA_ptr<OA::ICFG::ICFG> icfg;
   icfg = new OA::ICFG::ICFG();
   OA::OA_ptr<OA::ICFG::ManagerICFGStandard> icfgman;
   icfgman = new OA::ICFG::ManagerICFGStandard(irInterface);
   icfg = icfgman->performAnalysis(procIter,eachCFG,callgraph);

   // output ICFG
   //icfg->output(*irInterface);
   // dot output
   //OA::OA_ptr<OA::OutputBuilder> outBuild;
   //outBuild = new OA::OutputBuilderDOT;
   //icfg->configOutput(outBuild);
   //icfg->output(*irInterface);


   // ParamBindings
   OA::OA_ptr<OA::DataFlow::ManagerParamBindings> parambindman;
   parambindman = new OA::DataFlow::ManagerParamBindings(irInterface);
   OA::OA_ptr<OA::DataFlow::ParamBindings> parambind
      = parambindman->performAnalysis(callgraph);


   // Intra Side-Effect
   OA::OA_ptr<OA::SideEffect::ManagerSideEffectStandard> sideeffectman;
   sideeffectman = new OA::SideEffect::ManagerSideEffectStandard(irInterface);

   // InterSideEffect
   OA::OA_ptr<OA::SideEffect::ManagerInterSideEffectStandard> interSEman;
   interSEman = new OA::SideEffect::ManagerInterSideEffectStandard(irInterface);

   OA::OA_ptr<OA::SideEffect::InterSideEffectStandard> interSE;
   interSE = interSEman->performAnalysis(callgraph, parambind,
                                        interAlias, sideeffectman,
                                        OA::DataFlow::ITERATIVE);
 
   // ICFGReachConsts
   OA::OA_ptr<OA::ReachConsts::ManagerICFGReachConsts> ircsman;
   ircsman = new OA::ReachConsts::ManagerICFGReachConsts(irInterface);
   OA::OA_ptr<OA::ReachConsts::InterReachConsts> ircs
      = ircsman->performAnalysis(icfg,parambind,interAlias,interSE,
                                 OA::DataFlow::ITERATIVE);

   ircs->output(*irInterface);
 
   std::cout << "\n*******  end of DoICFGReachConsts *********\n\n";
   return returnvalue;
}
#endif

//<AIS|ATB>
#if 0
int DoUDDUChains(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;

  if ( debug ) printf("*******start of DoUDDUChains\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);

  OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
  cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

  // CFG
  //*********** this gets the same free error
  OA::OA_ptr<OA::CFG::CFG> cfg=
  cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));

  //Alias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p,*irInterface);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);
  OA::ProcHandle proc((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  OA::OA_ptr<OA::Alias::Interface> alias = interAlias->getAliasResults(proc);

  // SideEffect
 // Interprocedural Side-Effect Analysis
  // for now generate default conservative interprocedural side-effect results
  OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
  interSideEffect = new OA::SideEffect::InterSideEffectStandard;


  // then can do ReachDefs
  OA::OA_ptr<OA::ReachDefs::ManagerReachDefsStandard> rdman;
  rdman = new OA::ReachDefs::ManagerReachDefsStandard(irInterface);
  OA::OA_ptr<OA::ReachDefs::ReachDefsStandard> rds=
     rdman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),
                            cfg,alias,interSideEffect,
                            OA::DataFlow::ITERATIVE);

  // then UDDUChains
  OA::OA_ptr<OA::UDDUChains::ManagerUDDUChainsStandard> udman;
  udman = new OA::UDDUChains::ManagerUDDUChainsStandard(irInterface);
  OA::OA_ptr<OA::UDDUChains::UDDUChainsStandard> udduchains=
      udman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),alias,rds,interSideEffect);

  //udduchains->dump(std::cout, irInterface);
  udduchains->output(*irInterface);
  


}
#endif

//<AIS|ATB>
#if 0
int DoUDDUChainsXAIF(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;

  if ( debug ) printf("*******start of DoUDDUChainsXAIF\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);

  OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
  cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);


  //*********** this gets the same free error
  OA::OA_ptr<OA::CFG::CFG> cfg=
  cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));

  cfg->output(*irInterface);




  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p,*irInterface);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);
  OA::ProcHandle proc((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  OA::OA_ptr<OA::Alias::Interface> alias = interAlias->getAliasResults(proc);



  // Interprocedural Side-Effect Analysis
  // for now generate default conservative interprocedural side-effect results
  OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
  interSideEffect = new OA::SideEffect::InterSideEffectStandard;

  // then can do ReachDefs
  OA::OA_ptr<OA::ReachDefs::ManagerReachDefsStandard> rdman;
  rdman = new OA::ReachDefs::ManagerReachDefsStandard(irInterface);
  OA::OA_ptr<OA::ReachDefs::ReachDefsStandard> rds=
     rdman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),
                            cfg,alias,interSideEffect,
                            OA::DataFlow::ITERATIVE);

  rds->output(*irInterface);

 

  /*
  if ( debug ) printf("*******start of DoUDDUChains\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);
  //irInterface->createNodeArray(f); //what about global vars? //done in constr.
  // CFG
  OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
  cfgmanstd = new OA::CFG::ManagerStandard(irInterface);
  OA::OA_ptr<OA::CFG::CFGInterface> cfg=
     cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  
#if 0
  //AliasMap
  OA::OA_ptr<OA::Alias::ManagerAliasMapBasic> aliasmanstd;
  iroaptr=irInterface;
  aliasmanstd= new OA::Alias::ManagerAliasMapBasic(iroaptr);
  OA::OA_ptr<OA::Alias::AliasMap> alias = 
  aliasmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
#else
  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasEquivSets> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasEquivSets(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, irInterface);
  OA::OA_ptr<OA::Alias::EquivSets> alias = 
    fialiasman->performAnalysis(procIter);
#endif
  //alias->dump(std::cout, iroaptr);
  
  // Interprocedural Side-Effect Analysis
  // for now generate default conservative interprocedural side-effect results
  OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
  interSideEffect = new OA::SideEffect::InterSideEffectStandard;

  // then can do ReachDefs
  OA::OA_ptr<OA::ReachDefs::ManagerReachDefsStandard> rdman;
  rdman = new OA::ReachDefs::ManagerReachDefsStandard(irInterface);
  OA::OA_ptr<OA::ReachDefs::ReachDefsStandard> rds= 
      rdman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),cfg,alias,interSideEffect);
*/

  // then UDDUChains
  OA::OA_ptr<OA::UDDUChains::ManagerUDDUChainsStandard> udman;
  udman = new OA::UDDUChains::ManagerUDDUChainsStandard(irInterface);
  OA::OA_ptr<OA::UDDUChains::UDDUChainsStandard> udduchains= 
      udman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),alias,rds,interSideEffect);

  udduchains->dump(std::cout, irInterface);

 // and finally UDDUChainsXAIF
  OA::OA_ptr<OA::XAIF::ManagerUDDUChainsXAIF> udmanXAIF;
  udmanXAIF = new OA::XAIF::ManagerUDDUChainsXAIF(irInterface);
  OA::OA_ptr<OA::XAIF::UDDUChainsXAIF> udduchainsXAIF=
  udmanXAIF->performAnalysis(cfg,udduchains);
  udduchainsXAIF->dump(std::cout, irInterface);

  
	std::cout << "\n*******  end of DoUDDUChainsXAIF *********\n\n";
	return returnvalue;
}
#endif

// perform GCD Data-Dependence analysis.  For now this function manually
// constructs a Loop object based on the command line arguments.  The
// data-depanalysis manager uses this.  This is because OA doesn't yet have
// Loop detection analysis.
//<AIS|ATB>
#if 0
int DoDataDepGCD(
    SgProject * proj,
    std::vector<SgNode*> * na,
    bool p_handle,
    char argc, char *argv[])
{
    // construct the IR interface
    OA::OA_ptr<SageIRInterface> ir;
    ir = new SageIRInterface(proj, na, p_handle);

    // conduct alias analysis
    OA_ptr<ManagerFIAliasAliasMap> fialiasman;
    fialiasman = new ManagerFIAliasAliasMap(ir);
    OA_ptr<SageIRProcIterator> procIter;
    procIter = new SageIRProcIterator(proj, *ir);
    OA_ptr<InterAliasMap> interAliasMap;
    interAliasMap = fialiasman->performAnalysis(procIter);
    OA_ptr<Alias::Interface> aliasResults;
    aliasResults = interAliasMap.convert<Alias::Interface>();

    // iterate through the command line arguments searching for the
    // --oa-DataDepGCD flag.  After this will be the the index variable
    // identifier, lower bound, upper bound, and step values.  Note:
    // This is temporary and as such I don't do anything to check that
    // the command line arguments are valid.
    string sIndexVar;
    int lowerBound;
    int upperBound;
    int step = -1;
    for(int i = 0; i < argc; i++) {
        if(strcmp(argv[i], "--oa-DataDepGCD") == 0) {
            assert(argc > i + 4);

            sIndexVar = argv[i + 1];
            lowerBound = atoi(argv[i + 2]);
            upperBound = atoi(argv[i + 3]);
            step = atoi(argv[i + 4]);
            break;
        }
    }
    assert(step != -1);

    // gather the named location associated with the index variable
    // identifier
    OA_ptr<NamedLoc> namedLoc;
    ProcHandle proc;
    OA_ptr<SageIRSymIterator> iterSym = ir->getSymIterator(proj);
    for(; iterSym->isValid(); ++(*iterSym)) {
        SymHandle hSym = iterSym->current();

        if(ir->toString(hSym) == sIndexVar) {
            SgFunctionDefinition *func = UseOA::getEnclosingFunction(
                ir->getNodePtr(hSym));
            proc = ir->getProcHandle(func);
            namedLoc = (ir->getLocation(proc, hSym)).convert<NamedLoc>();
            break;
        }
    }
    if(namedLoc.ptrEqual(NULL)) {
        cerr << "Could not find index variable " << sIndexVar << endl;
        exit(1);
    }

    // construct a loop object
    cerr << "TODO: Implement DataDepGCD\n";
    assert(false);
    /*
    OA_ptr<LoopIndex> loopIdx;
    loopIdx =
        new LoopIndex(namedLoc, lowerBound, upperBound, step);
    OA_ptr<LoopAbstraction> loop;
    loop = new Loop(loopIdx);

    // construct the data dep GCD manager and perform the analysis
    OA_ptr<ManagerDataDepGCD> gcdAnal;
    gcdAnal = new ManagerDataDepGCD(ir, aliasResults);
    OA_ptr<DataDepResults> results;
    results = gcdAnal->performAnalysis(proc, loop);

    // output the results
    results->output(*ir);
    */
}
#endif


// The following was taken from 
// ROSE/test/roseTests/programAnalysisTests/MemRefExprTest.C.
// It produces output without pointers.
 
enum refType {
  USE,
  DEF,
  USEDEF,
  DEFUSE
};

refType getRefType(OA::OA_ptr<OA::MemRefExpr> memRefExp)
{
  refType type = USE;

  if ( memRefExp->isUseDef() ) type = USEDEF;
  else if ( memRefExp->isDefUse() ) type = DEFUSE;
  else if ( memRefExp->isDef() ) type = DEF;
  else if ( memRefExp->isUse() ) type = USE;
  else ROSE_ABORT();

  return type;
}

string refTypeToString(refType rType)
{
  string refTypeStr;

  if ( rType == DEF ) refTypeStr = "DEF";
  else if ( rType == USE ) refTypeStr = "USE";
  else if ( rType == USEDEF ) refTypeStr = "USEDEF";
  else if ( rType == DEFUSE ) refTypeStr = "DEFUSE";
  else ROSE_ABORT();

  return refTypeStr;
}

void dump(OA::OA_ptr<OA::NamedRef> memRefExp, bool addressTaken,
	  bool fullAccuracy, refType mrType, int numDeref,
	  SageIRInterface *ir, std::ostream& os) 
{
  OA::SymHandle symHandle = memRefExp->getSymHandle();

  os << "< " << refTypeToString(mrType) << ", T, ";
  os << "SymHandle(" << ir->toString(symHandle) << "), ";
  os << numDeref << ", " << ( addressTaken ? "T, " : "F, " );
  os << ( fullAccuracy ? "full" : "partial" ) << " > " << endl;
}

void dump(OA::OA_ptr<OA::UnnamedRef> memRefExp, bool addressTaken,
	  bool fullAccuracy, refType mrType, int numDeref,
	  SageIRInterface *ir, std::ostream& os) 
{
  os << "< " << refTypeToString(mrType) << ", F, Unnamed, ";
  os << numDeref << ", " << ( addressTaken ? "T, " : "F, " );
  os << ( fullAccuracy ? "full" : "partial" ) << " > " << endl;
}

void dump(OA::OA_ptr<OA::UnknownRef> memRefExp, bool addressTaken,
	  bool fullAccuracy, refType mrType, int numDeref,
	  SageIRInterface *ir, std::ostream& os) 
{
  os << "< " << refTypeToString(mrType) << ", F, UnknownRef, ";
  os << numDeref << ", " << ( addressTaken ? "T, " : "F, " );
  os << ( fullAccuracy ? "full" : "partial" ) << " > " << endl;
}

void dump(OA::OA_ptr<OA::MemRefExpr> memRefExp, bool addressTaken,
	  bool fullAccuracy, refType mrType, int numDerefs,
	  SageIRInterface *ir, std::ostream& os) 
{

  if ( memRefExp->isaNamed() ) {

    OA::OA_ptr<OA::NamedRef> namedRef = memRefExp.convert<OA::NamedRef>();
    ROSE_ASSERT(!namedRef.ptrEqual(0));

    dump(namedRef, addressTaken, fullAccuracy, mrType, numDerefs, ir, os);

  } else if ( memRefExp->isaUnnamed() ) {

    OA::OA_ptr<OA::UnnamedRef> unnamedRef = memRefExp.convert<OA::UnnamedRef>();
    ROSE_ASSERT(!unnamedRef.ptrEqual(0));

    dump(unnamedRef, addressTaken, fullAccuracy, mrType, numDerefs, ir, os);

  } else if ( memRefExp->isaUnknown() ) {

    OA::OA_ptr<OA::UnknownRef> unknownRef = memRefExp.convert<OA::UnknownRef>();
    ROSE_ASSERT(!unknownRef.ptrEqual(0));

    dump(unknownRef, addressTaken, fullAccuracy, mrType, numDerefs, ir, os);

  } else if ( memRefExp->isaRefOp() ) {

    OA::OA_ptr<OA::RefOp> refOp = memRefExp.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    if ( refOp->isaDeref() ) {

      OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = refOp->getMemRefExpr();
      OA::OA_ptr<OA::Deref> deref = refOp.convert<OA::Deref>();
      ROSE_ASSERT(!deref.ptrEqual(0));

      numDerefs += deref->getNumDerefs();

      dump(baseMemRefExpr, addressTaken, fullAccuracy, mrType, numDerefs, ir, os);

    } else {

      cerr << "Unknown memRefExp type!" << endl;
      ROSE_ABORT();

    }

  } else {

    cerr << "Unknown memRefExp type!" << endl;
    ROSE_ABORT();

  }



}

/*
void dump(OA::OA_ptr<OA::MemRefExpr> memRefExp, SageIRInterface *ir,
	  std::ostream &os)
{
  refType type = getRefType(memRefExp);

  bool addressTaken;

 // deprecated addressTaken 1/2/2007
  bool addressTaken = memRefExp->hasAddressTaken();
  
  bool fullAccuracy = memRefExp->hasFullAccuracy();
  int numDerefs = 0;
  
  dump(memRefExp, addressTaken, fullAccuracy, type, numDerefs, ir, os);

}
*/

/*
void prettyPrintNamedRef(OA::OA_ptr<OA::NamedRef> memRefExp, 
			 SageIRInterface *ir, std::ostream& os) 
{
  OA::SymHandle symHandle = memRefExp->getSymHandle();
  refType mrType = getRefType(memRefExp);

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Named Ref: addressTaken = F, fullAccuracy = full.
  //  deprecated addressTaken 1/2/2007
  bool addressTaken = memRefExp->hasAddressTaken();
  
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "NamedRef(";
  os << refTypeToString(mrType);
  os << ", SymHandle(" << ir->toString(symHandle) << ")";

  if ( ( addressTaken != false ) || ( fullAccuracy != true ) ) {
    if ( addressTaken == true ) 
      os << ", T";
    else
      os << ", F";
    if ( fullAccuracy == true ) 
      os << ", full";
    else
      os << ", partial";
  }
  os << ")";
}
*/

/*
void prettyPrintUnnamedRef(OA::OA_ptr<OA::UnnamedRef> memRefExp, 
			   SageIRInterface *ir, std::ostream& os) 
{
  refType mrType = getRefType(memRefExp);

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Unnamed Ref: addressTaken = T, fullAccuracy = partial.
  
 //  deprecated addressTaken 1/2/2007
  bool addressTaken = memRefExp->hasAddressTaken();
  
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "UnnamedRef(";
  os << refTypeToString(mrType);
  os << ", StmtHandle";

  
  if ( ( addressTaken != true ) || ( fullAccuracy != false ) ) {
    if ( addressTaken == true ) 
      os << ", T";
    else
      os << ", F";
    if ( fullAccuracy == true ) 
      os << ", full";
    else
      os << ", partial";
  }
  os << ")";
  
}
*/

/*
void prettyPrintUnknownRef(OA::OA_ptr<OA::UnknownRef> memRefExp, 
			   SageIRInterface *ir, std::ostream& os) 
{
  refType mrType = getRefType(memRefExp);

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Unknown Ref: addressTaken = F, fullAccuracy = partial.
  
  bool addressTaken = memRefExp->hasAddressTaken();
  
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "UnknownRef(";
  os << refTypeToString(mrType);

  if ( ( addressTaken != false ) || ( fullAccuracy != false ) ) {
    if ( addressTaken == true ) 
      os << ", T";
    else
      os << ", F";
    if ( fullAccuracy == true ) 
      os << ", full";
    else
      os << ", partial";
  }
}
*/

/*
void prettyPrintDeref(OA::OA_ptr<OA::Deref> memRefExp, 
		      SageIRInterface *ir, std::ostream& os) 
{
  int numDerefs = memRefExp->getNumDerefs();
  OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = memRefExp->getMemRefExpr();
  refType mrType = getRefType(memRefExp);

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Deref: addressTaken = F, fullAccuracy = partial.
  
  // deprecated addressTaken 1/2/2007
  bool addressTaken = memRefExp->hasAddressTaken();
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "Deref(";
  os << refTypeToString(mrType) << ", ";
  prettyPrintMemRefExp(baseMemRefExpr, ir, os);
  os << ", ";
  os << numDerefs;

  
  if ( ( addressTaken != false ) || ( fullAccuracy != false ) ) {
    if ( addressTaken == true ) 
      os << ", T";
    else
      os << ", F";
    if ( fullAccuracy == true ) 
      os << ", full";
    else
      os << ", partial";
  }
 
  os << ")";
}
*/

/*
void prettyPrintMemRefExp(OA::OA_ptr<OA::MemRefExpr> memRefExp, SageIRInterface *ir,
	  std::ostream &os)
{

  if ( memRefExp->isaNamed() ) {

    OA::OA_ptr<OA::NamedRef> namedRef = memRefExp.convert<OA::NamedRef>();
    ROSE_ASSERT(!namedRef.ptrEqual(0));

    prettyPrintNamedRef(namedRef, ir, os);

  } else if ( memRefExp->isaUnnamed() ) {

    OA::OA_ptr<OA::UnnamedRef> unnamedRef = memRefExp.convert<OA::UnnamedRef>();
    ROSE_ASSERT(!unnamedRef.ptrEqual(0));

    prettyPrintUnnamedRef(unnamedRef, ir, os);

  } else if ( memRefExp->isaUnknown() ) {

    OA::OA_ptr<OA::UnknownRef> unknownRef = memRefExp.convert<OA::UnknownRef>();
    ROSE_ASSERT(!unknownRef.ptrEqual(0));

    prettyPrintUnknownRef(unknownRef, ir, os);

  } else if ( memRefExp->isaRefOp() ) {

    OA::OA_ptr<OA::RefOp> refOp = memRefExp.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    if ( refOp->isaDeref() ) {

      OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = refOp->getMemRefExpr();
      OA::OA_ptr<OA::Deref> deref = refOp.convert<OA::Deref>();
      ROSE_ASSERT(!deref.ptrEqual(0));

      prettyPrintDeref(deref, ir, os);

    } else {

      cerr << "Unknown memRefExp type!" << endl;
      ROSE_ABORT();

    }

  } else {

    cerr << "Unknown memRefExp type!" << endl;
    ROSE_ABORT();

  }
}
*/

/*
void dump(OA::OA_ptr<OA::MemRefExprIterator> memRefIterator,
	  SageIRInterface *ir, std::ostream& os) {
  
  for ( ; memRefIterator->isValid(); (*memRefIterator)++ ) { 
    OA::OA_ptr<OA::MemRefExpr> memRefExp = memRefIterator->current();
    ROSE_ASSERT(!memRefExp.ptrEqual(0));

    //    dump(memRefExp, ir, os);
    prettyPrintMemRefExp(memRefExp, ir, os);
    os << endl;
  }
}
*/


//<AIS|ATB>
#if 0
void analyzeSubscript(
    OA_ptr<IdxExprAccess> exp,
    OA_ptr<LoopIRInterface> ir,
    OA_ptr<Alias::Interface> aliasResults,
    OA_ptr<LoopResults> results)
{
    assert(false);
    #if 0
    // get the loop which sorrounds the subscript
    OA_ptr<LoopAbstraction> loop;
    MemRefHandle mref = ir->getMemRefHandle(exp);
    StmtHandle stmt = ir->getStmt(mref);
    StmtHandle hLoop = ir->findEnclosingLoop(stmt);
    loop = results->getLoop(hLoop);

    // get global parameters
    OA_ptr<list<OA_ptr<LoopIndex> > > indices = results->getIndexVars(loop);

    for(list<OA_ptr<LoopIndex> >::iterator i = indices->begin();
        i != indices->end(); i++)
    {
        cout << "index: " << ir->toString((*i)->getVariable()->getSymHandle())
             << endl;
    }

    // attempt to pass the subscript expression through an affine expression
    // analyzer.
    AffineExpr::AffineAnlState affineExpAnlErrState;
    OA_ptr<ManagerAffineExpr> affineExpAnl;
    affineExpAnl = new ManagerAffineExpr(ir, aliasResults);
    cout << "Analyze: " << ir->toString(exp->getExpr()) << endl;

    // get the expression tree for the subscript then try and convert it into
    // an affine expression, the success or failure of this helps determine
    // whether an access is linear or not
    OA_ptr<ExprTree> expTree = ir->getExprTree(exp->getExpr());
    affineExpAnl->exprTreeToAffineExpr(*expTree, &affineExpAnlErrState);
    switch(affineExpAnlErrState) {
        case INVALID_OPERATOR:
            cout << "Invalid operator state" << endl;
            break;
    
        case NON_LINEAR_TERM:
            cout << "Nonlinear term state" << endl;
            break;

        case UNKNOWN_VAR:
            cout << "Unknown variable state" << endl;
            break;
    }
    #endif
}
#endif



//<AIS|ATB>
#if 0
int DoLoop(
    SgProject * p,
    std::vector<SgNode*> * na,
    bool p_handle)
{
    // construct a Sage interface
    OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);
    
    // construct an iterator over all the procedures in the project
    OA_ptr<SageIRProcIterator> procIter;
    procIter = new SageIRProcIterator(p, *irInterface);

    // perform alias analysis
    OA_ptr<Alias::ManagerFIAliasAliasMap> fialiasman;
    fialiasman = new Alias::ManagerFIAliasAliasMap(irInterface);
    OA_ptr<Alias::InterAliasMap> aliasMaps;
    aliasMaps = fialiasman->performAnalysis(procIter);

    // construct the manager for loop detection analysis
    OA_ptr<Loop::LoopManager> loopMngr;
    loopMngr = new LoopManager(irInterface);

    // iterate across all the procedures in the program, perform the loop
    // detection analysis on all of them.
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA_ptr<LoopResults> results;
        results = loopMngr->performLoopDetection(procIter->current());
        // results->printLoopTree();

        // iterate through index expressions in the procedure analyzing them
        /*
        OA_ptr<IdxExprAccessIterator> j =
            irInterface->getIdxExprAccessIter(procIter->current());
        for(; j->isValid(); (*j)++) {
            OA_ptr<IdxExprAccess> access = j->current();
            analyzeSubscript(
                access,
                irInterface,
                aliasMaps->getAliasResults(procIter->current()),
                results);
        */
            // for now get the enclosing loop and print information about it
            // out
            /*
            MemRefHandle mref = irInterface->getMemRefHandle(access);

            StmtHandle stmt = irInterface->getStmt(mref);
            StmtHandle loop = irInterface->findEnclosingLoop(stmt);

            cout << "Loop enclosing "
                 << irInterface->toString(stmt) << ": "
                 << irInterface->toString(loop) << endl;
           /
        }*/
    }

    

    // The code below whould be sectioned off into a manager class for
    // something like affine expression linearity analysis.

}
#endif




//<AIS|ATB>
#if 0
int DoLinearity(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
   int returnvalue=FALSE;
   OA::OA_ptr<SageIRInterface> irInterface;
   irInterface = new SageIRInterface(p, na, p_handle);


   //CFG
   OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
   cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);
   OA::OA_ptr<OA::CFG::CFG> cfg=
   cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));

   //Alias
   OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
   fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
   OA::OA_ptr<SageIRProcIterator> procIter;
   procIter = new SageIRProcIterator(p,*irInterface);
   OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
   interAlias = fialiasman->performAnalysis(procIter);
   OA::ProcHandle proc((OA::irhandle_t)(irInterface->getNodeNumber(f)));
   OA::OA_ptr<OA::Alias::Interface> alias = interAlias->getAliasResults(proc);

   // CallGraph
  OA::OA_ptr<OA::CallGraph::ManagerCallGraphStandard> cgraphman;
  cgraphman = new OA::CallGraph::ManagerCallGraphStandard(irInterface);
  OA::OA_ptr<OA::CallGraph::CallGraph> cgraph =
      cgraphman->performAnalysis(procIter, interAlias);

   // ParamBindings
   OA::OA_ptr<OA::DataFlow::ManagerParamBindings> parambindman;
   parambindman = new OA::DataFlow::ManagerParamBindings(irInterface);
   OA::OA_ptr<OA::DataFlow::ParamBindings> parambind
      = parambindman->performAnalysis(cgraph);

   OA::OA_ptr<OA::Linearity::ManagerLinearity> linmanstd;
   linmanstd = new OA::Linearity::ManagerLinearity(irInterface);
   OA::OA_ptr<OA::Linearity::LinearityMatrix> LM
       = linmanstd->performAnalysis(
               (OA::irhandle_t)irInterface->getNodeNumber(f),cfg,
                alias,parambind,OA::DataFlow::ITERATIVE);

    LM->output(*irInterface);
   

   return returnvalue;

}
#endif

//<AIS|ATB>
#if 0
int DoExprTree(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{

  if ( debug ) printf("*******start of DoExprTree\n");
  OA::OA_ptr<SageIRInterface> ir;
  ir = new SageIRInterface(p, na, p_handle);

  // iterate over all statements
  //   iterate over ExprHandles for the statement
  //     create ExprTree(expr)
  OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(f)));
  OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
  for ( ; sIt->isValid(); (*sIt)++) {
    OA::StmtHandle stmt = sIt->current();
    OA::OA_ptr<OA::ExprHandleIterator> exprIter;
    exprIter = ir->getExprHandleIterator(stmt);
    std::cout << "\n========================stmt============================\n";
    std::cout << "\nstmt = ";
    std::cout << ir->toString(stmt) << std::endl;

    for ( ; exprIter->isValid(); (*exprIter)++) {
         OA::ExprHandle expr = exprIter->current();
         std::cout << "\n\t--expr----------------------------------------\n";
         std::cout << "\t  expr = " << ir->toString(expr) << std::endl;
         std::cout << "\t----------------------------------------------";
         OA::OA_ptr<OA::ExprTree> eTreePtr = ir->getExprTree(expr);
         eTreePtr->output(*ir);
    }

  }
  return 0;

  /*  
  if ( debug ) printf("*******start of DoExprTree\n");
  OA::OA_ptr<SageIRInterface> ir;
  ir = new SageIRInterface(p, na, p_handle);
  
  // iterate over all statements
  OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(f)));
  OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
  for ( ; sIt->isValid(); (*sIt)++)
  {
     OA::StmtHandle stmt = sIt->current();
     std::cout << "\n========================stmt============================\n";
     std::cout << "\nstmt = " << ir->toString(stmt) << std::endl;
     
     // if the statement has an expression tree then dump that as well
     //     OA::ReachConsts::IRStmtType sType = ir->getReachConstsStmtType(stmt);
     OA::Activity::IRStmtType sType = ir->getActivityStmtType(stmt);
     //     if (sType == OA::ReachConsts::EXPR_STMT) {
     if (sType == OA::Activity::EXPR_STMT) {
       OA::OA_ptr<OA::AssignPairIterator> espIterPtr
           = ir->getAssignPairIterator(stmt);
       for ( ; espIterPtr->isValid(); (*espIterPtr)++) {
         // unbundle pair
         OA::MemRefHandle mref = espIterPtr->currentTarget();
         OA::ExprHandle expr = espIterPtr->currentSource();
         std::cout << "\n\t--expr----------------------------------------\n";
         std::cout << "\t  expr = " << ir->toString(expr) << std::endl;
         std::cout << "\t----------------------------------------------";
         OA::OA_ptr<OA::ExprTree> eTreePtr = ir->getExprTree(expr);
         eTreePtr->output(*ir);
      }
     //}

    // print out all of the ExprTrees for the parameters
    // Iterate over procedure calls of a statement
    OA::OA_ptr<OA::IRCallsiteIterator> callsiteItPtr = ir->getCallsites(stmt);
    for ( ; callsiteItPtr->isValid(); ++(*callsiteItPtr)) {
        OA::CallHandle call = callsiteItPtr->current();
        //if (debug) {
        std::cout << "\n\t--Call-----------------------------------------\n";
        std::cout << "\t  Call: [" << ir->toString(call) << "]\n";
        std::cout << "\t----------------------------------------------";
        //}
        
        OA::OA_ptr<OA::IRCallsiteParamIterator> paramIterPtr
            = ir->getCallsiteParams(call);
        for ( ; paramIterPtr->isValid(); (*paramIterPtr)++ ) {
            OA::ExprHandle param = paramIterPtr->current();
           
            // get the expression tree for the parameter
            OA::OA_ptr<OA::ExprTree> eTreePtr = ir->getExprTree(param);
            eTreePtr->output(*ir);
        }
    }
  } 
  */
}
#endif

//<AIS|ATB>
#if 0
int DoAssignPairs(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  if ( debug ) printf("*******start of DoPtrAssign\n");

  OA::OA_ptr<SageIRInterface> ir;
  ir = new SageIRInterface(p, na, p_handle);

  // iterate over all statements
  OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(f)));
  OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
  for ( ; sIt->isValid(); (*sIt)++)
  {
     OA::StmtHandle stmt = sIt->current();
     std::cout << "\n==========stmt========\n";
     std::cout << "\nstmt = ( " << ir->toString(stmt) << " ) "<< std::endl;
     OA::OA_ptr<OA::AssignPairIterator> espIterPtr
         = ir->getAssignPairIterator(stmt);
     for ( ; espIterPtr->isValid(); (*espIterPtr)++) {
         // unbundle pair
         OA::MemRefHandle mref = espIterPtr->currentTarget();
         OA::ExprHandle expr = espIterPtr->currentSource();
         std::cout << "\n\t--expr----------------------------------------\n";
         std::cout << "\t  expr = " << ir->toString(expr) << std::endl;
         std::cout << "\t----------------------------------------------";
     }
  }
}
#endif


//<AIS|ATB>
#if 0
// look at the last two expression trees in the source program's function and
// determine their ordering.
int DoCompareExpressionTrees(
    SgFunctionDefinition *f,
    SgProject *p,
    std::vector<SgNode*> *na,
    bool p_handle)
{
    ExprHandle prevExp = NULL, currentExp = NULL;
    OA_ptr<NewExprTree> lhsExp, rhsExp;

    OA::OA_ptr<SageIRInterface> ir;
    ir = new SageIRInterface(p, na, p_handle);

    // iterate over all statements
    //   iterate over ExprHandles for the statement
    //     create ExprTree(expr)
    OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(f)));
    OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
    for ( ; sIt->isValid(); (*sIt)++) {
        OA::StmtHandle stmt = sIt->current();
        OA::OA_ptr<OA::ExprHandleIterator> exprIter;
        exprIter = ir->getExprHandleIterator(stmt);

        if(exprIter->isValid()) {
            prevExp = currentExp;
            currentExp = exprIter->current();
        }
    }

    assert((int)prevExp != NULL && (int)currentExp != NULL);

    cout << "Compare expressions: " << endl;
    cout << "\t lhs: " << ir->toString(prevExp) << endl;
    cout << "\t rhs: " << ir->toString(currentExp) << endl;

    lhsExp = ir->getNewExprTree(prevExp);
    rhsExp = ir->getNewExprTree(currentExp);

    lhsExp->output(*ir);
    rhsExp->output(*ir);

    cout << "\033[32mChecking lhs < rhs\033[0m\n";
    bool lhsLTRhs = *lhsExp < *rhsExp;

    cout << "\033[32mChecking rhs < lhs\033[0m\n";
    bool rhsLTLhs = *rhsExp < *lhsExp;

    cout << "\033[32mChecking lhs == rhs\033[0m\n";
    bool lhsEqRhs = *lhsExp == *rhsExp;

    cout << "\033[32mChecking rhs == lhs\033[0m\n";
    bool rhsEqLhs = *rhsExp == *lhsExp;

    if(lhsLTRhs) { cout << "(lhs < rhs)  == true\n";  }
    else         { cout << "(lhs < rhs)  == false\n"; }

    if(rhsLTLhs) { cout << "(rhs < lhs)  == true\n";  }
    else         { cout << "(rhs < lhs)  == false\n"; }

    if(lhsEqRhs) { cout << "(lhs == rhs) == true\n";  }
    else         { cout << "(lhs == rhs) == false\n"; }

    if(rhsEqLhs) { cout << "(rhs == lhs) == true\n";  }
    else         { cout << "(rhs == lhs) == false\n"; }

    return 0;
}
#endif

int DoDFAGenLiveness(SgProject *p, std::vector<SgNode*> *na, bool p_handle)
{
    int returnvalue=FALSE;
    OA::OA_ptr<SageIRProcIterator> procIter;

    if ( debug ) printf("*******start of DFAGen Liveness analysis\n");

    double time = clock();

    // contruct IR
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);
    printf("Construct IR Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    // CFG
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
    cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

    // eachCFG
    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);

    // force a CFG analysis on all procedures and time it
    time = clock();
    procIter = new SageIRProcIterator(p, *irInterface);
    int count = 0;
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());
        count++;
    }
    printf("CFG Time: %lf, %d procs analyzed\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC), count);

    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
    fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
    procIter = new SageIRProcIterator(p, *irInterface);
    OA::OA_ptr<OA::Alias::Interface> alias;
    alias = fialiasman->performAnalysis(procIter);
    printf("FIAlias Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    //TODO: Need to create interprocedural alias to pass to side effects
    OA::OA_ptr<OA::Alias::InterAliasInterface> interAlias;

    // Interprocedural Side-Effect Analysis
    // for now generate default conservative interprocedural side-effect results
    OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
    interSideEffect = new OA::SideEffect::InterSideEffectStandard(interAlias);
    printf("Side-effects Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();
    
    // Construct Liveness manager
    OA::OA_ptr<OA::Liveness::ManagerLiveness> liveMgr;
    liveMgr = new OA::Liveness::ManagerLiveness(irInterface);
    
    // Iterate through each proc doing Liveness
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::Liveness::Liveness> results;
        results = 
        liveMgr->performAnalysis(
            procIter->current(),
            eachCFG->getCFGResults(procIter->current()),
            alias,
            interSideEffect); 

        //if(!silent) { results->output(*irInterface); }
    }
    printf("Liveness Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    std::cout << "\n*******  end of DFAGen Liveness analysis *********\n\n";

    return returnvalue;
}
/*
int DoDFAGenLivenessBV(SgProject *p, std::vector<SgNode*> *na, bool p_handle)
{
    int returnvalue=FALSE;
    OA::OA_ptr<SageIRProcIterator> procIter;

    if ( debug ) printf("*******start of DFAGen Liveness analysis\n");

    double time = clock();

    // contruct IR
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);
    printf("Construct IR Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    // CFG
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
    cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

    // eachCFG
    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);

    // force a CFG analysis on all procedures and time it
    time = clock();
    procIter = new SageIRProcIterator(p, *irInterface);
    int count = 0;
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());
        count++;
    }
    printf("CFG Time: %lf, %d procs analyzed\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC), count);

    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
    fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
    procIter = new SageIRProcIterator(p, *irInterface);
    OA::OA_ptr<OA::Alias::Interface> alias;
    alias = fialiasman->performAnalysis(procIter);
    printf("FIAlias Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    //TODO: Need to create interprocedural alias to pass to side effects
    OA::OA_ptr<OA::Alias::InterAliasInterface> interAlias;

    // Interprocedural Side-Effect Analysis
    // for now generate default conservative interprocedural side-effect results
    OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
    interSideEffect = new OA::SideEffect::InterSideEffectStandard(interAlias);
    printf("Side-effects Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();
    
    // Construct Liveness manager
    OA::OA_ptr<OA::LivenessBV::ManagerLivenessBV> liveMgr;
    liveMgr = new OA::LivenessBV::ManagerLivenessBV(irInterface);

    // Iterate through each proc doing Liveness
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::LivenessBV::LivenessBV> results;
        results = liveMgr->performAnalysis(
            procIter->current(),
            eachCFG->getCFGResults(procIter->current()),
            alias,
            interSideEffect);

        if(!silent) { results->dump(std::cout, irInterface, *alias); }
    }
    printf("Liveness Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    std::cout << "\n*******  end of DFAGen LivenessBV analysis *********\n\n";

    return returnvalue;
}*/

int DoDFAGenReachDefs(SgProject *p, std::vector<SgNode*> *na, bool p_handle)
{
    int returnvalue=FALSE;
    OA::OA_ptr<SageIRProcIterator> procIter;

    if ( debug ) printf("*******start of DFAGen ReachingDefs analysis\n");

    double time = clock();

    // contruct IR
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);
    printf("Construct IR Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    // CFG
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
    cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

    // eachCFG
    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);

    // force a CFG analysis on all procedures and time it
    time = clock();
    procIter = new SageIRProcIterator(p, *irInterface);
    int count = 0;
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());
        count++;
    }
    printf("CFG Time: %lf, %d procs analyzed\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC), count);

    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
    fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
    procIter = new SageIRProcIterator(p, *irInterface);
    OA::OA_ptr<OA::Alias::Interface> alias;
    alias = fialiasman->performAnalysis(procIter);
    printf("FIAlias Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    //TODO: Need to create interprocedural alias to pass to side effects
    OA::OA_ptr<OA::Alias::InterAliasInterface> interAlias;

    // Interprocedural Side-Effect Analysis
    // for now generate default conservative interprocedural side-effect results
    OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
    interSideEffect = new OA::SideEffect::InterSideEffectStandard(interAlias);
    printf("Side-effects Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();
    
    // Construct reachdefs manager
    OA::OA_ptr<OA::ReachingDefs::ManagerReachingDefs> reachdefsMgr;
    reachdefsMgr = new OA::ReachingDefs::ManagerReachingDefs(irInterface);

    // Iterate through each proc doing reaching definitions analysis
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::ReachingDefs::ReachingDefs> results;
        results = reachdefsMgr->performAnalysis(
            procIter->current(),
            eachCFG->getCFGResults(procIter->current()),
            alias,
            interSideEffect); 

        if(!silent) { results->output(*irInterface); }
    }
    printf("ReachingDefs Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    std::cout << "\n*******  end of DFAGen ReachingDefs analysis *********\n\n";

    return returnvalue;
}


int DoDFAGenVary(SgProject *p, std::vector<SgNode*> *na, bool p_handle)
{
    int returnvalue=FALSE;
    OA::OA_ptr<SageIRProcIterator> procIter;

    if ( debug ) printf("*******start of DFAGen Vary analysis\n");

    double time = clock();

    // contruct IR
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);
    printf("Construct IR Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    // CFG
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
    cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

    // eachCFG
    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);

    // force a CFG analysis on all procedures and time it
    time = clock();
    procIter = new SageIRProcIterator(p, *irInterface);
    int count = 0;
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());
        count++;
    }
    printf("CFG Time: %lf, %d procs analyzed\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC), count);

    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
    fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
    procIter = new SageIRProcIterator(p, *irInterface);
    OA::OA_ptr<OA::Alias::Interface> alias;
    alias = fialiasman->performAnalysis(procIter);
    printf("FIAlias Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    //TODO: Need to create interprocedural alias to pass to side effects
    OA::OA_ptr<OA::Alias::InterAliasInterface> interAlias;

    // Interprocedural Side-Effect Analysis
    // for now generate default conservative interprocedural side-effect results
    OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
    interSideEffect = new OA::SideEffect::InterSideEffectStandard(interAlias);
    printf("Side-effects Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();
    
    // Construct reachdefs manager
    OA::OA_ptr<OA::Vary::ManagerVary> varyMgr;
    varyMgr = new OA::Vary::ManagerVary(irInterface);

    // Iterate through each proc doing reaching definitions analysis
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::Vary::Vary> results;
        results = varyMgr->performAnalysis(
            procIter->current(),
            eachCFG->getCFGResults(procIter->current()),
            alias,
            interSideEffect); 

        if(!silent) { results->output(*irInterface); }
    }
    printf("Vary Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    std::cout << "\n*******  end of DFAGen ReachingDefs analysis *********\n\n";

    return returnvalue;
}

int DoDFAGenUseful(SgProject *p, std::vector<SgNode*> *na, bool p_handle)
{
    int returnvalue=FALSE;
    OA::OA_ptr<SageIRProcIterator> procIter;

    if ( debug ) printf("*******start of DFAGen Useful analysis\n");

    double time = clock();

    // contruct IR
    OA::OA_ptr<SageIRInterface> irInterface;
    irInterface = new SageIRInterface(p, na, p_handle);
    printf("Construct IR Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    // CFG
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgmanstd;
    cfgmanstd = new OA::CFG::ManagerCFGStandard(irInterface);

    // eachCFG
    OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
    OA::OA_ptr<OA::CFG::ManagerCFGStandard> cfgman;
    cfgman = new OA::CFG::ManagerCFGStandard(irInterface);
    eachCFG = new OA::CFG::EachCFGStandard(cfgman);

    // force a CFG analysis on all procedures and time it
    time = clock();
    procIter = new SageIRProcIterator(p, *irInterface);
    int count = 0;
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::CFG::CFGInterface> cfg;
        cfg = eachCFG->getCFGResults(procIter->current());
        count++;
    }
    printf("CFG Time: %lf, %d procs analyzed\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC), count);

    //FIAlias
    OA::OA_ptr<OA::Alias::ManagerFIAliasAliasTag> fialiasman;
    fialiasman = new OA::Alias::ManagerFIAliasAliasTag(irInterface);
    procIter = new SageIRProcIterator(p, *irInterface);
    OA::OA_ptr<OA::Alias::Interface> alias;
    alias = fialiasman->performAnalysis(procIter);
    printf("FIAlias Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    //TODO: Need to create interprocedural alias to pass to side effects
    OA::OA_ptr<OA::Alias::InterAliasInterface> interAlias;

    // Interprocedural Side-Effect Analysis
    // for now generate default conservative interprocedural side-effect results
    OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
    interSideEffect = new OA::SideEffect::InterSideEffectStandard(interAlias);
    printf("Side-effects Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();
    
    // Construct reachdefs manager
    OA::OA_ptr<OA::Useful::ManagerUseful> varyMgr;
    varyMgr = new OA::Useful::ManagerUseful(irInterface);

    // Iterate through each proc doing reaching definitions analysis
    procIter = new SageIRProcIterator(p, *irInterface);
    for(; procIter->isValid(); ++(*procIter)) {
        OA::OA_ptr<OA::Useful::Useful> results;
        results = varyMgr->performAnalysis(
            procIter->current(),
            eachCFG->getCFGResults(procIter->current()),
            alias,
            interSideEffect); 

        if(!silent) { results->output(*irInterface); }
    }
    printf("Useful Time: %lf\n",
        (clock() - time) / (1.0 * CLOCKS_PER_SEC));
    time = clock();

    std::cout << "\n*******  end of DFAGen ReachingDefs analysis *********\n\n";

    return returnvalue;
}


