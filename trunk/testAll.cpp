/*
  testAll.cpp

  This is a test driver that calls various OA analyses  depending on the command line option

 */
 
 /*
  {  0 , "oa-CFG",            CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-MemRefExpr",     CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-Alias",          CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasMap",       CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-CallGraph",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ReachDefs",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-UDDUChains",     CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-UDDUChainsXAIF", CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-MPICFG",         CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-ReachConsts",      CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  {  0 , "oa-AliasMapXAIF",       CLP::ARG_NONE, CLP::DUPOPT_ERR,  NULL },
  */
  

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <rose.h>
#include <OpenAnalysis/UDDUChains/ManagerUDDUChainsStandard.hpp>
#include <OpenAnalysis/ReachDefs/ManagerReachDefsStandard.hpp>
#include <OpenAnalysis/Alias/ManagerAliasMapBasic.hpp>
#include <OpenAnalysis/CFG/ManagerCFGStandard.hpp>
#include <OpenAnalysis/CallGraph/ManagerCallGraphStandard.hpp>
#include <OpenAnalysis/MemRefExpr/MemRefExpr.hpp>
#include <OpenAnalysis/SideEffect/InterSideEffectStandard.hpp>
#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
//#include "SageAttr.h"  // needed for findSymbolFromStmt

#include <string>
#include <iostream>

#ifdef DEBUG_testAll
    bool debug = true;
#else
    bool debug = false;
#endif

int DoOpenAnalysis(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
int DoAlias(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
int DoCallGraph(SgProject * sgproject, std::vector<SgNode*> * na, bool persistent_h);
int DoUDDUChains(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool persistent_h);
void OutputMemRefInfo(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt);



int
main ( unsigned argc,  char * argv[] )
{
  
  bool p_h=FALSE; //for debugging only switch between persistent and "pointer" handles (pointers are faster, persistent are easier to debug

  std::vector<SgNode*> nodeArray;

  //figure out which analysis to do based on the first command line arg
  if(argc<3)
  {
    printf("need to specify oa option and file name \n");
    return 1;
  }
  else
  {
    SgProject * sageProject =frontend( (int)(argc-1),&argv[1]);
    int filenum = sageProject->numberOfFiles();
    string oaopt=argv[1];
    if(oaopt=="--oa-CFG")
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
          return 0;
    }
    else if(oaopt=="--oa-MemRefExpr")
    {
      //printf("TO DO, implement mem ref expr analysis\n");
      OA::OA_ptr<SageIRInterface> ir; 
      ir = new SageIRInterface(sageProject, &nodeArray, p_h);
      for (int i = 0; i < filenum; ++i) 
      {
          SgFile &sageFile = sageProject->get_file(i);
          SgGlobal *root = sageFile.get_root();
          ir->createNodeArray(root);
          SgDeclarationStatementPtrList& declList = root->get_declarations ();
          for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p) 
          {
              SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
              if (func == 0)
                continue;
              SgFunctionDefinition *defn = func->get_definition();
              if (defn == 0)
                continue;
              OA::ProcHandle proc((OA::irhandle_t)(ir->getNodeNumber(defn)));
              OA::OA_ptr<OA::IRStmtIterator> sIt = ir->getStmtIterator(proc);
              for ( ; sIt->isValid(); (*sIt)++) 
              {
                OA::StmtHandle stmt = sIt->current();
                OutputMemRefInfo(ir, stmt);
              }

          }     
      }
      
      
      
      return 0;
    }
    else if(oaopt=="--oa-AliasMap")
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
          return 0;
    }
    /*else if(oaopt=="--oa-AliasMap")
    {
      printf("TO DO, implement alias map analysis\n");
      return 1;
    }*/
    else if(oaopt=="--oa-CallGraph")
    {
       DoCallGraph(sageProject, &nodeArray, p_h);
      return 1;
    }
    else if(oaopt=="--oa-ReachDefs")
    {
      printf("TO DO, implement reach. def. analysis\n");
      return 1;
    }
    else if(oaopt=="--oa-UDDUChains")
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
      return 0;
    }
    else if(oaopt=="--oa-UDDUChainsXAIF")
    {
      printf("TO DO, implement UDDUChainsXAIF analysis\n");
      return 1;
    }
    else if(oaopt=="--oa-MPICFG")
    {
      printf("TO DO, implement MPICFG analysis\n");
      return 1;
    }
    else if(oaopt=="--oa-ReachConsts")
    {
      printf("TO DO, implement ReachConsts analysis\n");
      return 1;
    }
    else if(oaopt=="--oa-AliasMapXAIF")
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

   return 0;
}

