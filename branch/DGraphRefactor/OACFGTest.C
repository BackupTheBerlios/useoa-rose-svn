/*
  CFGTest.C

  This is a test driver that calls the OA CFG generator and then 
  prints text output of the CFG.

  12/17/03 MMS: copied from ROSE/ProgramAnalysis/CFG/CFGTest.C 
                also copied Beata's text output from DoOpenAnalysis.cpp
  9/2/04 MMS: copied from adic-2.0/xaif
*/
#ifdef HAVE_CONFIG_H
//#include <config.h>
#endif

#include <rose.h>
#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include <OpenAnalysis/CFG/ManagerCFG.hpp>
//#include "SageAttr.h"  // needed for findSymbolFromStmt

#include <string>
#include <iostream>

using namespace std;

int DoOpenAnalysis(SgFunctionDefinition* f);


int
main ( unsigned argc,  char * argv[] )
{

  int frontEndErrorCode = 0;
  //SgProject sageProject ( (int)argc,argv,frontEndErrorCode);
  SgProject sageProject ( (int)argc,argv);
  assert(frontEndErrorCode <3);

  int filenum = sageProject.numberOfFiles();
  for (int i = 0; i < filenum; ++i) {
    SgFile &sageFile = sageProject.get_file(i);
    SgGlobal *root = sageFile.get_root();
    SgDeclarationStatementPtrList& declList = root->get_declarations ();
    for (SgDeclarationStatementPtrList::iterator p = declList.begin(); p != declList.end(); ++p) {
         SgFunctionDeclaration *func = isSgFunctionDeclaration(*p);
      if (func == 0)
        continue;
      SgFunctionDefinition *defn = func->get_definition();
      if (defn == 0)
        continue;
//          SgBasicBlock *stmts = defn->get_body();  
          
      // create a control flow graph and generate text output
      DoOpenAnalysis(defn);
      
    }     
  }

  return 0;
}

int DoOpenAnalysis(SgFunctionDefinition* f)
{
	int returnvalue=FALSE;
	printf("*******start of DoOpenAnalysis\n");
	OA::OA_ptr<SageIRInterface> irInterface; 

	bool p_h=FALSE; //for debugging only switch between persistent and "pointer" handles (pointers are faster, persistent are easier to debug
	
	std::vector<SgNode*> nodeArray;

	irInterface = new SageIRInterface(f, &nodeArray, p_h);
	irInterface->createNodeArray(f); //what about global vars?
	if(!f->get_body())
	{
		printf("forward declaration, will not create CFG\n");
		return FALSE;
	}

	//try
	//{
        // create CFG Manager and then CFG
        OA::OA_ptr<OA::CFG::ManagerStandard> cfgmanstd;
        cfgmanstd= new OA::CFG::ManagerStandard(irInterface);

        OA::OA_ptr<OA::CFG::CFG> cfg
          = cfgmanstd->performAnalysis((OA::irhandle_t)irInterface->getNodeNumber(f));
        cfg->dump(std::cout, irInterface);
	//}
	//catch(Exception &e)
	//{
//		printf("error in try\n");
		
//	}

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
		OA::OA_ptr<OA::CFG::CFG::Node> n = 
		  (nodeItPtr->current()).convert<OA::CFG::CFG::Node>();
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
	for (; edgeItPtr->isValid(); ++(*edgeItPtr)) {
      OA::OA_ptr<OA::CFG::CFG::Edge> e 
          = edgeItPtr->current().convert<OA::CFG::CFG::Edge>();
      OA::OA_ptr<OA::CFG::CFG::Node> n1 
          = e->source().convert<OA::CFG::CFG::Node>();
      OA::OA_ptr<OA::CFG::CFG::Node> n2 
          = e->sink().convert<OA::CFG::CFG::Node>();
      
      char tmpstr[100];
      sprintf(tmpstr, "<ControlFlowEdge source=\"%d\" target=\"%d\"/>", 
	      n1->getId(), n2->getId());
		cfgxaifout+="\n";
		cfgxaifout+=tmpstr;
    }

	cfgxaifout+="\n";
    cfgxaifout+= "</ControlFlowGraph>";
    printf("CFG is \n %s\n", cfgxaifout.c_str());
	
	std::cout << "\n*******  end of DoOpenAnalysis *********\n\n";
	return returnvalue;

}
