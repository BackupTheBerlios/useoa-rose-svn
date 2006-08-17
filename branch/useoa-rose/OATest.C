/*
// testAll.cpp

  This is a test driver that calls various OA analyses  depending on the command line option

 */
 
 /*
  {  0 , "oa-CFG",            CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-MemRefExpr",     CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-Alias",          CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasMap",       CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-CallGraph",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ICFG",           CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ICFGDep",        CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ReachDefs",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-UDDUChains",     CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-UDDUChainsXAIF", CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-MPICFG",         CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ReachConsts",    CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasMapXAIF",   CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },  */
  

#ifdef HAVE_CONFIG_H
//#include <config.h>
#endif

#include <rose.h>
#include <unistd.h>
#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include "debug.h"
#include <OpenAnalysis/Alias/ManagerAliasMapBasic.hpp>
#include <OpenAnalysis/Alias/ManagerFIAliasEquivSets.hpp>
#include <OpenAnalysis/Alias/ManagerFIAliasAliasMap.hpp>
#include <OpenAnalysis/CallGraph/ManagerCallGraphStandard.hpp>
#include <OpenAnalysis/CFG/ManagerCFGStandard.hpp>
#include <OpenAnalysis/CFG/EachCFGStandard.hpp>
#include <OpenAnalysis/DataFlow/ManagerParamBindings.hpp>
#include <OpenAnalysis/ICFG/ManagerICFGStandard.hpp>
#include <OpenAnalysis/Activity/ManagerICFGDep.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/ReachDefs/ManagerReachDefsStandard.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
#include <OpenAnalysis/Utils/OutputBuilderDOT.hpp>
#include <OpenAnalysis/Utils/Util.hpp>
#include <OpenAnalysis/ReachConsts/ManagerReachConstsStandard.hpp>
//#include "SageAttr.h"  // needed for findSymbolFromStmt

#include <string>
#include <iostream>
#include <sstream>
using namespace std;
#include <CommandOptions.h>

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