int DoOpenAnalysis(SgFunctionDefinition* f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
	int returnvalue=FALSE;
	if (debug) { printf("*******start of DoOpenAnalysis\n"); }
	OA::OA_ptr<SageIRInterface> irInterface; 
    irInterface = new SageIRInterface(p, na, p_handle);
	if(!f->get_body())
	{
		if (debug) { 
                  printf("forward declaration, will not create CFG\n");
                }
		return FALSE;
	}
 
	//try
	//{
        // create CFG Manager and then CFG
        OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
        cfgmanstd= new OA::CFG::ManagerStandard(irInterface);
        OA::OA_ptr<OA::CFG::CFGStandard> cfg
          = cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
        cfg->dump(std::cout, irInterface);
	//}
	//catch(Exception &e)
	//{
//		printf("error in try\n");
		
//	}

	if (debug) { printf("*********\n**********  printing CFG\n***********\n************\n"); }//code mostly from wn2f.cxx
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
    if (debug) { printf("CFG is \n %s\n", cfgxaifout.c_str()); }
	
	std::cout << "\n*******  end of DoOpenAnalysis *********\n\n";
	return returnvalue;

}


int DoCallGraph(SgProject* sgproject, std::vector<SgNode*> * na, bool p_handle)
{
	int returnvalue=FALSE;
	if (debug) { printf("*******start of DoCallGraph\n"); }
	SageIRInterface * irInterface; 
  OA::OA_ptr<SageIRInterface> iroaptr;
  irInterface = new SageIRInterface(sgproject, na, p_handle);
  //irInterface->createNodeArray(sgproject); //what about global vars?

	//try
	//{
        // create CallGraph Manager and then Call Graph
        OA::OA_ptr<OA::CallGraph::ManagerStandard> callgraphmanstd;
        iroaptr=irInterface;
        callgraphmanstd= new OA::CallGraph::ManagerStandard(iroaptr);
        OA::OA_ptr<SageIRProcIterator> procIter;
        procIter = new SageIRProcIterator(sgproject, irInterface);

        OA::OA_ptr<OA::CallGraph::CallGraphStandard> callgraph
          = callgraphmanstd->performAnalysis(procIter);
        callgraph->dump(std::cout, iroaptr);
	//}
	//catch(Exception &e)
	//{
//		printf("error in try\n");
		
//	}


	
	std::cout << "\n*******  end of DoCallGraph *********\n\n";
	return returnvalue;

}

int DoAlias(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
	int returnvalue=FALSE;
	if (debug) { printf("*******start of DoCallGraph\n"); }
	SageIRInterface * irInterface; 
  OA::OA_ptr<SageIRInterface> iroaptr;
  irInterface = new SageIRInterface(p, na, p_handle);

	//try
	//{
        // create CallGraph Manager and then Call Graph
        OA::OA_ptr<OA::Alias::ManagerAliasMapBasic> aliasmanstd;
        iroaptr=irInterface;
        aliasmanstd= new OA::Alias::ManagerAliasMapBasic(iroaptr);
        //OA::OA_ptr<SageIRProcIterator> procIter;
        //procIter = new SageIRProcIterator(sgproject, irInterface);

        //OA::OA_ptr<OA::CallGraph::CallGraphStandard> callgraph
         // = callgraphmanstd->performAnalysis(procIter);
        
        //callgraph->dump(std::cout, iroaptr);
        OA::OA_ptr<OA::Alias::AliasMap> alias = 
      aliasmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
      alias->dump(std::cout, iroaptr);

	//}
	//catch(Exception &e)
	//{
//		printf("error in try\n");
		
//	}


	
	std::cout << "\n*******  end of DoAlias *********\n\n";
	return returnvalue;

}

