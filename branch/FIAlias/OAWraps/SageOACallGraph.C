#include "SageOACallGraph.h"


//SageIRCallsiteIterator implementation

SageIRCallsiteIterator::SageIRCallsiteIterator(SgStatement * sgstmt, 
                                               OA::OA_ptr<SageIRInterface> in)
{
 //given an sgstmt put all call expressions in calls_in_stmt
   FindCallsitesInSgStmt(sgstmt, calls_in_stmt);
  begin = calls_in_stmt.begin();
  st_iter = calls_in_stmt.begin();
  end = calls_in_stmt.end();
  valid=TRUE;
  ir=in;
}

void SageIRCallsiteIterator::FindCallsitesInSgStmt(SgStatement *sgstmt, SgExpressionPtrList& lst)
{
  //we only want Callsites in non-scope stmts, for scopes only in non-body
  if(!isSgScopeStatement(sgstmt))
    FindCallsitesPass(lst).traverse(sgstmt, preorder);
  else
  {
    SgCatchOptionStmt * co=NULL;
    SgDoWhileStmt * dw=NULL;
    SgForStatement *forst=NULL;
    SgIfStmt *ifst=NULL;
    SgWhileStmt * whst=NULL;
   //SgBasicBlock ->no call sites
    //SgCatchOptionStmt go to condition 
     if(co=isSgCatchOptionStmt(sgstmt))
       FindCallsitesPass(lst).traverse(co->get_condition(), preorder);
     //SgClassDefinition ->no call sites
     else if(dw=isSgDoWhileStmt(sgstmt))
       FindCallsitesPass(lst).traverse(dw->get_condition(), preorder);
     else if(forst=isSgForStatement(sgstmt))
     {
       
      FindCallsitesPass(lst).traverse(forst->get_test_expr(), preorder);
      FindCallsitesPass(lst).traverse(forst->get_increment_expr(), preorder);
      FindCallsitesPass(lst).traverse(forst->get_for_init_stmt(), preorder);
     }
     //SgFunctionDefinition ->no call sites
     //SgGlobal ->no call sites
     else if(ifst=isSgIfStmt(sgstmt))
     {
       FindCallsitesPass(lst).traverse(ifst->get_conditional(), preorder);
     }
     //SgNamespaceDefinition -> no call sites
     //SgSwitchStatement -> no call sites
     else if(whst=isSgWhileStmt(sgstmt))
     {
      FindCallsitesPass(lst).traverse(whst->get_condition(), preorder);
     }
     
  }
  
}

// Returns the current item.
OA::CallHandle SageIRCallsiteIterator::current() const
{
  SgExpression* cur = *st_iter;
  OA::CallHandle h = 0;

  if (isValid()) {
    //cerr << "cur expr: " << cur->unparseToString() << endl;
    h = (OA::irhandle_t)(ir->getNodeNumber(cur));    
    //cerr << "handle : " << h.hval() << endl;
  }
  return h;
}

void SageIRCallsiteIterator::operator++()
{
  if(valid)
  {
    st_iter++;
  }
}

void SageIRCallsiteIterator::reset()
{
  st_iter = begin;
}


//SageIRProcIterator implementation

void SageIRProcIterator::FindProcsInSgTree(SgNode *node, SgStatementPtrList& lst)
{
  FindProcsPass(lst).traverse(node, preorder);
}

SageIRProcIterator::SageIRProcIterator(SgNode *node, 
                                       OA::OA_ptr<SageIRInterface> in)
{
 //given an sgstmt put all call expressions in calls_in_stmt
   FindProcsInSgTree(node, procs_in_proj);
  begin = procs_in_proj.begin();
  st_iter = procs_in_proj.begin();
  end = procs_in_proj.end();
  valid=TRUE;
  ir=in;
}


// Returns the current item.
OA::ProcHandle SageIRProcIterator::current() const
{
  SgStatement* cur = *st_iter;
  OA::ProcHandle h = 0;

  if (isValid()) {
    //cerr << "cur stmt: " << cur->unparseToString() << endl;
    h = (OA::irhandle_t)(ir->getNodeNumber(cur));    
    //cerr << "handle : " << h.hval() << endl;
  }
  return h;
}

void SageIRProcIterator::operator++()
{
  if(valid)
  {
    st_iter++;
  }
}

void SageIRProcIterator::reset()
{
  st_iter = begin;
}


void FindCallsitesPass::visit(SgNode* node)
{
  if(!node) { printf("visited with 0-Node!\n"); return; }
  
  // We expect to have expressions in the call_lst.
  SgExpression *exp = isSgExpression(node);
  if ( exp == NULL )
    return;

  // Though a method is represented as a function call, an
  // invocation of new is not.  Need to check for it 
  // explicitly.
  if( isSgFunctionCallExp(exp) || isSgNewExp(exp) )
    call_lst.push_back(exp);

  return;
}



void FindProcsPass::visit(SgNode* node)
{
  if(!node) { printf("visited with 0-Node!\n"); return; }
	
#if 1
  if(isSgFunctionDefinition(node))
#else
  if(isSgFunctionDeclaration(node))
#endif
    proc_lst.push_back(isSgStatement(node));
  return;
}