int DoOpenAnalysis(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
int DoAlias(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
int DoFIAliasEquivSets(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoFIAliasAliasMap(SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoCallGraph(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoICFG(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoICFGDep(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoParamBinding(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle);
int DoUDDUChains(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
void OutputMemRefInfo(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt);
void OutputMemRefInfoNoPointers(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt);
int DoReachDef(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int Foo(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
int DoSideEffect(SgProject* sgproject, std::vector<SgNode*> *na, bool p_handle);

void prettyPrintMemRefExp(OA::OA_ptr<OA::MemRefExpr> memRefExp, 
			  SageIRInterface *ir,
			  std::ostream &os);

int DoReachConsts(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle);
    

/* Debug flags:

    turn this on:      If you:
    debug               Want debugging output.
    outputRose          
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
bool exitWithTop = false;
bool skipAnalysis = false;
bool silent = false;

void readDebuggingFlags() {
    USEOA_DEBUG_CTRL_MACRO("DEBUG",         debug);
    USEOA_DEBUG_CTRL_MACRO("OUTPUT_ROSE",   outputRose);
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
  cerr << "          --exitWithTop" << endl;
  cerr << "          --skipAnalysis" << endl;
  cerr << "          --silent" << endl;
  cerr << "     where opt is one of:" << endl;
  cerr << "          --oa-CFG" << endl;
  cerr << "          --oa-MemRefExpr" << endl;
  cerr << "          --oa-FIAliasEquivSets" << endl;
  cerr << "          --oa-FIAliasAliasMap" << endl;
  cerr << "          --oa-AliasMap" << endl;
  cerr << "          --oa-CallGraph" << endl;
  cerr << "          --oa-ICFG" << endl;
  cerr << "          --oa-ICFGDep" << endl;
  cerr << "          --oa-ParamBindings" << endl;
  cerr << "          --oa-ReachDefs" << endl;
  cerr << "          --oa-UDDUChains" << endl;
  cerr << "          --oa-UDDUChainsXAIF" << endl;
  cerr << "          --oa-MPICFG" << endl;
  cerr << "          --oa-ReachConsts" << endl;
  cerr << "          --oa-AliasMapXAIF" << endl;
  cerr << "          --oa-SideEffect" << endl;
  exit(-1);
}

int
main ( unsigned argc,  char * argv[] )
{
  readDebuggingFlags();

  bool p_h=FALSE; //for debugging only switch between persistent and "pointer" handles (pointers are faster, persistent are easier to debug

  //  p_h = TRUE;
  
  std::vector<SgNode*> nodeArray;

  // Figure out which analysis to do based on command-line args.
  // Now we use CmdOptions and don't require the analysis option
  // to be the first specified.
  if(argc<3)
  {
    usage(argv);
  }
  else
  {
    SgProject * sageProject =frontend( (int)(argc-1),&argv[1]);
    int filenum = sageProject->numberOfFiles();

    // debug info
    //AstPDFGeneration pdftest;
    //pdftest.generateInputFiles(sageProject);
    //AstDOTGeneration dottest;
    //dottest.generateInputFiles(sageProject);

    CmdOptions *cmds = CmdOptions::GetInstance();
    cmds->SetOptions(argc, argv);

    // debug flags, these can alternatively be set with the USEOA_DEBUG environmental
    // variable.
    if ( cmds->HasOption("--debug") )        { debug = true; }
    if ( cmds->HasOption("--outputRose") )   { outputRose = true; }
    if ( cmds->HasOption("--exitWithTop") )  { exitWithTop = true; }
    if ( cmds->HasOption("--skipAnalysis") ) { skipAnalysis = true; }
    if ( cmds->HasOption("--silent") )       { silent = true; }
    
    if( cmds->HasOption("--oa-CFG") )
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
                DoOpenAnalysis(defn, sageProject, &nodeArray, p_h);
              }     
          }
    }
    else if( cmds->HasOption("--oa-MemRefExpr") )
    {
      //printf("TO DO, implement mem ref expr analysis\n");
      OA::OA_ptr<SageIRInterface> ir; 
      ir = new SageIRInterface(sageProject, &nodeArray, p_h);
      for (int i = 0; i < filenum; ++i) 
      {
          SgFile &sageFile = sageProject->get_file(i);
          SgGlobal *root = sageFile.get_root();
	  char *fileName = sageFile.getFileName();
	  ROSE_ASSERT(fileName != NULL);
          ir->createNodeArray(root);

	  list<SgNode *> nodes = NodeQuery::querySubTree(root,
							 V_SgFunctionDefinition);
	  for (list<SgNode *>::iterator it = nodes.begin();
	       it != nodes.end(); ++it ) {
	    
	    SgNode *n = *it;
	    ROSE_ASSERT(n != NULL);
	    
	    SgFunctionDefinition *defn = isSgFunctionDefinition(n);
	    ROSE_ASSERT(defn != NULL);

#if 1
	      if (!defn->get_file_info())
		continue;
	      // Don't output junk we pull in from header files.
	      if (!ROSE::isSameName(fileName,
				    (defn->get_file_info())->get_filename()))
		continue;
#endif
              OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(defn)));
              OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
              for ( ; sIt->isValid(); (*sIt)++) 
              {
                OA::StmtHandle stmt = sIt->current();
		if ( outputRose )
		  OutputMemRefInfoNoPointers(ir, stmt);
		else
		  OutputMemRefInfo(ir, stmt);
              }

          }     
      }
      
      
      
    }
    else if( ( cmds->HasOption("--oa-FIAliasEquivSets") ) ||
	     ( cmds->HasOption("--oa-FIAlias") ) )
    {
      DoFIAliasEquivSets(sageProject, &nodeArray, p_h);
      //      DoFIAliasAliasMap(sageProject, &nodeArray, p_h);
      //      DoFIAlias(sageProject, &nodeArray, FALSE);
    }
    else if( cmds->HasOption("--oa-FIAliasAliasMap") )
    {
      DoFIAliasAliasMap(sageProject, &nodeArray, p_h);
    }
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
    /*else if( cmds->HasOption("--oa-AliasMap") )
    {
      printf("TO DO, implement alias map analysis\n");
      return 1;
    }*/
    else if( cmds->HasOption("--oa-CallGraph") )
    {
       DoCallGraph(sageProject, &nodeArray, p_h);
      return 1;
    }
    else if( cmds->HasOption("--oa-ICFG") )
    {
       DoICFG(sageProject, &nodeArray, p_h);
      return 1;
    }
    else if( cmds->HasOption("--oa-ICFGDep") )
    {
       DoICFGDep(sageProject, &nodeArray, p_h);
      return 1;
    }
    else if( cmds->HasOption("--oa-ParamBindings") )
    {
      DoParamBinding(sageProject, &nodeArray, p_h);
       return 1;
    }
    else if( cmds->HasOption("--oa-SideEffect") )
    {
      DoSideEffect(sageProject, &nodeArray, p_h);
      return 1;
		   
    }
    else if( cmds->HasOption("--oa-ReachDefs") )
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
               DoReachDef(defn, sageProject, &nodeArray, p_h);
             }     
	 } 
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
    else if( cmds->HasOption("--oa-UDDUChainsXAIF") )
    {
      printf("TO DO, implement UDDUChainsXAIF analysis\n");
      return 1;
    }
    else if( cmds->HasOption("--oa-MPICFG") )
    {
      printf("TO DO, implement MPICFG analysis\n");
      return 1;
    }
    else if( cmds->HasOption("--oa-ReachConsts") )
    {
        
    //  printf("TO DO, implement ReachConsts analysis\n");
     /* for (int i = 0; i < filenum; ++i)
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
                                                                                             DoReachConsts(defn, sageProject, &nodeArray, p_h);
                                                                                           }
                                                                                        }
       */
        return 1;
    }
    else if( cmds->HasOption("--oa-AliasMapXAIF") )
    {
      printf("TO DO, implement AliasMapXAIF analysis\n");
      return 1;
    }
    else
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

int DoOpenAnalysis(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
	int returnvalue=FALSE;
	if ( debug )
	  printf("*******start of DoOpenAnalysis\n");
	OA::OA_ptr<SageIRInterface> irInterface; 
        irInterface = new SageIRInterface(p, na, p_handle);
	if(!f->get_body())
	{
                if ( debug ) 
                    printf("forward declaration, will not create CFG\n");
		return FALSE;
	}
 
	//try
	//{
        // create CFG Manager and then CFG
        OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
        cfgmanstd= new OA::CFG::ManagerStandard(irInterface);
        OA::OA_ptr<OA::CFG::CFGStandard> cfg
          = cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
        //cfg->dump(std::cout, irInterface);
	//}
	//catch(Exception &e)
	//{
//		printf("error in try\n");
		
//	}

        // output CFG analysis using OutputBuilderDOT
        OA::OA_ptr<OA::OutputBuilderDOT> dotBuilder;
        dotBuilder = new OA::OutputBuilderDOT();
        cfg->configOutput(dotBuilder);
        cfg->output(*irInterface);

        if ( debug ) {
	    printf("*********\n**********  printing CFG\n***********\n************\n"); //code mostly from wn2f.cxx
	string cfgxaifout;
	cfgxaifout+="<ControlFlowGraph subroutine_name=\"";
	//SgName strname=((SgSymbol*)(cfg->subprog_name()))->get_name(); //"no longer available"
	//cfgxaifout+=strname.str();
	cfgxaifout+="\"/> \n";

    OA::OA_ptr<OA::DGraph::Interface::NodesIterator> nodeItPtr 
        = cfg->getNodesIterator();
	for (; nodeItPtr->isValid(); ++(*nodeItPtr)) 
	{
		OA::OA_ptr<OA::CFG::CFGStandard::Node> n = 
		  (nodeItPtr->current()).convert<OA::CFG::CFGStandard::Node>();
		//FIXME: n->longdump(cfg, std::cerr); std::cerr << endl;
  
		// basic blocks
		cfgxaifout+="<ControlFlowVertex, vertex_id= ";
		char strid[100];
		sprintf(strid, "\"%d\"/>", n->getId());
		char newstrid[100];
		sprintf(newstrid, "\%d", n->getId());
		
		cfgxaifout+=strid;
		cfgxaifout+="\n";
		cfgxaifout+="<Statement List>\n";
		
    OA::OA_ptr<OA::CFG::Interface::NodeStatementsIterator> stmtItPtr
            = n->getNodeStatementsIterator();
      for (; stmtItPtr->isValid(); ++(*stmtItPtr)) 
      {
        //for now we just unparse SgStatement
        SgStatement * sgstmt=(SgStatement*)irInterface->getNodePtr(stmtItPtr->current());
        cfgxaifout+=sgstmt->unparseToString();
        cfgxaifout+="\n";
        
      }
		cfgxaifout+="</Statement List>\n";
		cfgxaifout+="</ControlFlowVertex>";
		cfgxaifout+="\n";
  }

    cfgxaifout+="\n";
    // output edges
    OA::OA_ptr<OA::DGraph::Interface::EdgesIterator> edgeItPtr 
        = cfg->getEdgesIterator();
	for (; edgeItPtr->isValid(); ++(*edgeItPtr)) 
    {
      OA::OA_ptr<OA::CFG::CFGStandard::Edge> e 
          = edgeItPtr->current().convert<OA::CFG::CFGStandard::Edge>();
      OA::OA_ptr<OA::CFG::CFGStandard::Node> n1 
          = e->source().convert<OA::CFG::CFGStandard::Node>();
      OA::OA_ptr<OA::CFG::CFGStandard::Node> n2 
          = e->sink().convert<OA::CFG::CFGStandard::Node>();
      
      char tmpstr[100];
      sprintf(tmpstr, "<ControlFlowEdge source=\"%d\" target=\"%d\"/>", 
	      n1->getId(), n2->getId());
      cfgxaifout+="\n";
      cfgxaifout+=tmpstr;
    }

	cfgxaifout+="\n";
    cfgxaifout+= "</ControlFlowGraph>";
    if ( debug )
       printf("CFG is \n %s\n", cfgxaifout.c_str());
	
	std::cout << "\n*******  end of DoOpenAnalysis *********\n\n";
	return returnvalue;
    }
}


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
    bool excludeInputFiles = true;
    // Don't pull in any procedures defined in input files.  For testing
    // purposes only:  avoids unexpected/spurious results due to 
    // stdlib.h, etc.
    procIter = new SageIRProcIterator(sgproject, 
                                      irInterface, excludeInputFiles);
    OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
    interAlias = fialiasman->performAnalysis(procIter);

    // create CallGraph Manager and then Call Graph
    OA::OA_ptr<OA::CallGraph::ManagerStandard> callgraphmanstd;
    callgraphmanstd= new OA::CallGraph::ManagerStandard(irInterface);
//    OA::OA_ptr<SageIRProcIterator> procIter;
//    procIter = new SageIRProcIterator(sgproject, irInterface);

    OA::OA_ptr<OA::CallGraph::CallGraphStandard> callgraph
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
  OA::OA_ptr<OA::CFG::ManagerStandard> cfgman;
  cfgman = new OA::CFG::ManagerStandard(irInterface);
  eachCFG = new OA::CFG::EachCFGStandard(cfgman);

  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  bool excludeInputFiles = true;
  // Don't pull in any procedures defined in input files.  For testing
  // purposes only:  avoids unexpected/spurious results due to 
  // stdlib.h, etc.
  procIter = new SageIRProcIterator(sgproject, 
                                    irInterface, excludeInputFiles);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);
  
  // create CallGraph Manager and then Call Graph
  OA::OA_ptr<OA::CallGraph::ManagerStandard> callgraphmanstd;
  callgraphmanstd= new OA::CallGraph::ManagerStandard(irInterface);
  //    OA::OA_ptr<SageIRProcIterator> procIter;
  //    procIter = new SageIRProcIterator(sgproject, irInterface);
  
  OA::OA_ptr<OA::CallGraph::CallGraphStandard> callgraph;
  callgraph = callgraphmanstd->performAnalysis(procIter,interAlias);
  
  // create ICFG Manager and then ICFG
    OA::OA_ptr<OA::ICFG::ICFGStandard> icfg;
    icfg = new OA::ICFG::ICFGStandard();
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

int DoICFGDep(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) 
    printf("*******start of DoICFGDep\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(sgproject, na, p_handle);
  //irInterface->createNodeArray(sgproject); //what about global vars?
  
  // eachCFG 
  OA::OA_ptr<OA::CFG::EachCFGInterface> eachCFG;
  OA::OA_ptr<OA::CFG::ManagerStandard> cfgman;
  cfgman = new OA::CFG::ManagerStandard(irInterface);
  eachCFG = new OA::CFG::EachCFGStandard(cfgman);

  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  bool excludeInputFiles = true;
  // Don't pull in any procedures defined in input files.  For testing
  // purposes only:  avoids unexpected/spurious results due to 
  // stdlib.h, etc.
  procIter = new SageIRProcIterator(sgproject, 
                                    irInterface, excludeInputFiles);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);
  
  // CallGraph
  OA::OA_ptr<OA::CallGraph::ManagerStandard> callgraphmanstd;
  callgraphmanstd= new OA::CallGraph::ManagerStandard(irInterface);
  OA::OA_ptr<OA::CallGraph::CallGraphStandard> cgraph;
  cgraph = callgraphmanstd->performAnalysis(procIter,interAlias);
  
  //ParamBindings
  OA::OA_ptr<OA::DataFlow::ManagerParamBindings> pbman;
  pbman = new OA::DataFlow::ManagerParamBindings(irInterface);
  OA::OA_ptr<OA::DataFlow::ParamBindings> parambind;
  parambind = pbman->performAnalysis(cgraph);

  // ICFG
  OA::OA_ptr<OA::ICFG::ManagerICFGStandard> icfgman;
  icfgman = new OA::ICFG::ManagerICFGStandard(irInterface);
  OA::OA_ptr<OA::ICFG::ICFGStandard> icfg;
  icfg = icfgman->performAnalysis(procIter,eachCFG,cgraph);
  
  //ICFGDep
  OA::OA_ptr<OA::Activity::ManagerICFGDep> icfgdepman;
  icfgdepman = new OA::Activity::ManagerICFGDep(irInterface);
  OA::OA_ptr<OA::Activity::ICFGDep> icfgDep;
  icfgDep = icfgdepman->performAnalysis(icfg, parambind, interAlias);

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
  
  icfgDep->output(*irInterface);
  
  // dump output
  //icfgDep->dump(std::cout,irInterface);
  
  
  std::cout << "\n*******  end of DoICFGDep *********\n\n";
  return returnvalue;
}


int DoSideEffect(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
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
  bool excludeInputFiles = true;
  procIter = new SageIRProcIterator(sgproject,irInterface, excludeInputFiles);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);

  OA::OA_ptr<OA::CallGraph::ManagerStandard> callgraphmanstd;
  callgraphmanstd= new OA::CallGraph::ManagerStandard(irInterface);
  OA::OA_ptr<OA::CallGraph::CallGraphStandard> callgraph
    = callgraphmanstd->performAnalysis(procIter,interAlias);
   

  
  OA::OA_ptr<OA::DataFlow::ManagerParamBindings> pbman;
  pbman = new OA::DataFlow::ManagerParamBindings(irInterface);
  OA::OA_ptr<OA::DataFlow::ParamBindings> parambind;
  parambind = pbman->performAnalysis(callgraph);
  //  parambind->dump(std::cout, irInterface);


  // Intra Side-Effect
  OA::OA_ptr<OA::SideEffect::ManagerStandard> sideeffectman;
  sideeffectman = new OA::SideEffect::ManagerStandard(irInterface);  

  // InterSideEffect
  OA::OA_ptr<OA::SideEffect::ManagerInterSideEffectStandard> interSEman;
  interSEman = new OA::SideEffect::ManagerInterSideEffectStandard(irInterface);

  OA::OA_ptr<OA::SideEffect::InterSideEffectStandard> interSE;
  interSE = interSEman->performAnalysis(callgraph, parambind, interAlias, sideeffectman);  

  interSE->output(*irInterface); 
  
  return returnvalue;

}


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
        bool excludeInputFiles = true;
        procIter = new SageIRProcIterator(sgproject,irInterface, excludeInputFiles);
        OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
        interAlias = fialiasman->performAnalysis(procIter);
		  
		  
        OA::OA_ptr<OA::CallGraph::ManagerStandard> callgraphmanstd;
        callgraphmanstd= new OA::CallGraph::ManagerStandard(irInterface);
        OA::OA_ptr<OA::CallGraph::CallGraphStandard> callgraph
	                          = callgraphmanstd->performAnalysis(procIter,interAlias);
		  
        OA::OA_ptr<OA::DataFlow::ManagerParamBindings> pbman;
        pbman = new OA::DataFlow::ManagerParamBindings(irInterface);
        OA::OA_ptr<OA::DataFlow::ParamBindings> parambind;
        parambind = pbman->performAnalysis(callgraph);
        parambind->output(*irInterface);

	
        return returnvalue;
 }
		                                        




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

int DoFIAliasEquivSets(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) printf("*******start of FIAlias\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);
  
  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasEquivSets> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasEquivSets(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  bool excludeInputFiles = true;
  // Don't pull in any procedures defined in input files.  For testing
  // purposes only:  avoids unexpected/spurious results due to 
  // stdlib.h, etc.
  procIter = new SageIRProcIterator(p, irInterface, excludeInputFiles);
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
}

int DoFIAliasAliasMap(SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) printf("*******start of FIAlias\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);
  
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
  bool excludeInputFiles = true;
  // Don't pull in any procedures defined in input files.  For testing
  // purposes only:  avoids unexpected/spurious results due to 
  // stdlib.h, etc.
  procIter = new SageIRProcIterator(p, irInterface, excludeInputFiles);
  //#define BRIAN_ADDED_DEBUG_PARAM_TO_PERFORMANALYSIS
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;

  if(!skipAnalysis) {
    interAlias = fialiasman->performAnalysis(procIter);
    if(!silent) { interAlias->output(*irInterface); }
  }
}

int DoReachDef(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) printf("*******start of DoUDDUChains\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);
  //irInterface->createNodeArray(f); //what about global vars? //done in constr.
  // CFG

  /*  OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
  cfgmanstd= new OA::CFG::ManagerStandard(irInterface);
  OA::OA_ptr<OA::CFG::CFGStandard> cfg
  = cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));*/


  OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
  cfgmanstd = new OA::CFG::ManagerStandard(irInterface);

  //********** this has the free error
  //OA::OA_ptr<OA::CFG::Interface> cfg=
  //   cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  //OA::DGraph::Interface& pDGraph = *cfg;
  //OA::OA_ptr<OA::DGraph::Interface::NodesIterator> nodeIter
  //            = pDGraph.getNodesIterator();

  //*********** this doesn't compile because have CFG::Interface
  //OA::OA_ptr<OA::CFG::Interface> cfg=
  //   cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  //OA::OA_ptr<OA::DGraph::Interface::NodesIterator> nodeIter
  //  = new OA::CFG::CFGStandard::NodesIterator(*cfg);

  //*********** this gets the same free error
  //OA::OA_ptr<OA::CFG::CFGStandard> cfg=
  //   cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  //OA::DGraph::Interface& pDGraph = *cfg;
  //OA::OA_ptr<OA::DGraph::Interface::NodesIterator> nodeIter
  //            = pDGraph.getNodesIterator();
  
  //*********** this gets the same free error
  OA::OA_ptr<OA::CFG::CFGStandard> cfg=
  cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f))); 

  /*  OA::OA_ptr<OA::OutputBuilderDOT> dotBuilder;
  dotBuilder = new OA::OutputBuilderDOT();
  cfg->configOutput(dotBuilder);
  cfg->output(*irInterface);*/



#if 0
  //AliasMap
  OA::OA_ptr<OA::Alias::ManagerAliasMapBasic> aliasmanstd;
  iroaptr=irInterface;
  aliasmanstd= new OA::Alias::ManagerAliasMapBasic(iroaptr);
  OA::OA_ptr<OA::Alias::AliasMap> alias = 
  aliasmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
#else
  /*
  //FIAlias
  OA::OA_ptr<OA::Alias::ManagerFIAliasEquivSets> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasEquivSets(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  procIter = new SageIRProcIterator(p, irInterface);
  OA::OA_ptr<OA::Alias::EquivSets> alias = 
    fialiasman->performAnalysis(procIter);
    */
#endif
//  alias->output(*irInterface); 

  OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
  fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
  OA::OA_ptr<SageIRProcIterator> procIter;
  bool excludeInputFiles = true;
  // Don't pull in any procedures defined in input files.  For testing
  // purposes only:  avoids unexpected/spurious results due to
  // stdlib.h, etc.
  procIter = new SageIRProcIterator(p,irInterface, excludeInputFiles);
  OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
  interAlias = fialiasman->performAnalysis(procIter);
  OA::ProcHandle proc((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  OA::OA_ptr<OA::Alias::Interface> alias = interAlias->getAliasResults(proc);
 
  
 
  // Interprocedural Side-Effect Analysis
  // for now generate default conservative interprocedural side-effect results
  OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
  interSideEffect = new OA::SideEffect::InterSideEffectStandard;

  
  // then can do ReachDefs
  OA::OA_ptr<OA::ReachDefs::ManagerStandard> rdman;
  rdman = new OA::ReachDefs::ManagerStandard(irInterface);
  OA::OA_ptr<OA::ReachDefs::ReachDefsStandard> rds= 
     rdman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),cfg,alias,interSideEffect); 
  
  rds->output(*irInterface);  

  //rds->dump(std::cout, irInterface);
 
	std::cout << "\n*******  end of DoReachDef *********\n\n";

	return returnvalue;

}

int DoReachConsts(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  /* int returnvalue=FALSE;
   if ( debug ) printf("*******start of DoUDDUChains\n");
   OA::OA_ptr<SageIRInterface> irInterface;
   irInterface = new SageIRInterface(p, na, p_handle);
           
    
   //CFG
   OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
   cfgmanstd = new OA::CFG::ManagerStandard(irInterface);
   OA::OA_ptr<OA::CFG::CFGStandard> cfg=
   cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));

   //Alias
   OA::OA_ptr<OA::Alias::ManagerFIAliasAliasMap> fialiasman;
   fialiasman= new OA::Alias::ManagerFIAliasAliasMap(irInterface);
   OA::OA_ptr<SageIRProcIterator> procIter;
   bool excludeInputFiles = true;
          
   procIter = new SageIRProcIterator(p,irInterface, excludeInputFiles);
   OA::OA_ptr<OA::Alias::InterAliasMap> interAlias;
   interAlias = fialiasman->performAnalysis(procIter);
   OA::ProcHandle proc((OA::irhandle_t)(irInterface->getNodeNumber(f)));
   OA::OA_ptr<OA::Alias::Interface> alias = interAlias->getAliasResults(proc);

   // Interprocedural Side-Effect Analysis
   // for now generate default conservative interprocedural side-effect results
   OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
   interSideEffect = new OA::SideEffect::InterSideEffectStandard;
        
   
   //ReachConsts        
   OA::OA_ptr<OA::ReachConsts::ManagerStandard> rcman;
   rcman = new OA::ReachConsts::ManagerStandard(irInterface);
   OA::OA_ptr<OA::ReachConsts::ReachConstsStandard> reachConsts=
          rcman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),cfg,alias,interSideEffect); 
   
   std::cout << "\n*******  end of DoUDDUChains *********\n\n";
   return returnvalue;
    */  
}


int DoUDDUChains(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
  int returnvalue=FALSE;
  if ( debug ) printf("*******start of DoUDDUChains\n");
  OA::OA_ptr<SageIRInterface> irInterface;
  irInterface = new SageIRInterface(p, na, p_handle);
  //irInterface->createNodeArray(f); //what about global vars? //done in constr.
  // CFG
  OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
  cfgmanstd = new OA::CFG::ManagerStandard(irInterface);
  OA::OA_ptr<OA::CFG::Interface> cfg=
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
  OA::OA_ptr<OA::ReachDefs::ManagerStandard> rdman;
  rdman = new OA::ReachDefs::ManagerStandard(irInterface);
  OA::OA_ptr<OA::ReachDefs::ReachDefsStandard> rds= 
      rdman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),cfg,alias,interSideEffect);


  // then UDDUChains
  OA::OA_ptr<OA::UDDUChains::ManagerStandard> udman;
  udman = new OA::UDDUChains::ManagerStandard(irInterface);
  OA::OA_ptr<OA::UDDUChains::UDDUChainsStandard> udduchains= 
      udman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),alias,rds,interSideEffect);

  udduchains->dump(std::cout, irInterface);


  
  
	std::cout << "\n*******  end of DoUDDUChains *********\n\n";
	return returnvalue;
}

 
void OutputMemRefInfo(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt)
{
  //adapted from Nathan's code --should be specific IR independent only OA interface used
    std::cout << "============================================" << std::endl;
    std::cout << "OA::StmtHandle: " << ir->toString(stmt);
    //    ir->dump(stmt, std::cout);
    // std::cout << "AliasStmtType: " 
    //	      << ir->toString(ir->getAliasStmtType(stmt)) << std::endl;
    std::cout << std::endl;
    
    OA::OA_ptr<OA::MemRefHandleIterator> mrIt = ir->getAllMemRefs(stmt);
    for ( ; mrIt->isValid(); (*mrIt)++) 
    {
      OA::MemRefHandle memref = mrIt->current();
      if ( debug ) {
	// print out the debugging information to give some context
	std::cout << "  ***** OA::MemRefHandle: ";
	fflush(stdout);
	ir->dump(memref, std::cout);
      }

      // get and print the memory reference expressions for this handle
      //OA::OA_ptr<std::list<OA::MemRefExpr::MemRefExpr> > mreList 
       //                       = ir->getMemRefExprBasicList(memref);
      
      OA::OA_ptr<OA::MemRefExprIterator> mreIterPtr;
      mreIterPtr = ir->getMemRefExprIterator(memref);

      // for each mem-ref-expr associated with this memref
      for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
        //printf("inside inner loop\n");
        //fflush(stdout);
        OA::OA_ptr<OA::MemRefExpr::MemRefExpr> mre = mreIterPtr->current();
        std::cout << "\tmre = ";
	mre->output(*ir);
	//	mre->dump(std::cout, ir);
        std::cout << std::endl;
      }
      //printf("outside inner loop\n");
    }
    //printf("outside outer loop\n"); 
}

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