int DoUDDUChains(SgFunctionDefinition * f, SgProject * p, std::vector<SgNode*> * na, bool p_handle)
{
	int returnvalue=FALSE;
	printf("*******start of DoUDDUChains\n");
	SageIRInterface * irInterface; 
  OA::OA_ptr<SageIRInterface> iroaptr;
  irInterface = new SageIRInterface(p, na, p_handle);
  //irInterface->createNodeArray(f); //what about global vars? //done in constr.
  iroaptr=irInterface;
  // CFG
  OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
  cfgmanstd = new OA::CFG::ManagerStandard(iroaptr);
  OA::OA_ptr<OA::CFG::Interface> cfg=
     cfgmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  
  //AliasMap
  OA::OA_ptr<OA::Alias::ManagerAliasMapBasic> aliasmanstd;
  iroaptr=irInterface;
  aliasmanstd= new OA::Alias::ManagerAliasMapBasic(iroaptr);
  OA::OA_ptr<OA::Alias::AliasMap> alias = 
  aliasmanstd->performAnalysis((OA::irhandle_t)(irInterface->getNodeNumber(f)));
  //alias->dump(std::cout, iroaptr);
  
  // Interprocedural Side-Effect Analysis
  // for now generate default conservative interprocedural side-effect results
  OA::OA_ptr<OA::SideEffect::InterSideEffectInterface> interSideEffect;
  interSideEffect = new OA::SideEffect::InterSideEffectStandard;

  // then can do ReachDefs
  OA::OA_ptr<OA::ReachDefs::ManagerStandard> rdman;
  rdman = new OA::ReachDefs::ManagerStandard(iroaptr);
  OA::OA_ptr<OA::ReachDefs::ReachDefsStandard> rds= 
      rdman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),cfg,alias,interSideEffect);


  // then UDDUChains
  OA::OA_ptr<OA::UDDUChains::ManagerStandard> udman;
  udman = new OA::UDDUChains::ManagerStandard(iroaptr);
  OA::OA_ptr<OA::UDDUChains::UDDUChainsStandard> udduchains= 
      udman->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f),alias,rds,interSideEffect);

  udduchains->dump(std::cout, iroaptr);


  
  
	std::cout << "\n*******  end of DoUDDUChains *********\n\n";
	return returnvalue;
}


void OutputMemRefInfo(OA::OA_ptr<SageIRInterface> ir, OA::StmtHandle stmt)
{
  //adapted from Nathan's code --should be specific IR independent only OA interface used
    std::cout << "============================================" << std::endl;
    std::cout << "OA::StmtHandle: " << ir->toString(stmt);
    //ir->dump(stmt, std::cout);
    //std::cout << "AliasStmtType: " 
//	      << ir->toString(ir->getAliasStmtType(stmt)) << std::endl;
    std::cout << std::endl;
    
    OA::OA_ptr<OA::MemRefHandleIterator> mrIt = ir->getAllMemRefs(stmt);
    for ( ; mrIt->isValid(); (*mrIt)++) 
    {
      OA::MemRefHandle memref = mrIt->current();
      
      // print out the debugging information to give some context
      if (debug) { std::cout << "  ***** OA::MemRefHandle: "; 
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
        mre->output(ir);
        std::cout << std::endl;
      }
      //printf("outside inner loop\n");
    }
    //printf("outside outer loop\n"); 
}