void dump(OA::OA_ptr<OA::MemRefExpr> memRefExp, SageIRInterface *ir,
	  std::ostream &os)
{
  refType type = getRefType(memRefExp);
  bool addressTaken = memRefExp->hasAddressTaken();
  bool fullAccuracy = memRefExp->hasFullAccuracy();
  int numDerefs = 0;
  
  dump(memRefExp, addressTaken, fullAccuracy, type, numDerefs, ir, os);

}


void prettyPrintNamedRef(OA::OA_ptr<OA::NamedRef> memRefExp, 
			 SageIRInterface *ir, std::ostream& os) 
{
  OA::SymHandle symHandle = memRefExp->getSymHandle();
  refType mrType = getRefType(memRefExp);

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Named Ref: addressTaken = F, fullAccuracy = full.
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

void prettyPrintUnnamedRef(OA::OA_ptr<OA::UnnamedRef> memRefExp, 
			   SageIRInterface *ir, std::ostream& os) 
{
  refType mrType = getRefType(memRefExp);

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Unnamed Ref: addressTaken = T, fullAccuracy = partial.
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
  os << ")";
}

void prettyPrintDeref(OA::OA_ptr<OA::Deref> memRefExp, 
		      SageIRInterface *ir, std::ostream& os) 
{
  int numDerefs = memRefExp->getNumDerefs();
  OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = memRefExp->getMemRefExpr();
  refType mrType = getRefType(memRefExp);

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Deref: addressTaken = F, fullAccuracy = partial.
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

void OutputMemRefInfoNoPointers(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt) 
{
  // Get all mem ref handles corresponding to USEs in this statement.
  std::cout << "USEs within statement: " << ir->toString(stmt) << std::endl;
  
  OA::OA_ptr<OA::MemRefHandleIterator> useIter;
  useIter = ir->getUseMemRefs(stmt);
  
  for ( ; useIter->isValid(); (*useIter)++ ) {
    
    OA::MemRefHandle memRefHandle = useIter->current();
    
    // Finally, get all of the mem ref expressions associated
    // with this memRefHandle.
    OA::OA_ptr<OA::MemRefExprIterator> memRefExprIterator = 
      ir->getMemRefExprIterator(memRefHandle);
    
    dump(memRefExprIterator, ir.operator->(), std::cout);
    
  }
  
  std::cout << std::endl;
  
  std::cout << "DEFs within statement: " << ir->toString(stmt) << std::endl;
  
  // Get all mem ref handles corresponding to DEFs in this statement.
  OA::OA_ptr<OA::MemRefHandleIterator> defIter;
  defIter = ir->getDefMemRefs(stmt);
  
  for ( ; defIter->isValid(); (*defIter)++ ) {
    
    OA::MemRefHandle memRefHandle = defIter->current();
    
    // Finally, get all of the mem ref expressions associated
    // with this memRefHandle.
    OA::OA_ptr<OA::MemRefExprIterator> memRefExprIterator = 
      ir->getMemRefExprIterator(memRefHandle);
    
    dump(memRefExprIterator, ir.operator->(), std::cout);
    
  }
  
  std::cout << std::endl;
  
}

