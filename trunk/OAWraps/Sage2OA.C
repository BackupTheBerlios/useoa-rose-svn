#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
 
using namespace std;

//########################################################
// Iterators
//########################################################

//std::vector<SgNode*> SageIRInterface::nodeArray;

//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------
SageIRInterface::SageIRInterface(SgNode *root, 
				 std::vector<SgNode*> *na, 
				 bool use_persistent_handles)
  : nodeArrayPtr(na), wholeProject(root), 
    persistent_handles(use_persistent_handles)
{ 
  if ( persistent_handles ) { 
    ROSE_ASSERT(nodeArrayPtr != NULL); 
    createNodeArray(root);
  } 

  // Initialize the list of function declarations in the program.
  // We want declarations with an associated SgFunctionDefinition,
  // therefore lookup the definitions and find its declaration.
  list<SgNode *> nodes = NodeQuery::querySubTree(root,
						 V_SgFunctionDefinition);

  for (list<SgNode *>::iterator it = nodes.begin();
       it != nodes.end(); ++it ) {

    SgNode *n = *it;
    ROSE_ASSERT(n != NULL);
  
    SgFunctionDefinition *functionDefinition = 
      isSgFunctionDefinition(n);
    ROSE_ASSERT(functionDefinition != NULL);

    SgFunctionDeclaration *functionDeclaration =
      functionDefinition->get_declaration();
    ROSE_ASSERT(functionDeclaration != NULL);

    SgFunctionDeclaration *key = 
      isSgFunctionDeclaration(functionDeclaration->get_firstNondefiningDeclaration());
    if ( key == NULL )
      key = functionDeclaration;
    mFunctions[key] = functionDeclaration;
  }

} 
 
//-----------------------------------------------------------------------------
// Statement Iterator
//-----------------------------------------------------------------------------
void FindAllStmtsPass::visit(SgNode* node)
{
	if(!node) { printf("visited with 0-Node!\n"); return; }
  if(SgStatement * stmt=isSgStatement(node))
  {
    stmt_lst.push_back(stmt);
    string unp=stmt->unparseToString();
    //printf("statement is %s\n", unp.c_str());
  }
  return;
}


SageIRRegionStmtIterator::SageIRRegionStmtIterator(SgStatementPtrList & lst, SageIRInterface * in)
{
	st_iter=lst.begin();
	//begin = lst.begin();
	//end=lst.end();
  valid=TRUE;
  mTheList=&lst;
  mLength=lst.size();
  mIndex=0;
  ir=in;
}

OA::StmtHandle SageIRRegionStmtIterator::current() const {
  SgStatement* cur = *st_iter;
  OA::StmtHandle h = 0;

  if (isValid()) {
    //cerr << "cur stmt: " << cur->unparseToString() << endl;
    h = (OA::irhandle_t)(ir->getNodeNumber(cur));    
    //cerr << "handle : " << h.hval() << endl;
  }
  return h;
}

void SageIRRegionStmtIterator::operator++()
{
  if(isValid())
  {  
    st_iter++;
    mIndex++;    
  }
}

void SageIRRegionStmtIterator::reset()
{
  st_iter = mTheList->begin();
  mIndex=0;
}

/** \brief Return any global object declarations or class definitions in
 *         project.
 *  \param project  a SgNode representing the entire project.
 *  \param globals  on output holds the global declarations and definitions.
 */
void 
getGlobalObjectDeclarationsAndClassDefinitions(SgNode *project, 
					       std::vector<SgStatement *> &globals)
{
  SgProject *proj = isSgProject(project);
  if ( proj == NULL )
    return;

  // For each file in the project ...
  for (int i = 0; i < proj->numberOfFiles(); ++i) {
    SgFile &f = proj->get_file(i);

    // Get the root of its AST.
    SgGlobal *global = f.get_root();
    ROSE_ASSERT(global != NULL);
    
    // Retrieve any global object declarations or class definitions.
    vector<SgNode *> children = global->get_traversalSuccessorContainer();
    for (vector<SgNode *>::iterator it = children.begin();
	 it != children.end(); ++it) {

      SgNode *child = *it;
      ROSE_ASSERT(child != NULL);

      switch(child->variantT()) {
      case V_SgVariableDeclaration:
	{
	  SgVariableDeclaration *variableDeclaration =
	    isSgVariableDeclaration(child);
	  ROSE_ASSERT(variableDeclaration != NULL);

	  SgInitializedNamePtrList &variables =
	    variableDeclaration->get_variables();
	  SgInitializedNamePtrList::iterator varIter;
	  for (varIter = variables.begin(); 
	       varIter != variables.end(); ++varIter) {

	    SgNode *var = *varIter;
	    ROSE_ASSERT(var != NULL);

	    SgInitializedName *initName =
	      isSgInitializedName(var);
	    ROSE_ASSERT(initName != NULL);

	    // We only collect object instantiations, not
	    // declarations of points to objects.
	    if ( isSgClassType(initName->get_type()) ) {
	      SgStatement *stmt = isSgStatement(child);
	      ROSE_ASSERT(stmt != NULL);
	      globals.push_back(stmt);
	      break;
	    }

	  }
	  
	  break;
	}
      case V_SgClassDefinition:
	{
	  SgStatement *stmt = isSgStatement(child);
	  ROSE_ASSERT(stmt != NULL);
	  globals.push_back(stmt);
	  break;
	}
      default:
	{
	  break;
	}
      }
    }

  }

  
}

SageIRStmtIterator::SageIRStmtIterator(SgFunctionDefinition* node, 
				       SageIRInterface * in)
{
  if ( node == NULL ) {
    valid = FALSE;
    mIndex = 0;
    return;
  }

  // If the functionDefinition represents a constructor, add the
  // constructor initialization list which dangles off of its parent.
  SgNode *parent = node->get_parent();
  ROSE_ASSERT(parent != NULL);

  SgMemberFunctionDeclaration *memberFunctionDeclaration =
    isSgMemberFunctionDeclaration(node->get_declaration());
  if ( memberFunctionDeclaration != NULL ) {
    SgCtorInitializerList *initializerList =
      memberFunctionDeclaration->get_CtorInitializerList();
    if ( initializerList != NULL ) {
      //      ROSE_ASSERT(initializerList->get_parent() != NULL);
      //      cout << "pushing for " << node->unparseToCompleteString() << endl;
      all_stmts.push_back(initializerList);
    }
  }

  // If the functionDefinition is for main, include all of the 
  // global _object_ declarations and class definitions.  Why?  
  // Because alias analysis (and presumably others) will need to visit 
  // these to create implicit ptr assign.  However, these analyses
  // take an iterator over procs, which would exclude the global-level
  // statements.  
  // NB:  the intent is _not_ that these will be considered local to
  //      main.
  SgFunctionDeclaration *functionDeclaration =
    node->get_declaration();
  ROSE_ASSERT(functionDeclaration != NULL);
  if ( !strcmp(functionDeclaration->get_name().str(), "main") ) {
    std::vector<SgStatement *> globals;
    getGlobalObjectDeclarationsAndClassDefinitions(in->getProject(), globals);
    for (std::vector<SgStatement *>::iterator it = globals.begin();
	 it != globals.end(); ++it) {
      SgStatement *n = *it;
      all_stmts.push_back(n);
    }
  }

  //given an sgstmt put all call expressions in calls_in_stmt
  FindAllStmts(node, all_stmts);
  //begin = all_stmts.begin();
  st_iter = all_stmts.begin();
  //end = all_stmts.end();
  valid=TRUE;
  mIndex=0;
  mLength=all_stmts.size();
  ir=in;
}

OA::StmtHandle SageIRStmtIterator::current() const {
  SgStatement* cur = *st_iter;
  OA::StmtHandle h = 0;

  if (isValid()) {
    //cerr << "cur stmt: " << cur->unparseToString() << endl;
    h = (OA::irhandle_t)(ir->getNodeNumber(cur));    
    //cerr << "handle : " << h.hval() << endl;
  }
  return h;
}

void SageIRStmtIterator::operator++()
{
  if(isValid())
  {  
    st_iter++; 
    mIndex++;
    if(mIndex<mLength)
      valid=TRUE;
    else 
      valid=FALSE;
  }
}

void SageIRStmtIterator::reset()
{
  st_iter = all_stmts.begin();
  mIndex=0;
}

void SageIRStmtIterator::FindAllStmts(SgNode * node, SgStatementPtrList& lst)
{
  //printf("in SageIRStmtIterator::FindAllStmts\n");
  //FindAllStmtsPass(lst).traverse(node, preorder);
  //for scope stmt add it to the list and traverse the body,
  //else
  //  add stmt to the list
  SgStatement * stmt=NULL;
  SgCatchOptionStmt * co=NULL;
  SgDoWhileStmt * dw=NULL;
  SgForStatement *forst=NULL;
  SgIfStmt *ifst=NULL;

  SgWhileStmt * whst=NULL;
  SgClassDefinition * cldef=NULL;
  SgFunctionDefinition * fdef=NULL;
  SgGlobal *sggl=NULL;
  SgNamespaceDefinitionStatement *nmdef=NULL;
  SgSwitchStatement *switchStmt = NULL;
  SgCaseOptionStmt *caseOptionStmt = NULL;

  
  if(isSgScopeStatement(node))
  {
    lst.push_back(isSgStatement(node));
    SgBasicBlock * bb=isSgBasicBlock(node);
    
    if(bb)
    {
      SgStatementPtrList & bblist=bb->get_statements();
      //call FindAllStmts for each stmt on the bblist
      for(SgStatementPtrList::iterator iter=bblist.begin();
                              iter!=bblist.end(); iter++)
      {
        SgStatement *st=*iter;
        FindAllStmts(st, lst);
      }
    }
    else if( ( switchStmt = isSgSwitchStatement(node) ) != NULL ) 
    {
      FindAllStmts(switchStmt->get_item_selector(), lst);
      FindAllStmts(switchStmt->get_body(), lst);
    }
    else if( ( co=isSgCatchOptionStmt(node) ) != NULL )
    {
      FindAllStmts(co->get_body(), lst);
    }
    else if( ( dw=isSgDoWhileStmt(node) ) != NULL )
    {
     FindAllStmts(dw->get_body(), lst);
    }      
    else if( ( forst=isSgForStatement(node) ) != NULL )
    {
     FindAllStmts(forst->get_loop_body(), lst);
    }
    else if( ( ifst=isSgIfStmt(node) ) != NULL )
    {
     FindAllStmts(ifst->get_true_body(), lst);
     FindAllStmts(ifst->get_false_body(), lst);
    }
    else if( ( whst=isSgWhileStmt(node) ) != NULL )
    {
     FindAllStmts(whst->get_body(), lst);
    }
    else if( ( cldef=isSgClassDefinition(node) ) != NULL )
    {
      //     get_members returns an SgDeclarationStatementPtrList

       SgDeclarationStatementPtrList & bblist=cldef->get_members();
      //call FindAllStmts for each stmt on the bblist
      for(SgDeclarationStatementPtrList::iterator iter=bblist.begin();
                              iter!=bblist.end(); iter++)
      {
        SgStatement *st=*iter;
        FindAllStmts(st, lst);
      }
    }      
    else if( ( fdef=isSgFunctionDefinition(node) ) != NULL )
    {
     FindAllStmts(fdef->get_body(), lst);
   }      
    else if( ( sggl=isSgGlobal(node) ) != NULL )
    {
      // get_declarations returns an SgDeclarationStatementPtrList
      SgDeclarationStatementPtrList & bblist=sggl->get_declarations();
      //call FindAllStmts for each stmt on the bblist
      for(SgDeclarationStatementPtrList::iterator iter=bblist.begin();
                              iter!=bblist.end(); iter++)
      {
        SgStatement *st=*iter;
        FindAllStmts(st, lst);
      }
    }
    else if( ( nmdef=isSgNamespaceDefinitionStatement(node) ) != NULL )
    {
      // get_declarations returns an SgDeclarationStatementPtrList
      SgDeclarationStatementPtrList & bblist=nmdef->get_declarations();
      //call FindAllStmts for each stmt on the bblist
      for(SgDeclarationStatementPtrList::iterator iter=bblist.begin();
                              iter!=bblist.end(); iter++)
      {
        SgStatement *st=*iter;
        FindAllStmts(st, lst);
      }
    }
    else
    {
      printf("error in FindAllStmts\n");
    }
      
  }
  else if( ( stmt=isSgStatement(node) ) != NULL )
  {
    if( ( caseOptionStmt = isSgCaseOptionStmt(node) ) != NULL ) 
    {
      FindAllStmts(caseOptionStmt->get_key(), lst);
      FindAllStmts(caseOptionStmt->get_body(), lst);
    } else {
#ifdef UNRELEASED_ROSE
      if ( !isSgNullStatement(node) ) {
	lst.push_back(stmt);
      }
#else
      lst.push_back(stmt);
#endif
    }
  }
}

/*!
 * Enumerate all the statements in a program 
 * This iterator DOES step into compound statements.
 */

OA::OA_ptr<OA::IRStmtIterator> 
SageIRInterface::getStmtIterator(OA::ProcHandle h)
{
  OA::OA_ptr<OA::IRStmtIterator> iter;

  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

#if 1
  SgFunctionDefinition *functionDefinition =
    isSgFunctionDefinition(node);
  ROSE_ASSERT(node != NULL);
#else
  SgFunctionDeclaration *functionDeclaration =
    isSgFunctionDeclaration(node);
  ROSE_ASSERT(functionDeclaration != NULL);

  SgFunctionDefinition *functionDefinition =
    functionDeclaration->get_definition();
#endif

  iter=new SageIRStmtIterator(functionDefinition, this);
    
  return iter; 
}
  
//! Return an iterator over all of the callsites in a given stmt
OA::OA_ptr<OA::IRCallsiteIterator> SageIRInterface::getCallsites(OA::StmtHandle h)
{
  OA::OA_ptr<OA::IRCallsiteIterator> iter;
  SgStatement * stmt=(SgStatement *)getNodePtr(h);
  OA::OA_ptr<SageIRInterface> ir; ir = new SageIRInterface(*this);
  iter=new SageIRCallsiteIterator(stmt, ir);
  
  return iter;
}

SgNode *
SageIRInterface::getMethodInvocationLhs(SgFunctionCallExp *functionCall)
{
  ROSE_ASSERT(functionCall != NULL);

  SgNode *lhs = NULL;

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  bool isMethod = false;

  switch(expression->variantT()) {
  case V_SgDotExp:
    {
      SgDotExp *dotExp = isSgDotExp(expression);
      ROSE_ASSERT(dotExp != NULL);
	  
      lhs = dotExp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);
	  
      SgPointerDerefExp *pointerDerefExp =
	isSgPointerDerefExp(lhs);
	  
      if ( pointerDerefExp != NULL ) {
	
	// This is (*b).foo() == b->foo();
	lhs = pointerDerefExp->get_operand_i();
	
      } 

      break;
    }
  case V_SgArrowExp:
    {
      SgArrowExp *arrowExp = isSgArrowExp(expression);
      ROSE_ASSERT(arrowExp != NULL);

      lhs = arrowExp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      break;
    }
  case V_SgMemberFunctionRefExp:
  case V_SgFunctionRefExp:
  case V_SgPointerDerefExp:
    {
      break;
    }
  default:
    {
      cerr << "Was not expecting an " << expression->sage_class_name() << endl;
      cerr << "in a function call." << endl;
      ROSE_ABORT();
    }
  }

  return lhs;

}

bool
SageIRInterface::isMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp)
{
  ROSE_ASSERT(functionCall != NULL);

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  bool isMethod = false;
  isDotExp = false;

  switch(expression->variantT()) {
  case V_SgDotExp:
    {
      isMethod = true;

      SgDotExp *dotExp = isSgDotExp(expression);
      ROSE_ASSERT(dotExp != NULL);
	  
      SgExpression *lhs = dotExp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);
	  
      SgPointerDerefExp *pointerDerefExp =
	isSgPointerDerefExp(lhs);
	  
      if ( pointerDerefExp != NULL ) {
	;
      } else {
	isDotExp = true;
      }

      break;
    }
  case V_SgMemberFunctionRefExp:
  case V_SgArrowExp:
    {
      isMethod = true;
      break;
    }
  case V_SgFunctionRefExp:
  case V_SgPointerDerefExp:
    {
      isMethod = false;
      break;
    }
  default:
    {
      cerr << "Was not expecting an " << expression->sage_class_name() << endl;
      cerr << "in a function call." << endl;
      ROSE_ABORT();
    }
  }

  return isMethod;

}

SgFunctionDeclaration * 
SageIRInterface::getFunctionDeclaration(SgFunctionCallExp *functionCall) 
{ 
  SgFunctionDeclaration *funcDec = NULL; 

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  switch(expression->variantT()) {
  case V_SgMemberFunctionRefExp:
    {
      SgMemberFunctionRefExp *memberFunctionRefExp =
	isSgMemberFunctionRefExp(expression);
      ROSE_ASSERT(memberFunctionRefExp != NULL);

      funcDec = memberFunctionRefExp->get_symbol_i()->get_declaration(); 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgDotExp:
    {
      SgDotExp *dotExp = isSgDotExp(expression);
      ROSE_ASSERT(dotExp != NULL);

      if(dotExp->get_traversalSuccessorContainer().size()>=2) { 

	SgMemberFunctionRefExp *memberFunctionRefExp = 
	  isSgMemberFunctionRefExp(dotExp->get_traversalSuccessorContainer()[1]); 
	funcDec = memberFunctionRefExp->get_symbol_i()->get_declaration(); 
      } 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgArrowExp:
    {
      SgArrowExp *arrowExp = isSgArrowExp(expression);
      ROSE_ASSERT(arrowExp != NULL);

      if(arrowExp->get_traversalSuccessorContainer().size()>=2) { 

	SgMemberFunctionRefExp *memberFunctionRefExp = 
	  isSgMemberFunctionRefExp(arrowExp->get_traversalSuccessorContainer()[1]); 
	funcDec = memberFunctionRefExp->get_symbol_i()->get_declaration(); 
      } 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgFunctionRefExp:
    {
      SgFunctionRefExp *functionRefExp = 
	isSgFunctionRefExp(expression);
      ROSE_ASSERT(functionRefExp != NULL);

      // found a standard function reference  
      funcDec = functionRefExp->get_symbol_i()->get_declaration(); 

      ROSE_ASSERT(funcDec != NULL);

      break;
    }
  case V_SgPointerDerefExp:
    {
      ROSE_ABORT();
      break;
    }
  default:
    {
      ROSE_ABORT();
    }
  }

  return funcDec; 
} 


SgExpression * 
SageIRInterface::getFunction(SgFunctionCallExp *functionCall) 
{ 
  SgExpression *function = NULL;

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  switch(expression->variantT()) {
  case V_SgMemberFunctionRefExp:
    {
      function = isSgMemberFunctionRefExp(expression);

      break;
    }
  case V_SgDotExp:
    {
      SgDotExp *dotExp = isSgDotExp(expression);
      ROSE_ASSERT(dotExp != NULL);

      function = isSgMemberFunctionRefExp(dotExp->get_rhs_operand());

      break;
    }
  case V_SgArrowExp:
    {
      SgArrowExp *arrowExp = isSgArrowExp(expression);
      ROSE_ASSERT(arrowExp != NULL);

      function = isSgMemberFunctionRefExp(arrowExp->get_rhs_operand());

      break;
    }
  case V_SgFunctionRefExp:
    {
      function = isSgFunctionRefExp(expression);

      break;
    }
  case V_SgPointerDerefExp:
    {
      ROSE_ABORT();
      break;
    }
  default:
    {
      ROSE_ABORT();
    }
  }

  return function; 
} 


SgPointerDerefExp *
SageIRInterface::isFunctionPointer(SgFunctionCallExp *functionCall) 
{ 
  SgFunctionDeclaration *funcDec = NULL; 

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  SgPointerDerefExp *pointerDerefExp = NULL;

  switch(expression->variantT()) {
  case V_SgPointerDerefExp:
    {
      pointerDerefExp = isSgPointerDerefExp(expression);
      break;
    }
  default:
    {
      break;
    }
  }

  return pointerDerefExp;
} 


OA::SymHandle SageIRInterface::getSymHandle(OA::ExprHandle eh)
{
  OA::SymHandle sh;
  SgExpression * sgexp=(SgExpression*)getNodePtr(eh);
  if(SgFunctionCallExp * fcall=isSgFunctionCallExp(sgexp))
  {

    SgFunctionDeclaration *fundecl = getFunctionDeclaration(fcall);
    ROSE_ASSERT(fundecl != NULL);
    sh=(OA::irhandle_t)(getNodeNumber(fundecl));
  }
  else
  {
    printf("in SageIRInterface::getSymHandle, expression type not yet implemented\n");
    sh=0;
    ROSE_ABORT();
    //if it is a varref rerurn an SgInitializedName
  }
  return sh;
}


OA::MemRefHandle SageIRInterface::getSymMemRefHandle(OA::SymHandle h)
{
  SgNode *node = getNodePtr(h);
  OA::MemRefHandle memRef = getNodeNumber(node);

  return memRef;
}


/** \brief Return a SymHandle corresponding to an initialized name.
 *  \param initName  a SgInitializedName corresponding to an initialized name.
 *  \return An OA::SymHandle representing the SgInitializedName.
 */
OA::SymHandle 
SageIRInterface::getVarSymHandle(SgInitializedName *initName)
{
  if ( initName == NULL ) return ((OA::SymHandle) 0);
  return getNodeNumber(initName);
}

OA::SymHandle 
SageIRInterface::getProcSymHandle(SgFunctionDeclaration *functionDeclaration)
{
  OA::SymHandle sh;

  if ( functionDeclaration == NULL ) return ((OA::SymHandle) 0);

  // If at all possible return a SgFunctionDeclaration that has
  // an associated SgFunctionDefinition.

  sh = getNodeNumber(functionDeclaration);
  if ( functionDeclaration->get_definition() != NULL ) {
    return sh;
  }

  SgFunctionDeclaration *key = 
    isSgFunctionDeclaration(functionDeclaration->get_firstNondefiningDeclaration());
  if ( key == NULL )
    key = functionDeclaration;

  std::map<SgFunctionDeclaration *, SgFunctionDeclaration *>::iterator f;
  f = mFunctions.find(key);

  SgFunctionDeclaration *definingDeclaration = NULL;
  if ( f != mFunctions.end() ) {
    definingDeclaration = f->second;
  } else {
    // Hmmm ... not in there by non-defining definition.  Search
    // explicitly.

    for (std::map<SgFunctionDeclaration *, SgFunctionDeclaration *>::iterator it = mFunctions.begin(); it != mFunctions.end(); ++it) {
      if ( it->first == functionDeclaration ) {
	definingDeclaration = it->second;
	break;
      }
    }
    
  }

  if ( definingDeclaration == NULL )
    definingDeclaration = functionDeclaration;

  sh = getNodeNumber(definingDeclaration);

  return sh;
}

OA::SymHandle SageIRInterface::getProcSymHandle(OA::ProcHandle ph)
{
  OA::SymHandle sh;
  SgFunctionDefinition* fdef=NULL;
  SgNode * pnode=(SgNode*)(getNodePtr(ph));

  // We should allow a NULL ProcHandle.  For example,
  // if a callee proc is undefined (in a particular source file),
  // the ProcHandle will be NULL.  Inter-side effect analysis
  // will call 
  //     mIR->getSideEffect(caller, mIR->getProcSymHandle(callee));
  // The right thing to do here seems to be to return an empty handle.
  if ( pnode == NULL ) return ((OA::SymHandle) 0);

  //A BIG PROBLEM HERE, no pointer from SgFunctionDefinition to SgSymbol
  //ROSE group uses declarations, not symbols to handle call graph
  //maybe we should too (there is always a link from symbol to decl:
  //SgFunctionDeclaration *funcDec = funcSymb->get_declaration()
  
#if 1
  if( ( fdef=isSgFunctionDefinition(pnode) ) != NULL )
  {
    	//SgSymbol * fsymbol=(isSgFunctionRefExp(fcall->get_function()))->get_symbol();
      //sh=(OA::irhandle_t)fsymbol;
    //we will use function declaration, at least for now
    SgFunctionDeclaration * fundecl=fdef->get_declaration();
    sh = getProcSymHandle(fundecl);
#if 0
    //printf("returning func decl from getProcSymHandle\n");
    sh=(OA::irhandle_t)(getNodeNumber(fundecl));
#endif
  }
#else
  if ( isSgFunctionDeclaration(pnode) ) {
    sh = getNodeNumber(pnode);
  }
#endif
  else
  {
    printf("in SageIRInterface::getSymHandle, expression type not yet implemented\n");
    ROSE_ABORT();
    sh=0;
  }
  return sh;
}


// Given a ProcHandle, return an IRRegionStmtIterator* for the
// procedure. The user must free the iterator's memory via delete.
  
  /*!
 * Enumerate all the statements in a program region, e.g. all the statements
 * in a procedure or a loop.  This iterator does NOT step into compound
 * statements.
 */

OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::procBody(OA::ProcHandle h) 
{	
    OA::OA_ptr<SageIRRegionStmtIterator> irStmtIter;
#if 1
    SgFunctionDefinition* functionDefinition = 
      (SgFunctionDefinition*)(getNodePtr(h));
#else
    SgNode *node = getNodePtr(h);
    ROSE_ASSERT(node != NULL);

    SgFunctionDeclaration *functionDeclaration = 
      isSgFunctionDeclaration(node);
    ROSE_ASSERT(functionDeclaration != NULL);

    SgFunctionDefinition *functionDefinition =
      functionDeclaration->get_definition();
    ROSE_ASSERT(functionDefinition != NULL);
#endif
    if (functionDefinition) {
        SgStatementPtrList & pl = 
	  functionDefinition->get_body()->get_statements();
        irStmtIter=new SageIRRegionStmtIterator(pl, this);
    } else {
        cerr << "error in SageIRInterface::procBody\n";
        irStmtIter = NULL;
    }
    return irStmtIter;
}
    


//########################################################
// Statements: General
//########################################################


// Are return statements allowed
bool SageIRInterface::returnStatementsAllowed()
{
    return true;
}


// Translate a Sage statement type into a CFG::IRStmtType.
OA::CFG::IRStmtType SageIRInterface::getCFGStmtType(OA::StmtHandle h)
{
	OA::CFG::IRStmtType ty;
	switch(((SgStatement*)(getNodePtr(h)))->variantT())
	{
		case V_SgBasicBlock:
			ty=OA::CFG::COMPOUND;
			break;
		case V_SgDoWhileStmt:
			ty=OA::CFG::END_TESTED_LOOP;
			break;
		case V_SgForStatement:
			ty=OA::CFG::LOOP;
			break;
		case V_SgIfStmt:
			ty=OA::CFG::STRUCT_TWOWAY_CONDITIONAL;
			break;
		case V_SgSwitchStatement:             //switch is structured multiway conditional
			ty=OA::CFG::STRUCT_MULTIWAY_CONDITIONAL;
			break;
		case V_SgWhileStmt:
			ty=OA::CFG::LOOP;
			break;
		case V_SgContinueStmt:
			ty=OA::CFG::LOOP_CONTINUE;
			break;
		case V_SgBreakStmt:
			ty=OA::CFG::BREAK;
			break;
		case V_SgReturnStmt:
			ty=OA::CFG::RETURN;
			break;
		case V_SgGotoStatement:
			ty=OA::CFG::UNCONDITIONAL_JUMP; //BW or UNCONDITIONAL_JUMP_I not sure which...
			break;
		default:
			ty=OA::CFG::SIMPLE;
	}
	return ty;
}

// Given a statement, return the label associated with it (or NULL if none).
OA::StmtLabel SageIRInterface::getLabel(OA::StmtHandle h)
{
  SgLabelStatement *labelStatement = isSgLabelStatement(getNodePtr(h));
  if (labelStatement == NULL)
    return 0;

  return (OA::irhandle_t)(getNodeNumber(labelStatement));
}

// Given a compound statement, return an IRRegionStmtIterator for the stmts.
OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::getFirstInCompound(OA::StmtHandle h)
{
    OA::OA_ptr<SageIRRegionStmtIterator> retit;
	SgStatement * sptr=(SgStatement*)(getNodePtr(h));
	if(SgScopeStatement * scope=isSgScopeStatement(sptr))
	{
		SgStatementPtrList  & stptr=scope->getStatementList();
		retit=new SageIRRegionStmtIterator(stptr, this);
	}
	else
	{
		printf("error in SageIRInterface::GetFirstInCompound\n");
    retit = new SageIRRegionStmtIterator(this);
	}
	return retit;
}

OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::body(OA::StmtHandle h)  //for now same as GetFirstInCompound
{
    OA::OA_ptr<SageIRRegionStmtIterator> retit;
	SgStatement * sptr=(SgStatement*)(getNodePtr(h));
	if(SgScopeStatement * scope=isSgScopeStatement(sptr))
	{
		SgStatementPtrList  & stptr=scope->getStatementList();
		retit=new SageIRRegionStmtIterator(stptr, this);
	}
	else
	{
		printf("error in SageIRInterface::body, stmt is %s\n", sptr->sage_class_name());
		retit = new SageIRRegionStmtIterator(this);
	}
    return retit;
}


OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::loopBody(OA::StmtHandle h)  //for now same as GetFirstInCompound
{
    OA::OA_ptr<SageIRRegionStmtIterator> retit;
	SgStatement * sptr=(SgStatement*)(getNodePtr(h));
	if(SgScopeStatement * scope=isSgScopeStatement(sptr))
	{
		SgStatementPtrList  & stptr=scope->getStatementList();
		retit=new SageIRRegionStmtIterator(stptr, this);
	}
	else
	{
		printf("error in SageIRInterface::LoopBody\n");
		retit = new SageIRRegionStmtIterator(this);
	}
    return retit;
}

OA::StmtHandle SageIRInterface::loopHeader(OA::StmtHandle h)
{
	SgStatement * st=NULL;
	SgForStatement * forst=isSgForStatement((SgStatement *)getNodePtr(h));
	if(forst)
	{
		st=forst->get_init_stmt().front();
	}
	
	return (OA::irhandle_t)(getNodeNumber(st));
}

OA::StmtHandle SageIRInterface::getLoopIncrement(OA::StmtHandle h)
{
	SgStatement * st=NULL;
	SgForStatement * forst=isSgForStatement((SgStatement *)(getNodePtr(h)));
#if 0
	if(forst)
	{
		Sg_File_Info * finfo=new Sg_File_Info("foobar", 1,1);
		SgExpression * exp=forst->get_increment_expr();
		st=new SgExprStatement(finfo, exp);
    int currentNumber=nodeArrayPtr->size();
    getAttribute(st).add("OANumber", new SageNodeNumAttr(currentNumber));
    nodeArrayPtr->push_back(st);
	}
	
	return (OA::irhandle_t)(getNodeNumber(st));
#else
	SgExpression * exp=NULL;
	if(forst)
	{
		exp=forst->get_increment_expr();
	}
	
	return (OA::irhandle_t)(getNodeNumber(exp));
#endif

}


bool SageIRInterface::loopIterationsDefinedAtEntry(OA::StmtHandle h)
{
	//this needs to be false for C (read comments in CFG.C)
	return false;
}


OA::StmtLabel SageIRInterface::getTargetLabel(OA::StmtHandle h, int) 
{
  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  OA::StmtLabel targetLabel = 0;

  switch(node->variantT()) {

  case V_SgGotoStatement:
    {
      SgGotoStatement *gotoStatement = isSgGotoStatement(node);
      ROSE_ASSERT(gotoStatement != NULL);

      SgLabelStatement *labelStatement = gotoStatement->get_label();
      ROSE_ASSERT(labelStatement != NULL);
      
      targetLabel = (OA::irhandle_t)(getNodeNumber(labelStatement));

      break;
    }
  default:
    {
      cerr << "Didn't know how to handle node type " << node->sage_class_name();
      cerr << " in SageIRInterface::getTargetLabel" << endl;
      ROSE_ABORT();
      break;
    }
  }
  return targetLabel;
}


//-----------------------------------------------------------------------------
// Unstructured multiway conditionals
//-----------------------------------------------------------------------------
//No unstructured multiway conditionals in C?
// Given an unstructured multi-way branch, return the number of targets.
// The count does not include the optional default/catchall target.
int SageIRInterface::numUMultiTargets(OA::StmtHandle h) 
{
	//for now
  ROSE_ABORT();
	return 0;
}

// Given an unstructured multi-way branch, return the label of the target
// statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
OA::StmtLabel SageIRInterface::getUMultiTargetLabel(OA::StmtHandle h, int)
{
	//for now
  ROSE_ABORT();
	return 0;
}

// Given an unstructured multi-way branch, return the label of the optional
// default/catchall target. Return 0 if no default target.
OA::StmtLabel SageIRInterface::getUMultiCatchallLabel(OA::StmtHandle h)
{
	//for now
  ROSE_ABORT();
	return 0;
}

OA::ExprHandle 
SageIRInterface::getUMultiCondition (OA::StmtHandle h, int targetIndex)
{
    // for now
  ROSE_ABORT();
    return 0;
}

  //------------------------------
  // Structured two-way conditionals
  //
  // Note: An important pre-condition for structured conditionals is that chains
  // of else-ifs must be represented as nested elses.  For example, this Matlab
  // statement:
  //   if (c1)
  //     s1;
  //   elseif (c2)
  //     s2;
  //   else
  //     s3;
  //   end;
  //
  // would need be represented by the underlying IR as:
  //   if (c1)
  //     s1;
  //   else
  //     if (c2)
  //       s2;
  //     else
  //       s3;
  //     end;
  //   end; 
  //------------------------------
  
  //BW Sage already converts else ifs into nested elses...

  // Given a structured two-way conditional statement, return an IRRegionStmtIterator*
  // for the "true" part (i.e., the statements under the "if" clause).
  

OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::trueBody(OA::StmtHandle h)
{
  OA::OA_ptr<SageIRRegionStmtIterator> retval;
  //return iterator of the true branch if  h if an if stmt
  //for now
  //return NULL;
  //printf("in TrueBody\n");
  retval = NULL;
  if(SgIfStmt * ifs=isSgIfStmt((SgStatement*) getNodePtr(h)))
    {
      SgStatement * sptr=ifs->get_true_body();
      SgScopeStatement * scope=isSgScopeStatement(sptr);
      if(scope)
	{
	  retval = new SageIRRegionStmtIterator(scope->getStatementList(), this);	
	} else { retval = new SageIRRegionStmtIterator(this); }
    }

  return retval; 
}

OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::elseBody(OA::StmtHandle h)
{
    OA::OA_ptr<SageIRRegionStmtIterator> retval;  retval = NULL;
	//printf("in ElseBody\n");
    if(SgIfStmt * ifs=isSgIfStmt((SgStatement*) getNodePtr(h)))
      {
	SgStatementPtrList stptr;
	SgStatement * sptr=ifs->get_false_body();
	SgScopeStatement * scope=isSgScopeStatement(sptr);
	fflush(stdout);
	if(scope)
	  {
	    retval = new SageIRRegionStmtIterator(scope->getStatementList(), this);	
	  }
	else
	  {
	    retval = new SageIRRegionStmtIterator(this);
	  }
      }
    return retval;
}
/*
ExprHandle SageIRInterface::GetLoopCondition(StmtHandle h)
{
	SgForStatement * forst=isSgForStatement((SgStatement *) h);
	SgExpression * exp=NULL;
	if(forst)
	{
		exp=forst->get_test_expr();
	}
	return (ExprHandle)exp;
}
*/

//-----------------------------------------------------------------------------
// Structured conditionals
//
// FIXME: Is GetCondition for unstructured conditionals also?  It is currently
// implemented that way.
//-----------------------------------------------------------------------------
/*
OA::ExprHandle SageIRInterface::GetCondition(StmtHandle h)
{
	SgExpression * exp=NULL;
	SgForStatement * forst=NULL;
	SgWhileStmt * whst=NULL;
	SgDoWhileStmt * dost=NULL;
	SgIfStmt * ifst=NULL;

	if(forst=isSgForStatement((SgStatement *) h))
	{
		exp=forst->get_test_expr();
	}
	else if(whst=isSgWhileStmt((SgStatement *) h))
	{
		SgStatement * condst=whst->get_condition();
		SgExprStatement * expst=isSgExprStatement(condst);
		if(condst)
    {
			exp=expst->get_the_expr();
    }
	}
	else if(dost=isSgDoWhileStmt((SgStatement *) h))
	{
		SgStatement * condst=dost->get_condition();
		SgExprStatement * expst=isSgExprStatement(condst);
		if(condst)
    {
			exp=expst->get_the_expr();
    }
	}
	else if(ifst=isSgIfStmt((SgStatement *) h))
	{
		SgStatement * condst=ifst->get_conditional();
		SgExprStatement * expst=isSgExprStatement(condst);
		if(condst)
    {
			exp=expst->get_the_expr();
    }
	}
	return (ExprHandle)exp;
}

// Given an unstructured multi-way branch, return the condition expression
// corresponding to target 'targetIndex'. The n targets are indexed [0..n-1].
ExprHandle SageIRInterface::GetUMultiCondition(StmtHandle h, int targetIndex)
{
  // FIXME: It isn't yet decided whether or not this function is needed in
  // the IR interface.
  assert (0);
  return 0;
}
*/
//------------------------------
// structured multiway conditionals
//------------------------------
//treat C/C++ switch as structured multiway conditional
// condition for multi body 
OA::ExprHandle SageIRInterface::getSMultiCondition(OA::StmtHandle h, int bodyIndex)
{
  OA::ExprHandle ret=0;
  SgSwitchStatement * swstmt=isSgSwitchStatement((SgStatement*)getNodePtr(h));
  if(swstmt)
  {
    SgBasicBlock * bb=swstmt->get_body();
    if(bb)
    {
      SgNode * n=bb->get_traversalSuccessorContainer()[bodyIndex];
      SgCaseOptionStmt * copt;
      SgDefaultOptionStmt * dopt;
      if(n && (copt=isSgCaseOptionStmt(n))!=NULL)
      {
        ret=(OA::irhandle_t)(getNodeNumber((copt->get_key())));
      }
      else if(n && (dopt=isSgDefaultOptionStmt(n))!=NULL)
      {
        ret=0;
      }
    }
  }
  return ret;
}

/*
// multi-way beginning expression
ExprHandle SageIRInterface::GetMultiExpr(StmtHandle h)
{
  ExprHandle ret=0;
  SgSwitchStatement * sw=isSgSwitchStatement((SgStatement*)h);
  if(sw)
  {
    ret=(ExprHandle)sw->get_item_selector();
  }
  return ret;
}
*/

//NumMultiCases returns a number that is equal to either:
// 1. the number of non-default case if default comes last (or not at all)
//2. the total number of cases (including the default) if default is not the last
// MMS 12/17/03, changing this so it just returns the number of cases

int SageIRInterface::numMultiCases (OA::StmtHandle h)
{
  int tot_len=0;
  SgSwitchStatement * sw=isSgSwitchStatement((SgStatement*)getNodePtr(h));
  if(sw)
  {
    SgBasicBlock * bb=sw->get_body();
    if(bb)
    {
      tot_len=bb->get_traversalSuccessorContainer().size();
/*
      SgNode * n=body->get_traversalSuccessorContainer()[tot_len-1];
      if(isSgDefaultOptionStmt(n))
      {
        tot_len=tot_len-1;
      }
*/
    }
  }
  return tot_len;
}

OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::multiBody (OA::StmtHandle h, int bodyIndex)
{
  OA::OA_ptr<OA::IRRegionStmtIterator> retval;  retval = NULL;
  SgSwitchStatement * swstmt=isSgSwitchStatement((SgStatement*)getNodePtr(h));
  if(swstmt)
  {
    SgBasicBlock * bb=swstmt->get_body();
    if(bb)
    {
      SgNode * n=bb->get_traversalSuccessorContainer()[bodyIndex];
      SgCaseOptionStmt * copt;
      SgDefaultOptionStmt * dopt;
      if(n && (copt=isSgCaseOptionStmt(n))!=NULL)
      {
        retval = body((OA::irhandle_t)(getNodeNumber((copt->get_body()))));
      }
      else if(n && (dopt=isSgDefaultOptionStmt(n))!=NULL)
      {
        retval = body((OA::irhandle_t)(getNodeNumber((dopt->get_body()))));
      }
    }
  } else { retval = new SageIRRegionStmtIterator(this); }
  return retval;
}

bool SageIRInterface::isBreakImplied (OA::StmtHandle multicond)
{
  return FALSE;  //break is not implied in C
}

bool SageIRInterface::isCatchAll(OA::StmtHandle h, int bodyIndex)
{
  bool isDefault = false;

  SgSwitchStatement * swstmt=isSgSwitchStatement((SgStatement*)getNodePtr(h));
  if(swstmt)
  {
    SgBasicBlock * bb=swstmt->get_body();
    if(bb)
    {
      SgNode * n=bb->get_traversalSuccessorContainer()[bodyIndex];
/*
      if (isSgDefaultOptionStmt(n)) {
        isDefault = true;
      }
*/

      SgCaseOptionStmt * copt;
      SgDefaultOptionStmt * dopt;
      if(n && (copt=isSgCaseOptionStmt(n))!=NULL)
      {
        isDefault = false;
      }
      else if(n && (dopt=isSgDefaultOptionStmt(n))!=NULL)
      {
        isDefault = true;
      }

    }
  }
  return isDefault;
}

//IMPORTANT
//GetMultiCatchall returns the iterator over the statements in the default case
//ONLY IF the default case is the last one!!!!
//

OA::OA_ptr<OA::IRRegionStmtIterator> 
    SageIRInterface::getMultiCatchall (OA::StmtHandle h)
{
  OA::OA_ptr<OA::IRRegionStmtIterator> retval;  retval = NULL;
  int tot_len=0;
  SgSwitchStatement * sw=isSgSwitchStatement((SgStatement*)getNodePtr(h));
  if(sw)
  {
    SgBasicBlock * bb=sw->get_body();
    if(bb)
    {
      tot_len=bb->get_traversalSuccessorContainer().size();
      SgNode * n=bb->get_traversalSuccessorContainer()[tot_len-1];
      SgDefaultOptionStmt * dopt;
      if( ( dopt=isSgDefaultOptionStmt(n) ) != NULL )
      {
        //printf("found default!!!!!!!!!!!!\n");
        retval = body((OA::irhandle_t)(getNodeNumber((dopt->get_body()))));
      }
    }
  } else { retval = new SageIRRegionStmtIterator(this); }
  return retval;
}

//! Return a list of all def  memory reference handles that appear
//! in the given statement.
//! This is nearly identical to Open64IRInterface::getDefMemRefs.
OA::OA_ptr<OA::MemRefHandleIterator> 
SageIRInterface::getDefMemRefs(OA::StmtHandle stmt) 
{
  OA::OA_ptr<std::list<OA::MemRefHandle> > retList;
  retList = new std::list<OA::MemRefHandle>;

  // get iterator over memory references for this statement
  // and only put DEFs in the list
  OA::OA_ptr<OA::MemRefHandleIterator> mIter = getMemRefIterator(stmt);
  for ( ; mIter->isValid(); (*mIter)++ ) {
    OA::MemRefHandle memref = mIter->current();

    // loop over memory reference expressions for this memref handle
    set<OA::OA_ptr<OA::MemRefExpr> >::iterator mreIter;
    for (mreIter = sMemref2mreSetMap[memref].begin();
         mreIter != sMemref2mreSetMap[memref].end(); mreIter++ )
      {
	OA::OA_ptr<OA::MemRefExpr> mre = *mreIter;
        if (mre->isDef()) {
          retList->push_back(memref);
          break;
        }
      }
  }

  OA::OA_ptr<SageMemRefHandleIterator> retval;
  retval = new SageMemRefHandleIterator(retList);
  return retval;
}

//! Return a list of all use memory reference handles that appear
//! in the given statement.
//! This is nearly identical to Open64IRInterface::getUseMemRefs.
OA::OA_ptr<OA::MemRefHandleIterator> 
SageIRInterface::getUseMemRefs(OA::StmtHandle stmt)
{
  OA::OA_ptr<std::list<OA::MemRefHandle> > retList;
  retList = new std::list<OA::MemRefHandle>;

  // get iterator over memory references for this statement
  // and only put USES in the list
  OA::OA_ptr<OA::MemRefHandleIterator> mIter = getMemRefIterator(stmt);
  for ( ; mIter->isValid(); (*mIter)++ ) {
    OA::MemRefHandle memref = mIter->current();

    // loop over memory reference expressions for this memref handle
    set<OA::OA_ptr<OA::MemRefExpr> >::iterator mreIter;
    for (mreIter = sMemref2mreSetMap[memref].begin();
         mreIter != sMemref2mreSetMap[memref].end(); mreIter++ )
      {
	OA::OA_ptr<OA::MemRefExpr> mre = *mreIter;
        if (mre->isUse()) {
          retList->push_back(memref);
          break;
        }
      }
  }

  OA::OA_ptr<SageMemRefHandleIterator> retval;
  retval = new SageMemRefHandleIterator(retList);
  return retval;

}

//! Return a list of all the memory reference handles that appear
//! in the given statement.
//! This is nearly identical to Open64IRInterface::getAllMemRefs.
OA::OA_ptr<OA::MemRefHandleIterator> 
SageIRInterface::getAllMemRefs(OA::StmtHandle stmt)
{
  OA::OA_ptr<std::list<OA::MemRefHandle> > retList;
  retList = new std::list<OA::MemRefHandle>;

  // get iterator over memory references for this statement
  // and for now just copy the list
  OA::OA_ptr<OA::MemRefHandleIterator> mIter = getMemRefIterator(stmt);
  for ( ; mIter->isValid(); (*mIter)++ ) {
    OA::MemRefHandle memref = mIter->current();
    retList->push_back(memref);
  }

  OA::OA_ptr<SageMemRefHandleIterator> retval;
  retval = new SageMemRefHandleIterator(retList);
  return retval;
}

OA::OA_ptr<OA::MemRefHandleIterator> 
SageIRInterface::getAllMemRefs(OA::IRHandle h)
{
  OA::OA_ptr<std::list<OA::MemRefHandle> > retList;
  retList = new std::list<OA::MemRefHandle>;

  // get iterator over memory references for this statement
  // and for now just copy the list
  OA::OA_ptr<OA::MemRefHandleIterator> mIter = getMemRefIterator(h);
  for ( ; mIter->isValid(); (*mIter)++ ) {
    OA::MemRefHandle memref = mIter->current();
    retList->push_back(memref);
  }

  OA::OA_ptr<SageMemRefHandleIterator> retval;
  retval = new SageMemRefHandleIterator(retList);
  return retval;
}


// Utility function to look through typedefs to return a type.
SgType *
SageIRInterface::getBaseType(SgType *type) 
{
  if ( type == NULL ) return NULL;

  SgTypedefType *typedefType = isSgTypedefType(type);
  if (typedefType != NULL) {

    SgType *baseType = typedefType->get_base_type();
    ROSE_ASSERT(baseType != NULL);
    return getBaseType(baseType);

  }

  return type;
}

OA::Alias::IRStmtType SageIRInterface::getAliasStmtType(OA::StmtHandle h)
{ 
  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  OA::Alias::IRStmtType stmtType = OA::Alias::ANY_STMT;

  bool collectPtrAssigns = false;

  switch(node->variantT()) {
 
  case V_SgReturnStmt:
    {
      // A return statement will generate a pointer assign pair
      // if it returns a pointer or a reference.
      SgReturnStmt *returnStmt = isSgReturnStmt(node);
      ROSE_ASSERT(returnStmt != NULL);

      SgExpression *returnExpr = returnStmt->get_return_expr();
      ROSE_ASSERT(returnExpr != NULL);

      SgType *returnType = returnExpr->get_type();
      ROSE_ASSERT(returnType != NULL);

      SgType *baseType = getBaseType(returnType);
      ROSE_ASSERT(baseType != NULL);
      if ( isSgPointerType(baseType) || isSgReferenceType(baseType) ) {
	stmtType = OA::Alias::PTR_ASSIGN_STMT;
      } else {
	// For some reason the type of '(new B)' is SgClassType.
	if ( isSgClassType(baseType) ) {
	  SgFunctionDefinition *enclosingProc = 
	    getEnclosingMethod(returnStmt);
	  ROSE_ASSERT(enclosingProc != NULL);
	  
	  SgFunctionDeclaration *functionDeclaration =
	    enclosingProc->get_declaration();
	  ROSE_ASSERT(functionDeclaration != NULL);

	  SgType *funcReturnType = functionDeclaration->get_orig_return_type();
	  if ( funcReturnType != NULL ) {

	    SgType *baseRetType = getBaseType(funcReturnType);
	    ROSE_ASSERT(baseRetType != NULL);
	    if ( isSgPointerType(baseRetType) || isSgReferenceType(baseRetType) ) {
	      stmtType = OA::Alias::PTR_ASSIGN_STMT;
	    }

	  }
	}
      }

      break;
    }
  case V_SgExprStatement:
    {
      SgExprStatement *exprStatement = isSgExprStatement(node);
      ROSE_ASSERT(exprStatement != NULL);

      SgExpression *expression = exprStatement->get_the_expr();
      SgType *lhsType = NULL;

      switch(expression->variantT()) {
      case V_SgAssignOp:
	{
	  SgBinaryOp *assignOp = isSgBinaryOp(expression);
	  ROSE_ASSERT(assignOp != NULL);

	  SgExpression *lhs = assignOp->get_lhs_operand();
	  ROSE_ASSERT(lhs != NULL);

	  lhsType = lhs->get_type();
	  ROSE_ASSERT(lhsType != NULL);
	  break;
	}
      default:
	{
	  break;
	}
      }
      
      if ( lhsType != NULL ) {
	SgType *baseType = getBaseType(lhsType);
	ROSE_ASSERT(baseType != NULL);
	if ( isSgPointerType(baseType) || isSgReferenceType(baseType) ) 
	  stmtType = OA::Alias::PTR_ASSIGN_STMT;
      }
      break;
    }

  case V_SgVariableDeclaration:
    {
      SgVariableDeclaration *varDecl = isSgVariableDeclaration(node);
      ROSE_ASSERT(varDecl != NULL);

      bool collectPtrAssigns = false;
      
      if ( varDeclHasPtrAssign(varDecl, collectPtrAssigns, NULL) ) {
	stmtType = OA::Alias::PTR_ASSIGN_STMT;
      } else {
	if ( isObjectDeclaration(varDecl, collectPtrAssigns, NULL) )
	  stmtType = OA::Alias::PTR_ASSIGN_STMT;
      }

      break;
    }

  case V_SgCtorInitializerList: 
    {
      SgCtorInitializerList *initializerList =
	isSgCtorInitializerList(node);
      ROSE_ASSERT(initializerList != NULL);

      bool collectPtrAssigns = false;

      if ( ctorInitListHasPtrAssign(initializerList, collectPtrAssigns, NULL) )
	stmtType = OA::Alias::PTR_ASSIGN_STMT;

      break;
    }

#ifdef VTABLE_OPT
  case V_SgClassDefinition:
    {
      // When using the vtable-optimization for virtual method
      // resolution (via FIAlias), we return implicit assignments
      // at a class definition.  For each method m of a class C, we get an 
      // implicit assignment of the form < C.m, C::m >, i.e.,
      // < FieldAccess(NamedRef(SymHandle(class C)),FieldHandle(m)), 
      //   NamedRef(SymHandle(C::m)) >
      // Note, we should only do this if the class actually has
      // virtual methods.
      
      SgClassDefinition *classDefinition =
	isSgClassDefinition(node);

      if ( classHasVirtualMethods(classDefinition )
	   stmtType = OA::Alias::PTR_ASSIGN_STMT;

      break;
    }
#endif

  default: 
    {
      break;
    }

  }

  return stmtType;
}

OA::OA_ptr<OA::IRSymIterator> SageIRInterface::getVisibleSymIterator(OA::ProcHandle h)
{
  OA::OA_ptr<OA::IRSymIterator> retval;//=NULL;
  ROSE_ABORT();
  return retval;
}

OA::OA_ptr<OA::IRStmtIterator> SageIRInterface::getUnnamedDynAllocStmtIterator(OA::ProcHandle h)
{
  OA::OA_ptr<OA::IRStmtIterator> retval;//=NULL;
  ROSE_ABORT();
  return retval;
}


OA::OA_ptr<OA::MemRefExprIterator> 
SageIRInterface::getMemRefExprIterator(OA::MemRefHandle h)
{
  OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr> > > retList;
  retList = new std::list<OA::OA_ptr<OA::MemRefExpr> >;

  // iterate over set of MemRefExpr's associated with
  // the given MemRefHandle and put them in our list
  set<OA::OA_ptr<OA::MemRefExpr> >::iterator mreIter;
  for (mreIter = sMemref2mreSetMap[h].begin();
       mreIter != sMemref2mreSetMap[h].end(); mreIter++ )
    {
      retList->push_back(*mreIter);
    }

  OA::OA_ptr<SageMemRefExprIterator> retval;
  retval = new SageMemRefExprIterator(retList, this);
  return retval;
}

void SageIRInterface::dump(OA::OA_ptr<OA::MemRefExprIterator> memRefIterator,
			   std::ostream& os) {	  
  
  for ( ; memRefIterator->isValid(); (*memRefIterator)++ ) { 
    OA::OA_ptr<OA::MemRefExpr> memRefExp = memRefIterator->current();
    ROSE_ASSERT(!memRefExp.ptrEqual(0));

    memRefExp->dump(os, *this);
    os << endl;
  }
}

//need to know if a location is "local" with respect to a procedure.  
//local here means visible only in that procedure.  
//Member variables are not local to any one method in a class.

// Create a named location based on SymHandle.
OA::OA_ptr<OA::Location::Location> 
SageIRInterface::getLocation(OA::ProcHandle p, OA::SymHandle s)
{
  OA::OA_ptr<OA::Location> loc;
  loc = NULL;

  if((((int)s)==0) || (s.hval()==0))
  {
    return loc;
  }

  bool isLocal = false;

  SgNode *node = getNodePtr(s);
  ROSE_ASSERT(node != NULL);

  SgNode *procNode = getNodePtr(p);
  ROSE_ASSERT(procNode != NULL);

  switch(node->variantT()) {
 
  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL);

      SgDeclarationStatement *declarationStmt = initName->get_declaration();
      ROSE_ASSERT(declarationStmt != NULL);
      
      SgNode *declarationParent = declarationStmt->get_parent();
      ROSE_ASSERT(declarationParent != NULL);
      
      if ( isSgGlobal(declarationParent) ) {
	// This symbol is global.
	isLocal = false;
      } else {
	
	SgFunctionDefinition *enclosingProc = 
	  getEnclosingMethod(declarationStmt);
	
	SgFunctionDefinition *procDefn = 
	  isSgFunctionDefinition(procNode);
	ROSE_ASSERT(procDefn != NULL);
	
	if ( enclosingProc == procDefn ) {
	  // This symbol is local to this procedure.
	  isLocal = true;
	} else {
	  // This symbol is not visible within this procedure, 
	  // so return a NULL location.
	  return loc;
	}
      } 
    break;
    }
  case V_SgFunctionDeclaration:
  case V_SgMemberFunctionDeclaration:
    {
#if 1
      // We had problems in which a method didn't appear visible but
      // should have been in PandeEtAlRutgers05.C.
      // In main, p and q have static type Base, though they may
      // dynamically be Derived.  Thus, ROSE can only report that
      // the SgFunctionDeclaration for p->foo or q->foo is 
      // Base::foo (as per the static type of p and q).  Thus, it
      // does not see that Derived::foo is called (i.e., when 
      // we ask for all function/method invocations via querySubTree
      // below).  Hence, the logic dictates that since Derived::foo is
      // not main and is not invoked from main, it is not visible in
      // main.  Clearly, this isn't true.
      //
      // For now, the solution is simply to call all functions/methods
      // visible.  This isn't true given static functions, private methods,
      // etc.  There is probably some way to resolve this in ROSE using
      // the context-specific symbol tables, but I have little 
      // experience there and we have a deadline.
      // MMS and BW 3/16/06
      isLocal = true;
#else
      SgFunctionDeclaration *functionDeclaration =
	isSgFunctionDeclaration(node);
      ROSE_ASSERT(functionDeclaration != NULL);

      // We return SgFunctionDeclarations from pointer assign
      // pairs to model the lhs of a return expression.
      // e.g., int *foo() { ... return ptr; }
      //       -> foo = pt.
      // So, if the SgFunctionDeclaration passed here as a symbol
      // is the same as proc, it is local.
      SgFunctionDefinition *procDefn = 
	isSgFunctionDefinition(procNode);
      ROSE_ASSERT(procDefn != NULL);

      SgFunctionDeclaration *procDecl = procDefn->get_declaration();
      ROSE_ASSERT(procDecl != NULL);

      if ( ( functionDeclaration == procDecl ) ||
	   ( functionDeclaration == procDecl->get_firstNondefiningDeclaration() ) ||
	   ( functionDeclaration->get_firstNondefiningDeclaration() == procDecl ) ||
	   ( ( functionDeclaration->get_firstNondefiningDeclaration() == procDecl->get_firstNondefiningDeclaration() ) && ( functionDeclaration->get_firstNondefiningDeclaration() != NULL ) ) )  {
	isLocal = true;
      }

      if ( !isLocal ) {

	// We also return SgFunctionDeclarations from pointer assign
	// pairs to model the rhs of an assignment to a function's
	// return value.
	// e.g., ptr = foo();
	//       -> ptr = foo
	// If the procedure calls this function then we could
	// have legitimately seen this SgFunctionDeclaration used
	// to represent the called procedure's return.
	
	// See which functions/methods were invoked from procedure p.
	list<SgNode *> nodes = NodeQuery::querySubTree(procDefn,
						       V_SgFunctionCallExp);
	for (list<SgNode *>::iterator it = nodes.begin();
	     it != nodes.end(); ++it ) {
	  SgNode *n = *it;
	  ROSE_ASSERT(n != NULL);
	  
	  SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(n);
	  ROSE_ASSERT(functionCallExp != NULL);
	  
	  SgFunctionDeclaration *invokedFuncDeclaration =
	    getFunctionDeclaration(functionCallExp);
	  
	  if ( invokedFuncDeclaration == functionDeclaration ) {
	    isLocal = true;
	    break;
	  }
	}
      }

      // We did not call this function from p.
      if ( !isLocal ) {
	
	// If this is a method invocation, perhaps it is a constructor?
	SgMemberFunctionDeclaration *memberFunctionDeclaration =
	  isSgMemberFunctionDeclaration(functionDeclaration);
	if ( memberFunctionDeclaration != NULL ) {

	  list<SgNode *> newExprs = NodeQuery::querySubTree(procDefn,
							    V_SgNewExp);
	  for (list<SgNode *>::iterator it = newExprs.begin();
	       it != newExprs.end(); ++it ) {
	    SgNode *n = *it;
	    ROSE_ASSERT(n != NULL);
	    
	    SgNewExp *newExp = isSgNewExp(n);
	    ROSE_ASSERT(newExp != NULL);
	    
	    SgConstructorInitializer *ctorInitializer = 
	      newExp->get_constructor_args();
	    ROSE_ASSERT(ctorInitializer != NULL);
	    
	    SgMemberFunctionDeclaration *invokedFuncDeclaration =
	      ctorInitializer->get_declaration();
	    
	    if ( invokedFuncDeclaration == functionDeclaration ) {
	      isLocal = true;
	      break;
	    }
	  }	

	  if ( !isLocal ) {
	    list<SgNode *> ctorExprs = NodeQuery::querySubTree(procDefn,
							       V_SgConstructorInitializer);
	    for (list<SgNode *>::iterator it = ctorExprs.begin();
		 it != ctorExprs.end(); ++it ) {
	      SgNode *n = *it;
	      ROSE_ASSERT(n != NULL);
	      
	      SgConstructorInitializer *ctorInitializer = 
		isSgConstructorInitializer(n);
	      ROSE_ASSERT(ctorInitializer != NULL);
	      
	      SgMemberFunctionDeclaration *invokedFuncDeclaration =
		ctorInitializer->get_declaration();
	      
	      if ( invokedFuncDeclaration == functionDeclaration ) {
		isLocal = true;
		break;
	      }
	    }	
	  }
	}
      }

      if ( !isLocal ) {
	return loc;
      }
#endif
      break;
    }
  case V_SgFunctionParameterList:
    {
      // If we see a SgFunctionParameterList where we expected a symbol,
      // it means that the symbol represents a 'this' pointer.
      SgFunctionParameterList *parameterList = 
	isSgFunctionParameterList(node);
      ROSE_ASSERT(parameterList != NULL);

      SgNode *parent = parameterList->get_parent();
      ROSE_ASSERT(parent != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	isSgFunctionDeclaration(parent);
      ROSE_ASSERT(functionDeclaration != NULL);
	
      SgFunctionDefinition *procDefn = 
	isSgFunctionDefinition(procNode);
      ROSE_ASSERT(procDefn != NULL);

      SgFunctionDeclaration *procDecl = procDefn->get_declaration();
      ROSE_ASSERT(procDecl != NULL);

      if ( ( functionDeclaration == procDecl ) ||
	   ( functionDeclaration == procDecl->get_firstNondefiningDeclaration() ) ||
	   ( functionDeclaration->get_firstNondefiningDeclaration() == procDecl ) ||
	   ( ( functionDeclaration->get_firstNondefiningDeclaration() == procDecl->get_firstNondefiningDeclaration() ) && ( functionDeclaration->get_firstNondefiningDeclaration() != NULL ) ) )  {
	isLocal = true;
      }

      if ( !isLocal ) {
	// This symbol is not visible within this procedure, 
	// so return a NULL location.
	return loc;
      }
      break;
    }
#if 0
  case V_SgCtorInitializerList:
    {
      // If we see a SgCtorIntializerList where we expected a symbol,
      // it means that the symbol represents a 'this' pointer.

      SgCtorInitializerList *ctorInitializer = 
	isSgCtorInitializerList(node);
      ROSE_ASSERT(ctorInitializer != NULL);

      SgNode *parent = ctorInitializer->get_parent();
      ROSE_ASSERT(parent != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	isSgFunctionDeclaration(parent);
      ROSE_ASSERT(functionDeclaration != NULL);
	
      SgFunctionDefinition *procDefn = 
	isSgFunctionDefinition(procNode);
      ROSE_ASSERT(procDefn != NULL);

      SgFunctionDeclaration *procDecl = procDefn->get_declaration();
      ROSE_ASSERT(procDecl != NULL);

      if ( ( functionDeclaration == procDecl ) ||
	   ( functionDeclaration == procDecl->get_firstNondefiningDeclaration() ) ||
	   ( functionDeclaration->get_firstNondefiningDeclaration() == procDecl ) ||
	   ( ( functionDeclaration->get_firstNondefiningDeclaration() == procDecl->get_firstNondefiningDeclaration() ) && ( functionDeclaration->get_firstNondefiningDeclaration() != NULL ) ) )  {
	isLocal = true;
      }

      if ( !isLocal ) {
	// This symbol is not visible within this procedure, 
	// so return a NULL location.
	return loc;
      }
      break;
    }
#endif
  default:
    {
      break;
    }
  }
  
  loc = new OA::NamedLoc(s, isLocal);
  return loc;
}

void
NumberTraversal::visit ( SgNode* astNode )
{
	if(!astNode) { printf("visited with 0-Node!\n"); return; }
	//  printf("node is %s\n", astNode->sage_class_name());
  SgExpression * exp=isSgExpression(astNode);
  SgStatement * st=isSgStatement(astNode);
  SgInitializedName * in=isSgInitializedName(astNode);
  int currentNumber=ir->nodeArrayPtr->size();

  switch (astNode->variantT() ) {

  case V_SgNewExp:
    {

      SgNewExp *newExp = isSgNewExp(astNode);
      ROSE_ASSERT(newExp != NULL);

      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      if ( !ir->createsBaseType(ctorInitializer) ) {

	// This function declaration won't get visited by the 
	// traversal.
	SgMemberFunctionDeclaration *functionDeclaration = 
	  ctorInitializer->get_declaration();
	ROSE_ASSERT(functionDeclaration != NULL);
	
	if ( !ir->getAttribute(functionDeclaration).exists("OANumber") ) {
	  ir->getAttribute(functionDeclaration).add("OANumber", new SageNodeNumAttr(currentNumber));
	  ir->nodeArrayPtr->push_back(functionDeclaration);
	  
	  currentNumber=ir->nodeArrayPtr->size();
	}
	
#if 1
	SgFunctionParameterList *parameterList = 
	  functionDeclaration->get_parameterList(); 
	
	if ( !ir->getAttribute(parameterList).exists("OANumber") ) {
	  ir->getAttribute(parameterList).add("OANumber", new SageNodeNumAttr(currentNumber));
	  ir->nodeArrayPtr->push_back(parameterList);
	  
	  currentNumber=ir->nodeArrayPtr->size();
	  
	}
#endif
      }

      break;
    }

  case V_SgFunctionDeclaration:
  case V_SgMemberFunctionDeclaration:
    {
      SgFunctionDeclaration *functionDeclaration =
	isSgFunctionDeclaration(astNode);
      ROSE_ASSERT(functionDeclaration != NULL);

      SgFunctionParameterList *parameterList = 
	functionDeclaration->get_parameterList(); 

      if ( !ir->getAttribute(parameterList).exists("OANumber") ) {
	ir->getAttribute(parameterList).add("OANumber", new SageNodeNumAttr(currentNumber));
	ir->nodeArrayPtr->push_back(parameterList);
	
	currentNumber=ir->nodeArrayPtr->size();

      }

      SgMemberFunctionDeclaration *memberFunctionDeclaration =
	isSgMemberFunctionDeclaration(astNode);
      if ( memberFunctionDeclaration != NULL ) {
	// Do _not_ remove this next call to get_ctors().
	// It has side effects.  In particular, the initializerList
	// may be NULL before the call, but is always non-NULL
	// afterwards.  i.e., force it into existence now and number it.
	// Bad things could result if it is later conjured up,
	// since we won't have a number for it.
	SgInitializedNamePtrList &list = 
	  memberFunctionDeclaration->get_ctors(); 

	SgCtorInitializerList *initializerList =
	  memberFunctionDeclaration->get_CtorInitializerList();
	if ( initializerList != NULL ) {
	  if ( !ir->getAttribute(initializerList).exists("OANumber") ) {
	    ir->getAttribute(initializerList).add("OANumber", new SageNodeNumAttr(currentNumber));
	    ir->nodeArrayPtr->push_back(initializerList);
	    currentNumber=ir->nodeArrayPtr->size();
	  }
	}
      }

      break;
    }

  default:
    {
      break;
    }

  }
  

  if(exp || st || in)
  {
    if(in)
    { 
      //this does not work for math.h 
      //apparently some SgInitializedNames have no p_char in SgName
      //printf("assigning node number %i to SgInitializedName %s\n", currentNumber+1,
      //                                          in->get_name().getString().c_str());
    }
    if ( !ir->getAttribute(astNode).exists("OANumber") ) {
      ir->getAttribute(astNode).add("OANumber", new SageNodeNumAttr(currentNumber));
      ir->nodeArrayPtr->push_back(astNode);
    }
  } 
  //TO DO
  //also need to visit SgInitializedName nodes under SgVariableDeclaration and SgFunctionParameterList
  //because SgSimpleProcessing does not visit those nodes
  /*
  SgVariableDeclaration * vardecl=isSgVariableDeclaration(astNode);
  SgFunctionParameterList * fparam;
  if(vardecl)
  {
    SgInitializedNamePtrList & arglist=vardecl->get_variables();
		for(SgInitializedNamePtrList::iterator argiter=arglist.begin(); 
						argiter!=arglist.end();++argiter)
		{
			SgInitializedName * in=*argiter;
      if(in)
      {
        in->attribute.add("OANumber", new SageNodeNumAttr(currentNumber));
        ir->nodeArray.push_back(in);
      }
		}
  }
  */
  /*if(fparam=isSgFunctionParameterList(astNode))
  {
    SgInitializedNamePtrList & arglist=fparam->get_args();
		for(SgInitializedNamePtrList::iterator argiter=arglist.begin(); 
						argiter!=arglist.end();++argiter)
		{
			SgInitializedName * in=*argiter;
      if(in)
      {
        in->attribute.add("OANumber", new SageNodeNumAttr(currentNumber));
        ir->nodeArray.push_back(in);
      }
		}
    
  }*/
}

void SageIRInterface::numberASTNodes(SgNode *astNode)
{
  // If we haven't already numbered this node, do it now.
  int currentNumber = nodeArrayPtr->size();

  if ( astNode == NULL )
    return;

  //  cout << "Visiting " << astNode->sage_class_name() << " " << astNode->unparseToCompleteString() << endl;
  //  cout << "Visiting " << astNode->sage_class_name() << " " << endl;

  // We've already been here.  Return to avoid a loop.
  if ( getAttribute(astNode).exists("OANumber") ) {
    //    cout << " returning" << endl;
    return;
  }

  //  cout << endl;

  if ( !getAttribute(astNode).exists("OANumber") ) {
    getAttribute(astNode).add("OANumber", new SageNodeNumAttr(currentNumber));
    nodeArrayPtr->push_back(astNode);
    currentNumber = nodeArrayPtr->size();
  }

  // Some nodes need special processing because the 
  // traversalSuccessorContainer mechanism does not visit all of their
  // children.
  // Aggregate such nodes in a children vector and then visit them.
  std::vector<SgNode *> children;

  switch (astNode->variantT() ) {

  case V_SgInitializedName:
    {
      // We need to explicitly handle cases like:
      // struct S { int *s1; int *s2; } s;
      // in which s1 and s2 aren't automatically visited.

      SgInitializedName *initializedName = 
	isSgInitializedName(astNode);
      ROSE_ASSERT(initializedName);

      SgType *type = initializedName->get_type();
      ROSE_ASSERT(type != NULL);

      SgNamedType *namedType = isSgNamedType(type);
      if ( namedType != NULL ) {
	children.push_back(namedType->get_declaration());
	children.push_back(initializedName->get_declaration());
	children.push_back(initializedName->get_definition());

	SgInitializer *initializer = initializedName->get_initializer();
	if ( initializer != NULL ) {
	  
	  SgConstructorInitializer *ctorInitializer =
	    isSgConstructorInitializer(initializer);

	  if ( ctorInitializer ) {

	    if ( !createsBaseType(ctorInitializer) ) {

	      // This function declaration won't get visited by the 
	      // traversal.
	      SgMemberFunctionDeclaration *functionDeclaration = 
		ctorInitializer->get_declaration();
	      ROSE_ASSERT(functionDeclaration != NULL);
	      
	      children.push_back(functionDeclaration);
	      
	      SgFunctionParameterList *parameterList = 
		functionDeclaration->get_parameterList(); 
	      
	      if ( parameterList != NULL )
		children.push_back(parameterList);
	      
	      SgClassDeclaration *classDeclaration =
		ctorInitializer->get_class_decl();
	      if ( classDeclaration != NULL)
		children.push_back(classDeclaration);
	    }	    

	  }
	  
	}
      }
      break;
    }

  case V_SgNewExp:
    {

      SgNewExp *newExp = isSgNewExp(astNode);
      ROSE_ASSERT(newExp != NULL);

      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      if ( !createsBaseType(ctorInitializer) ) {

	// This function declaration won't get visited by the 
	// traversal.
	SgMemberFunctionDeclaration *functionDeclaration = 
	  ctorInitializer->get_declaration();
	ROSE_ASSERT(functionDeclaration != NULL);
	
	children.push_back(functionDeclaration);
	
	// Do _not_ remove this next call to get_ctors().
	// It has side effects.  In particular, the initializerList
	// may be NULL before the call, but is always non-NULL
	// afterwards.  i.e., force it into existence now and number it.
	// Bad things could result if it is later conjured up,
	// since we won't have a number for it.
	SgInitializedNamePtrList &list = 
	  functionDeclaration->get_ctors(); 
	
	SgCtorInitializerList *initializerList =
	  functionDeclaration->get_CtorInitializerList();
	if ( initializerList != NULL ) {
	  children.push_back(initializerList);
	}
	
	SgFunctionParameterList *parameterList = 
	  functionDeclaration->get_parameterList(); 

	if ( parameterList != NULL )
	  children.push_back(parameterList);

      }

      SgClassDeclaration *classDeclaration =
	ctorInitializer->get_class_decl();
      if ( classDeclaration != NULL)
	children.push_back(classDeclaration);

      break;
    }

  case V_SgClassDefinition:
    {
      // Number all methods since some of them might be compiler-generated
      // constructors, and hence not show up elsewhere.

      SgClassDefinition *classDefinition =
	isSgClassDefinition(astNode);
      ROSE_ASSERT(classDefinition != NULL);

      SgDeclarationStatementPtrList &members = classDefinition->get_members(); 
      for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
	   it != members.end(); ++it) { 
	
	SgDeclarationStatement *declarationStatement = *it; 
	ROSE_ASSERT(declarationStatement != NULL);
	
	switch(declarationStatement->variantT()) {
	  
	case V_SgMemberFunctionDeclaration:
	  {
	    SgMemberFunctionDeclaration *functionDeclaration =  
	      isSgMemberFunctionDeclaration(declarationStatement); 
	    
	    if ( functionDeclaration != NULL ) { 
	      children.push_back(functionDeclaration);
	      
	      // Do _not_ remove this next call to get_ctors().
	      // It has side effects.  In particular, the initializerList
	      // may be NULL before the call, but is always non-NULL
	      // afterwards.  i.e., force it into existence now and number it.
	      // Bad things could result if it is later conjured up,
	      // since we won't have a number for it.
	      SgInitializedNamePtrList &list = 
		functionDeclaration->get_ctors(); 

	      SgCtorInitializerList *initializerList =
		functionDeclaration->get_CtorInitializerList();
	      if ( initializerList != NULL ) {
		children.push_back(initializerList);
	      }
	    }

	    break;
	  }

	default:
	  {
	    break;
	  }
	}

      }

      break;
    }

  case V_SgConstructorInitializer:
    {
      SgConstructorInitializer *ctorInitializer =
	isSgConstructorInitializer(astNode);
      ROSE_ASSERT(ctorInitializer != NULL);

      if ( !createsBaseType(ctorInitializer) ) {

	// Get the declaration of the function.
	SgFunctionDeclaration *functionDeclaration = 
	  ctorInitializer->get_declaration();
	ROSE_ASSERT(functionDeclaration != NULL);
	
	children.push_back(functionDeclaration);
	
	SgMemberFunctionDeclaration *memberFunctionDeclaration =  
	  isSgMemberFunctionDeclaration(functionDeclaration); 
	
	// Do _not_ remove this next call to get_ctors().
	// It has side effects.  In particular, the initializerList
	// may be NULL before the call, but is always non-NULL
	// afterwards.  i.e., force it into existence now and number it.
	// Bad things could result if it is later conjured up,
	// since we won't have a number for it.
	SgInitializedNamePtrList &list = 
	  memberFunctionDeclaration->get_ctors(); 
	
	if ( memberFunctionDeclaration != NULL ) { 
	  SgCtorInitializerList *initializerList =
	    memberFunctionDeclaration->get_CtorInitializerList();
	  if ( initializerList != NULL ) {
	    children.push_back(initializerList);
	  }
	}

      }

      break;
    }

  case V_SgFunctionDefinition:
    {
      SgFunctionDefinition *functionDefinition =
	isSgFunctionDefinition(astNode);
      ROSE_ASSERT(functionDefinition != NULL);

      //      cout << "Visiting at " << functionDefinition->unparseToCompleteString() << endl;

      SgFunctionDeclaration *functionDeclaration =
	functionDefinition->get_declaration();
      ROSE_ASSERT(functionDeclaration != NULL);

      SgMemberFunctionDeclaration *memberFunctionDeclaration =
	isSgMemberFunctionDeclaration(functionDeclaration);
      if ( memberFunctionDeclaration != NULL ) {
	// Do _not_ remove this next call to get_ctors().
	// It has side effects.  In particular, the initializerList
	// may be NULL before the call, but is always non-NULL
	// afterwards.  i.e., force it into existence now and number it.
	// Bad things could result if it is later conjured up,
	// since we won't have a number for it.
	SgInitializedNamePtrList &list = 
	  memberFunctionDeclaration->get_ctors(); 
	SgCtorInitializerList *initializerList =
	  memberFunctionDeclaration->get_CtorInitializerList();
	if ( initializerList != NULL ) {
	  //	  cout << "Visiting for " << functionDefinition->unparseToCompleteString() << endl;
	  children.push_back(initializerList);
	}

      }

      break;
    }
  case V_SgFunctionDeclaration:
  case V_SgMemberFunctionDeclaration:
    {
      SgFunctionDeclaration *functionDeclaration =
	isSgFunctionDeclaration(astNode);
      ROSE_ASSERT(functionDeclaration != NULL);

      SgFunctionParameterList *parameterList = 
	functionDeclaration->get_parameterList(); 

      children.push_back(parameterList);

      SgMemberFunctionDeclaration *memberFunctionDeclaration =
	isSgMemberFunctionDeclaration(astNode);
      if ( memberFunctionDeclaration != NULL ) {
	// Do _not_ remove this next call to get_ctors().
	// It has side effects.  In particular, the initializerList
	// may be NULL before the call, but is always non-NULL
	// afterwards.  i.e., force it into existence now and number it.
	// Bad things could result if it is later conjured up,
	// since we won't have a number for it.
	SgInitializedNamePtrList &list = 
	  memberFunctionDeclaration->get_ctors(); 
	SgCtorInitializerList *initializerList =
	  memberFunctionDeclaration->get_CtorInitializerList();
	if ( initializerList != NULL ) {
	  children.push_back(initializerList);
	}
      }

      break;
    }

  case V_SgTypedefDeclaration:
    {
      SgTypedefDeclaration *typedefDeclaration =
	isSgTypedefDeclaration(astNode);
      ROSE_ASSERT(typedefDeclaration != NULL);
      
      SgDeclarationStatement *definingDeclaration =
	typedefDeclaration->get_definingDeclaration();

      if ( definingDeclaration != NULL )
	children.push_back(definingDeclaration);

      SgType *baseType = typedefDeclaration->get_base_type();
      ROSE_ASSERT(baseType != NULL);

      SgNamedType *namedType = isSgNamedType(baseType);
      if ( namedType != NULL ) {
	children.push_back(namedType->get_declaration());
      }

      break;
      
    }

  default:
    {
      break;
    }

  }

  // Visit all children.
  vector<SgNode *> containerList = 
    astNode->get_traversalSuccessorContainer();
  
  // Iterate over all children.
  for(vector<SgNode *>::iterator it = containerList.begin();
      it != containerList.end(); ++it) {
    
    SgNode *node = *it;
    if ( node != NULL ) {
      numberASTNodes(node);
    }
    
  }

  for(vector<SgNode *>::iterator it = children.begin();
      it != children.end(); ++it) {
    
    SgNode *node = *it;
    if ( node != NULL ) {
      numberASTNodes(node);
    }
    
  }

}

void SageIRInterface::createNodeArray(SgNode * root)
{
  if(nodeArrayPtr->size()==0)
  {
#if 1
    // The pre-defined Sage traversals skip over nodes that
    // we need to be numbered.  Therefore, manually traverse the AST.
    numberASTNodes(root);
    //    NumberTraversal t(this);
#else
    NumberTraversal t(this);
    t.traverse(root, preorder);
#endif
    //    printf("created nodeArray of size %i\n", nodeArrayPtr->size());
  }
}

long SageIRInterface::getNodeNumber(SgNode * n) //can be zero
{
  if(n==NULL)
  {
    //    cerr << "NULL ptr in getNodeNumber" << endl;
    return 0;
  }
  else if(persistent_handles)
  {
    if(getAttribute(n).exists("OANumber"))
    {
      SageNodeNumAttr * attr=dynamic_cast<SageNodeNumAttr * >(getAttribute(n)["OANumber"]);
      return (attr->number)+1;
    }
    else {
      cerr << "Could not find persistent handle for " << n->sage_class_name() << endl;
      ROSE_ABORT();
      return 0;
    }
  }
  else
    return (long)n;
}


//various toString methods

std::string SageIRInterface::toString(const OA::ProcHandle h) 
{
  SgNode *node = getNodePtr(h);

  if (node == NULL) {
    return("warning: ProcHandle is 0  ");
  }

  SgName nm;
  std::string ret;
  
  switch(node->variantT()) {
    
  case V_SgFunctionDefinition:
    {
      SgFunctionDefinition *functionDefinition = isSgFunctionDefinition(node);
      ROSE_ASSERT(functionDefinition != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	functionDefinition->get_declaration();
      ROSE_ASSERT(functionDefinition != NULL);

      nm = functionDeclaration->get_name();
      
      SgMemberFunctionDeclaration *fd = 
	isSgMemberFunctionDeclaration(functionDeclaration);
      if (fd != NULL) {
	nm = fd->get_qualified_name();
      }

      ret = nm.str();

      break;
    }

  case V_SgFunctionDeclaration:
    {
      SgFunctionDeclaration *fd = isSgFunctionDeclaration(node);
      ROSE_ASSERT(fd != NULL);

      nm = fd->get_name();
      ret = nm.str();

      break;
    }

  case V_SgMemberFunctionDeclaration:
    {
      SgMemberFunctionDeclaration *fd = isSgMemberFunctionDeclaration(node);
      ROSE_ASSERT(fd != NULL);

      //      nm = fd->get_name();
      nm = fd->get_qualified_name();
      ret = nm.str();

      break;
    }

  default:
    {
      ret = node->sage_class_name();
      ROSE_ABORT();
      break;
    }

  }

  return ret;
}

std::string SageIRInterface::toString(const OA::MemRefHandle h)
{
  std::string strdump;
  //  char val[20];
  if(h.hval()==0) {
    strdump="NULL mem ref ";
    return strdump;
  }

  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);
  
  SgExpression *expression = isSgExpression(node);
  SgInitializedName *initializedName = isSgInitializedName(node);

  if ( expression != NULL ) {
    SgConstructorInitializer *ctorInitializer =
      isSgConstructorInitializer(expression);
    if ( ctorInitializer ) {
      SgMemberFunctionDeclaration *constructor =
	ctorInitializer->get_declaration();
      ROSE_ASSERT(constructor != NULL);
      SgNode *scope = constructor->get_scope();
      ROSE_ASSERT(scope != NULL);
      SgClassDefinition *classDefinition = isSgClassDefinition(scope);
      ROSE_ASSERT(classDefinition != NULL);

      strdump = classDefinition->get_qualified_name().str() + 
	expression->unparseToString();
    } else {
      strdump = expression->unparseToString();
    }
  } else if ( initializedName != NULL ) {
    
    SgType *type = initializedName->get_type();
    ROSE_ASSERT(type != NULL);
    
    // If an initialized name is a reference, then we model
    // it is a pointer.
    // e.g., int &lhs = ... -> int *lhs = ...
    // However, subsequent assignments (different from initializations)
    // of lhs change the value at the location not of the location:
    // e.g. lhs = 5 -> *lhs = 5
    // In OpenAnalysis, *lhs will result in two mem ref expressions:
    // *lhs and the sub mem ref expr lhs.  We wish to differentiate
    // between this "sub" lhs and the above lhs in the initialization.
    // Therefore, we will annotate the lhs initialization here with
    // an "&":
    if ( isSgReferenceType(type) ) {
      strdump = initializedName->get_name().getString() + "&";
    } else {
      strdump = initializedName->get_name().getString();
    }
  } else {
    ROSE_ABORT();
  }

  // Let's not print the memory address.  This creates problems
  // comparing results of regression tests.
  //    strdump+=val;
  return strdump;
}

std::string SageIRInterface::toString(const OA::CallHandle h) 
{
  SgNode *node = getNodePtr(h);
  std::string retstr;

  switch(node->variantT()) {
  case V_SgFunctionCallExp:
    {
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      ROSE_ASSERT(functionCallExp != NULL);
      retstr = functionCallExp->unparseToString();
      break;
    }
  case V_SgConstructorInitializer:
    {
      SgConstructorInitializer *ctorInitializer = 
	isSgConstructorInitializer(node);
      ROSE_ASSERT(ctorInitializer != NULL);
      SgNode *parent = ctorInitializer->get_parent();
      ROSE_ASSERT(parent != NULL);
      switch(parent->variantT()) {
      case V_SgNewExp:
	{
	  SgNewExp *newExp = isSgNewExp(parent);
	  ROSE_ASSERT(newExp != NULL);
	  retstr = newExp->unparseToString();
	  break;
	}
      case V_SgInitializedName:
	{
	  SgInitializedName *initName = isSgInitializedName(parent);
	  ROSE_ASSERT(initName != NULL);
	  
	  retstr = initName->get_name().str();
	  break;
	}
      default:
	{
	  ROSE_ABORT();
	  break;
	}
      }
      break;
    }
  default:
    {
      ROSE_ABORT();
    }
  }
  return retstr;
}

std::string SageIRInterface::toString(const OA::SymHandle h) 
{
  SgNode *node = getNodePtr(h);

  if (node == NULL) {
    return("warning: SymHandle is 0  ");
  }

  SgName nm;
  std::string ret;
  
  switch(node->variantT()) {
    
  case V_SgFunctionDeclaration:
    {
      SgFunctionDeclaration *fd = isSgFunctionDeclaration(node);
      ROSE_ASSERT(fd != NULL);

      nm = fd->get_name();
      ret = nm.str();

      break;
    }

  case V_SgMemberFunctionDeclaration:
    {
      SgMemberFunctionDeclaration *fd = isSgMemberFunctionDeclaration(node);
      ROSE_ASSERT(fd != NULL);

      //      nm = fd->get_name();
      nm = fd->get_qualified_name() + "__" + fd->get_mangled_name();
      ret = string("method:") + nm.str();

      break;
    }

  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL);

      nm = initName->get_name();

      if (nm.str() == NULL) {
	// This occurs when using varags or in a function
	// prototype which does not name its args.
	SgNode *parent = initName->get_parent();
	ROSE_ASSERT(isSgFunctionParameterList(parent));
	ret = "anonymousArg";
      } else {

	SgDeclarationStatement *declarationStmt = 
	  initName->get_declaration();
	ROSE_ASSERT(declarationStmt != NULL);

	SgFunctionDefinition *functionDefinition = 
	  getEnclosingMethod(declarationStmt);

	if (functionDefinition == NULL) {

	  //	  ret = string("global: " ) + nm.str();
	  ret = nm.str();

	} else {

	  SgFunctionDeclaration *functionDeclaration = 
	    functionDefinition->get_declaration();
	  ROSE_ASSERT(functionDefinition != NULL);
	  
	  SgName funcName;
	  funcName = functionDeclaration->get_name();
	  
	  SgMemberFunctionDeclaration *fd = 
	    isSgMemberFunctionDeclaration(functionDeclaration);
	  if (fd != NULL) {
	    funcName = fd->get_qualified_name();
	  }
	  
	  //	  ret = nm.str() + string(" defined in: ") + funcName.str();
	  ret = nm.str();
	}
      }

      break;
    }

  case V_SgThisExp:
    {
      ret = "this";
      break;
    }

  case V_SgClassSymbol:
    {
      SgClassSymbol *classSymbol = isSgClassSymbol(node);
      ROSE_ASSERT(classSymbol != NULL);

      nm = classSymbol->get_name();
      ret = string("SgClassSymbol: ") + nm.str();

      break;
    }
#if 0
  case V_SgFunctionDefinition:
    {
      // If we see a SgFunctionDefinition where we expected a symbol,
      // it means that the symbol represents a 'this' pointer.
      SgFunctionDefinition *functionDefinition = isSgFunctionDefinition(node);
      ROSE_ASSERT(functionDefinition != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	functionDefinition->get_declaration();
      ROSE_ASSERT(functionDefinition != NULL);

      nm = functionDeclaration->get_name() + "__" + functionDeclaration->get_mangled_name();

      SgMemberFunctionDeclaration *fd = 
	isSgMemberFunctionDeclaration(functionDeclaration);
      if (fd != NULL) {
	nm = fd->get_qualified_name() + "__" + fd->get_mangled_name();
      }

      ret = string("this: ") + nm.str();

      break;
    }
#endif
#if 1
  case V_SgFunctionParameterList:
    {
      // If we see a SgFunctionDefinition where we expected a symbol,
      // it means that the symbol represents a 'this' pointer.
      SgFunctionParameterList *parameterList = 
	isSgFunctionParameterList(node);
      ROSE_ASSERT(parameterList != NULL);

      SgNode *parent = parameterList->get_parent();
      ROSE_ASSERT(parent != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	isSgFunctionDeclaration(parent);
      ROSE_ASSERT(functionDeclaration != NULL);
	  
      SgName funcName;
      funcName = functionDeclaration->get_name() + "__" + 
	functionDeclaration->get_mangled_name();

      SgMemberFunctionDeclaration *fd = 
	isSgMemberFunctionDeclaration(functionDeclaration);
      ROSE_ASSERT(fd != NULL);

      SgClassDefinition *classDefinition = isSgClassDefinition(fd->get_scope());
      ROSE_ASSERT(classDefinition != NULL);

      SgName qualifiedName;
      qualifiedName = classDefinition->get_qualified_name();

      ret = string("this::") + qualifiedName.str() + "::" + funcName.str();
      
      break;
    }
#else
  case V_SgCtorInitializerList:
    {
      // If we see a SgCtorIntializerList where we expected a symbol,
      // it means that the symbol represents a 'this' pointer.
      SgCtorInitializerList *ctorInitializer = 
	isSgCtorInitializerList(node);
      ROSE_ASSERT(ctorInitializer != NULL);

      SgNode *parent = ctorInitializer->get_parent();
      ROSE_ASSERT(parent != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	isSgFunctionDeclaration(parent);
      ROSE_ASSERT(functionDeclaration != NULL);
	  
      SgName funcName;
      funcName = functionDeclaration->get_name() + "__" + 
	functionDeclaration->get_mangled_name();

      SgMemberFunctionDeclaration *fd = 
	isSgMemberFunctionDeclaration(functionDeclaration);
      ROSE_ASSERT(fd != NULL);

      SgClassDefinition *classDefinition = isSgClassDefinition(fd->get_scope());
      ROSE_ASSERT(classDefinition != NULL);

      SgName qualifiedName;
      qualifiedName = classDefinition->get_qualified_name();

      ret = string("this::") + qualifiedName.str() + "::" + funcName.str();
      
      break;
    }
#endif
#if 0
  case V_SgFunctionParameterList:
    {
      // If we see a SgFunctionParameterList where we expected a symbol,
      // it means that the symbol represents the return slot of
      // a function. 
      SgFunctionParameterList *parameterList = 
	isSgFunctionParameterList(node);
      ROSE_ASSERT(parameterList != NULL);

      SgNode *parent = parameterList->get_parent();
      ROSE_ASSERT(parent != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	isSgFunctionDeclaration(parent);
      ROSE_ASSERT(functionDeclaration != NULL);
	  
      SgName funcName;
      funcName = functionDeclaration->get_name() + "__" + 
	functionDeclaration->get_mangled_name();

      SgMemberFunctionDeclaration *fd = 
	isSgMemberFunctionDeclaration(functionDeclaration);
      ROSE_ASSERT(fd != NULL);

      SgClassDefinition *classDefinition = isSgClassDefinition(fd->get_scope());
      ROSE_ASSERT(classDefinition != NULL);

      SgName qualifiedName;
      qualifiedName = classDefinition->get_qualified_name();

      ret = string("ret::") + qualifiedName.str() + "::" + funcName.str();
      
      break;
    }
#endif
  case V_SgMemberFunctionType:
  case V_SgFunctionType:
    {
      SgFunctionType *functionType = isSgFunctionType(node);
      ROSE_ASSERT(functionType != NULL);

      // We model a function's return slot with its 
      // SgFunctionType.

      ROSE_ABORT();
      break;
    }
  default:
    {
      ret = node->sage_class_name();
      break;
    }

  }

  return ret; 
}

std::string SageIRInterface::toString(OA::Alias::IRStmtType x)
{
  //printf("implement SageIRInterface::toString(OA::Alias::IRStmtType x)\n");
  std::string ret;
  switch(x)
  {
    case OA::Alias::PTR_ASSIGN_STMT:
      ret="pointer assignment\n";
      break;
    case OA::Alias::ANY_STMT:
      ret="not a pointer assignment\n";
      break;
    default:
      ret="unknown\n";
  }
  return ret;
}
  
//-------------------------------------------------------------------------
// AliasIRInterfaceDefault
//-------------------------------------------------------------------------

// Create the lhs/rhs pairs corresponding to pointer assignments
// for a return statement.
void 
SgPtrAssignPairStmtIterator::createPtrAssignPairsFromReturnStmt(SgReturnStmt *returnStmt)
{
  SgExpression *returnExpr = returnStmt->get_return_expr();
  ROSE_ASSERT(returnExpr != NULL);

  SgFunctionDefinition *enclosingFunction =
    mIR->getEnclosingMethod(returnStmt);
  ROSE_ASSERT(enclosingFunction != NULL);

  SgFunctionDeclaration *functionDeclaration =
    enclosingFunction->get_declaration();
  ROSE_ASSERT(functionDeclaration != NULL);

  // Create the lhs to represent the method declaration.
  bool addressTaken = false;
  bool fullAccuracy = true;
  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::DEF;
  OA::SymHandle symHandle;

  symHandle = mIR->getProcSymHandle(functionDeclaration);
  //  symHandle = mIR->getFunctionReturnSlotSymbol(functionDeclaration);

  OA::OA_ptr<OA::MemRefExpr> function;
  function = new OA::NamedRef(addressTaken, 
			    fullAccuracy,
			    memRefType,
			    symHandle);

#if 0
  // We don't need to do this anymore-- I changed the symbol used
  // in MREs to represent rets to use something that wouldn't
  // conflict, a SgCtorInitializerList.

  // NB: Dereference the (functionDeclaration) MRE.
  //     Otherwise, were we to return a non-deref'ed
  //     functionDeclaration, it would be aliased with 
  //     the functionDeclaration MREs returned via the
  //     implicit ptr assigns.
  function = mIR->dereferenceMre(function);
#endif

  // Create the rhs from the return expression.
  memRefType = OA::MemRefExpr::USE;

  // Find the top (i.e., first) MemRefNode on the LHS.
  list<SgNode *>* topMemRefs;
  
  topMemRefs = mIR->findTopMemRefs(returnExpr);
  ROSE_ASSERT(topMemRefs != NULL);
      
  // On the RHS we may have multiple top MemRefHandles.
  // e.g., int *b; int *c; ret ( cond ? b : c );
  // has MemRefHandles for b and c on the RHS.  Create
  // a pairing for each.
  ROSE_ASSERT(topMemRefs->size() >= 1);
  
  for (list<SgNode *>::iterator it = topMemRefs->begin();
       it != topMemRefs->end(); ++it) {
	
    SgNode *rhsMemRefNode = *it;
    ROSE_ASSERT(rhsMemRefNode != NULL);

    rhsMemRefNode = mIR->lookThroughCastExpAndAssignInitializer(rhsMemRefNode);
    ROSE_ASSERT(mIR->isMemRefNode(rhsMemRefNode));
      
    // Create the lhs/rhs MemRefHandles.
    OA::MemRefHandle rhsHandle = mIR->getNodeNumber(rhsMemRefNode);

    OA::OA_ptr<OA::MemRefExprIterator> rhsMreIterPtr 
      = mIR->getMemRefExprIterator(rhsHandle);
	    
    for (; rhsMreIterPtr->isValid(); (*rhsMreIterPtr)++) {
      
      OA::OA_ptr<OA::MemRefExpr> rhsMre = rhsMreIterPtr->current();
	      
      mMemRefList.push_back(pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> >(function, rhsMre));

#if 0
      std::cout << "Creating return ptr assign for node types " << functionDeclaration->sage_class_name() << " and " << rhsMemRefNode->sage_class_name() << std::endl;
      OA::OA_ptr<OA::IRHandlesIRInterface> interface;
      SageIRInterface *ir = new SageIRInterface(mIR->getProject(), NULL, FALSE);
      interface = ir;
      std::cout << "lhs mre = " << std::endl;
      function->output(interface);
#endif     
 
      mIR->createImplicitPtrAssigns(function, rhsMre, returnExpr, &mMemRefList); 

    }
      
  }
}

// Create lhs/rhs pairs corresponding to pointer assignments
// within the tree rooted at an assign.
SgExpression *
SgPtrAssignPairStmtIterator::createPtrAssignPairsFromAssignment(SgNode *node)
{
  if ( node == NULL ) return NULL;

  SgExpression *lhs = NULL;
  SgExpression *rhs = NULL;
  std::vector<SgExpression *> rhses;

  SgExpression *exprVal = NULL;

  switch(node->variantT()) {
#if 0
  case V_SgCommaOpExp:
    {
      // The result of evaluating a comma expression is the right-hand side.
      // So only return the rhs' MRE.
      SgCommaOpExp *commaOp = isSgCommaOpExp(node);
      ROSE_ASSERT(commaOp != NULL);

      lhs = commaOp->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      rhs = commaOp->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      rhses.push_back(rhs);

      createPtrAssignPairsFromAssignment(lhs);
      
      exprVal = createPtrAssignPairsFromAssignment(rhs);

      break;
    }
#endif
  case V_SgAssignOp:
#if 0
    // hmm ... On second thought, I don't think these belong.  bwhite
  case V_SgAndAssignOp:
  case V_SgDivAssignOp:
  case V_SgIorAssignOp:
  case V_SgMinusAssignOp:
  case V_SgModAssignOp:
  case V_SgMultAssignOp:
  case V_SgPlusAssignOp:
  case V_SgXorAssignOp:
#endif
    {
      SgBinaryOp *assign = isSgBinaryOp(node);
      ROSE_ASSERT(assign != NULL);

      lhs = assign->get_lhs_operand();
      ROSE_ASSERT(lhs != NULL);

      rhs = assign->get_rhs_operand();
      ROSE_ASSERT(rhs != NULL);

      SgType *lhsType = lhs->get_type();
      ROSE_ASSERT(lhsType != NULL);

      SgType *baseType = mIR->getBaseType(lhsType);
      ROSE_ASSERT(baseType != NULL);
      if ( !isSgPointerType(baseType) && !isSgReferenceType(baseType) ) 
	return lhs;

      exprVal = lhs;

      switch(rhs->variantT()) {
      case V_SgAssignOp:
#if 0
      case V_SgAndAssignOp:
      case V_SgDivAssignOp:
      case V_SgIorAssignOp:
      case V_SgMinusAssignOp:
      case V_SgModAssignOp:
      case V_SgMultAssignOp:
      case V_SgPlusAssignOp:
      case V_SgXorAssignOp:
#endif
	{
	  SgAssignOp *assignOp = isSgAssignOp(rhs);
	  ROSE_ASSERT(assignOp != NULL);

	  rhs = assignOp->get_lhs_operand();
	  ROSE_ASSERT(rhs != NULL);

	  createPtrAssignPairsFromAssignment(assignOp);

	  rhses.push_back(rhs);

	  break;
	}
      case V_SgCommaOpExp:
	{
	  SgCommaOpExp *commaOpExp = isSgCommaOpExp(rhs);
	  ROSE_ASSERT(commaOpExp != NULL);

	  // Either the lhs or the rhs could be an assignment.
	  rhs = createPtrAssignPairsFromAssignment(commaOpExp->get_lhs_operand());
	  createPtrAssignPairsFromAssignment(commaOpExp->get_rhs_operand());

	  ROSE_ASSERT(rhs != NULL);

	  rhses.push_back(rhs);

	  break;
	}
      case V_SgConditionalExp:
	{
	  SgConditionalExp *conditionalExp = isSgConditionalExp(rhs);
	  ROSE_ASSERT(conditionalExp != NULL);

	  rhs = isSgExpression(mIR->lookThroughCastExpAndAssignInitializer(conditionalExp->get_true_exp()));
	  ROSE_ASSERT(rhs != NULL);
	  SgType *type = rhs->get_type();
	  SgType *baseType = mIR->getBaseType(type);
	  ROSE_ASSERT(baseType != NULL);
	  if ( isSgPointerType(baseType) || isSgReferenceType(baseType) )
	    rhses.push_back(rhs);

	  rhs = isSgExpression(mIR->lookThroughCastExpAndAssignInitializer(conditionalExp->get_false_exp()));
	  ROSE_ASSERT(rhs != NULL);
	  type = rhs->get_type();
	  baseType = mIR->getBaseType(type);
	  ROSE_ASSERT(baseType != NULL);
	  if ( isSgPointerType(baseType) || isSgReferenceType(baseType) )
	    rhses.push_back(rhs);

	  exprVal = rhs;

	  break;
	}
      default:
	{
	  // The lhs is a pointer type and the rhs is not an assign.
	  rhses.push_back(rhs);

	  break;
	}
      }
	
      // We have a lhs/rhs pair involved in a pointer assigment.
      // Ensure that these are the type of nodes to which we
      // normally assign MemRefHandles.
      
      // Find the top (i.e., first) MemRefNode on the LHS.
      list<SgNode *>* topMemRefs;
      
      topMemRefs = mIR->findTopMemRefs(lhs);
      ROSE_ASSERT(topMemRefs != NULL);
      
      // We are only expecting one on the LHS.
      ROSE_ASSERT(topMemRefs->size() == 1);
      SgNode *lhsMemRefNode = *(topMemRefs->begin());
      ROSE_ASSERT(lhsMemRefNode != NULL);
      
      lhsMemRefNode = mIR->lookThroughCastExpAndAssignInitializer(lhsMemRefNode);
      ROSE_ASSERT(mIR->isMemRefNode(lhsMemRefNode));
      
      for (int i = 0; i < rhses.size(); ++i) {

	rhs = rhses[i];
	ROSE_ASSERT(rhs != NULL);

	// Find the top (i.e., first) MemRefNode on the RHS.
	topMemRefs = mIR->findTopMemRefs(rhs);
	ROSE_ASSERT(topMemRefs != NULL);
      
	// On the RHS we may have multiple top MemRefHandles.
	// e.g., int *a; int *b; int *c; a = ( cond ? b : c );
	// has MemRefHandles for b and c on the RHS.  Create
	// a pairing for each.
	// Could be none on the RHS (e.g., int *x; *x = 0; )
	// ROSE_ASSERT(topMemRefs->size() >= 1);
	for (list<SgNode *>::iterator it = topMemRefs->begin();
	     it != topMemRefs->end(); ++it) {
	  SgNode *rhsMemRefNode = *it;
	  ROSE_ASSERT(rhsMemRefNode != NULL);
	  
	  rhsMemRefNode = mIR->lookThroughCastExpAndAssignInitializer(rhsMemRefNode);
	  ROSE_ASSERT(mIR->isMemRefNode(rhsMemRefNode));

	  // Create the lhs/rhs MemRefHandles.
	  OA::MemRefHandle lhsHandle = mIR->getNodeNumber(lhsMemRefNode);
	  OA::MemRefHandle rhsHandle = mIR->getNodeNumber(rhsMemRefNode);
	  
	  // Save this lhs/rhs pair.
	  //	  mMemRefList.push_back(pair<OA::MemRefHandle, OA::MemRefHandle>(lhsHandle, rhsHandle));
	  
	  OA::OA_ptr<OA::MemRefExprIterator> lhsMreIterPtr 
            = mIR->getMemRefExprIterator(lhsHandle);
	  // for each mem-ref-expr associated with this memref
	  for (; lhsMreIterPtr->isValid(); (*lhsMreIterPtr)++) {
	    
	    OA::OA_ptr<OA::MemRefExpr> lhsMre = lhsMreIterPtr->current();
	    
	    OA::OA_ptr<OA::MemRefExprIterator> rhsMreIterPtr 
	      = mIR->getMemRefExprIterator(rhsHandle);
	    
	    for (; rhsMreIterPtr->isValid(); (*rhsMreIterPtr)++) {
	      
	      OA::OA_ptr<OA::MemRefExpr> rhsMre = rhsMreIterPtr->current();

#if 0
	      // We don't need to do this anymore-- I changed the symbol used
	      // in MREs to represent rets to use something that wouldn't
	      // conflict, a SgCtorInitializerList.

	      // If the rhs is a function call exp, it returns an
	      // address, and we are modeling the assignment from its
	      // return value to the lhs of an assign.  We need this
	      // to be consistent with the MRE creating in
	      // createPtrAssignPairsFromReturnStmt, which 
	      // dereferences a symbol derived from a SgFunctionDeclaration
	      // to represent the "return slot" the corresponding function.
	      // The symbol in the MRE associated with a function call
	      // expression will also be a SgFunctionDeclaration.
	      // To make them consistent we need to dereference it.
	      // NB:  we must dereference the SgFunctionDeclaration MRE
	      //      else it will collide with the implict ptr assigns
	      //      used to model virtual methods (also 
	      //      SgFunctionDeclaration-based MREs).  This would
	      //      result in unintended alias pairs.
	      if ( isSgFunctionCallExp(rhsMemRefNode) ) {
		rhsMre = mIR->dereferenceMre(rhsMre);
	      }
#endif

#if 0
	      std::cout << "Creating ptr assign for node types " << lhsMemRefNode->sage_class_name() << " and " << rhsMemRefNode->sage_class_name() << std::endl;
	      std::cout << lhsMemRefNode->unparseToCompleteString() << std::endl;
	      std::cout << rhsMemRefNode->unparseToCompleteString() << std::endl;
	      OA::OA_ptr<OA::IRHandlesIRInterface> interface;
	      SageIRInterface *ir = new SageIRInterface(mIR->getProject(), NULL, FALSE);
	      interface = ir;
	      std::cout << "lhs mre = " << std::endl;
	      lhsMre->output(interface);
	      std::cout << "rhs mre = " << std::endl;
	      rhsMre->output(interface);
#endif

	      mMemRefList.push_back(pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> >(lhsMre, rhsMre));
	      
	      mIR->createImplicitPtrAssigns(lhsMre, rhsMre, rhs, &mMemRefList); 
	      
	    }
	    
	  }
	  
	}

      }
      
      break;
    }
  default:
    {
      exprVal = isSgExpression(node);
      break;
    }
  }

  return exprVal;
}

void
SgPtrAssignPairStmtIterator::reset()
{
  mIter = mMemRefList.begin();
  mEnd = mMemRefList.end();
  mBegin = mMemRefList.begin();
}

void
SgParamBindPtrAssignIterator::reset()
{
  mIter = mPairList.begin();
  mEnd = mPairList.end();
  mBegin = mPairList.begin();
}

SgTypePtrList &
SageIRInterface::getFormalTypes(SgNode *node)
{
  ROSE_ASSERT(node != NULL);

  SgFunctionType *functionType = NULL;

  switch(node->variantT()) {
  case V_SgFunctionCallExp:
    {
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      ROSE_ASSERT(functionCallExp != NULL);

      SgPointerDerefExp *pointerDerefExp =
	isFunctionPointer(functionCallExp);

      if ( pointerDerefExp ) {

	SgType *type = pointerDerefExp->get_type();
	ROSE_ASSERT(type != NULL);

	functionType = isSgFunctionType(type);
	ROSE_ASSERT(functionType != NULL);

      } else {

	SgFunctionDeclaration *functionDeclaration = 
	  getFunctionDeclaration(functionCallExp); 
	ROSE_ASSERT(functionDeclaration != NULL);

	functionType = functionDeclaration->get_type();
	ROSE_ASSERT(functionType != NULL);

      }

      break;
    }
#if 0
  case V_SgNewExp:
    {
      SgNewExp *newExp = isSgNewExp(node);
      ROSE_ASSERT(newExp != NULL);

      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	ctorInitializer->get_declaration();
      ROSE_ASSERT(functionDeclaration != NULL);

      functionType = functionDeclaration->get_type();
      ROSE_ASSERT(functionType != NULL);

      break;

    }
#endif
  case V_SgConstructorInitializer:
    {
      SgConstructorInitializer *ctorInitializer =
	isSgConstructorInitializer(node);
      ROSE_ASSERT(ctorInitializer != NULL);

      SgFunctionDeclaration *functionDeclaration = 
	ctorInitializer->get_declaration();
      ROSE_ASSERT(functionDeclaration != NULL);

      functionType = functionDeclaration->get_type();
      ROSE_ASSERT(functionType != NULL);

      break;

    }
  default:
    {
      //      cerr << "Call must be a SgFunctionCallExp or a SgNewExp, got a" << endl;
      cerr << "Call must be a SgFunctionCallExp or a SgConsructorInitializer, got a" << endl;
      cerr << node->sage_class_name() << endl;
      ROSE_ABORT();
      break;
    }
  }

  ROSE_ASSERT(functionType != NULL);
  return functionType->get_arguments();
}

#if 1
// Create iterator consisting of pairs of procedure formal SymHandles
// and procedure call actual MemRefHandles.  A pair is created
// only for pointer or reference formals.
void SgParamBindPtrAssignIterator::create(OA::ExprHandle call)
{
  SgNode *node = mIR->getNodePtr(call);
  ROSE_ASSERT(node != NULL);
  
  SgFunctionDeclaration *functionDeclaration = NULL;
  //  SgNode *lhs = NULL;

  bool isDotExp     = false;
  bool isMethod = false;

  switch(node->variantT()) {

  case V_SgFunctionCallExp:
    {      
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      ROSE_ASSERT(functionCallExp != NULL);
      
      isMethod = mIR->isMethodCall(functionCallExp, isDotExp);
#if 0
      if ( isMethod ) {
	lhs = mIR->getMethodInvocationLhs(functionCallExp);
	ROSE_ASSERT(lhs != NULL);
	lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);
      }
#endif
      break;
    }
#if 0
  case V_SgNewExp:
    {
      isMethod = true;
#if 0
      SgNewExp *newExp = isSgNewExp(node);
      ROSE_ASSERT(newExp != NULL);

      lhs = mIR->getNewLhs(newExp);
      if ( lhs != NULL ) {
	lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);
      }
#endif
      break;
    }
#endif
  case V_SgConstructorInitializer:
    {
      isMethod = true;
      SgNode *parent = node->get_parent();
      ROSE_ASSERT(parent != NULL);
      if ( isSgInitializedName(parent) ) {
	// If the parent of the constructor initialize is a var, then
	// we are in the case:
	// A a;
	// rather than:
	// A a = new A;
	// Therefore, in the implicit param binding with 'this', we
	// need to treat this as if it were a dot expression, so
	// that we take the address of a:  this = &a.
	isDotExp = true;
      }
      break;
    }
  default:
    {
      //      cerr << "Call must be a SgFunctionCallExp or a SgNewExp, got a" << endl;
      cerr << "Call must be a SgFunctionCallExp or a SgConstructorInitializer, got a" << endl;
      cerr << node->sage_class_name() << endl;
      ROSE_ABORT();
      break;
    }
  }

  SgTypePtrList &typePtrList = mIR->getFormalTypes(node);

  OA::OA_ptr<OA::IRCallsiteParamIterator> actualsIter = 
    mIR->getCallsiteParams(call);

  int paramNum = 0;

  if ( ( paramNum == 0 ) && ( isMethod ) ) {

    bool handleThisExp = true;

    ROSE_ASSERT(actualsIter->isValid());

    OA::ExprHandle actualExpr = actualsIter->current(); 
    SgNode *actualNode = mIR->getNodePtr(actualExpr);
    // actualExpr could be NULL if expression was 'return (new B)'--
    // i.e., no lhs.
    //    ROSE_ASSERT(actualNode != NULL);
    
    // We fold the object upon which a method is invoked into
    // the argument list (as the first actual) of a method
    // invocation.  If this is the first arg of a method
    // invocation we need special processing for the 'this'
    // expression.
    
    OA::MemRefHandle actualMemRefHandle = (OA::MemRefHandle)0;
    
    bool isArrowExp = false;
    if ( actualNode != NULL ) {
      actualNode = mIR->lookThroughCastExpAndAssignInitializer(actualNode);
      if ( mIR->isMemRefNode(actualNode) )
	actualMemRefHandle = mIR->getNodeNumber(actualNode);
    }

    if ( actualMemRefHandle == (OA::MemRefHandle)0 ) {
      
      // This isn't true.  Consider that printf's first formal is
      // a const char *.  We may pass it an actual string, which
      // is not a MemRefNode.
      
    } else {
      
      OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
	= mIR->getMemRefExprIterator(actualMemRefHandle);
      
      // for each mem-ref-expr associated with this memref
      for (; actualMreIterPtr->isValid(); (*actualMreIterPtr)++) {
	
	OA::OA_ptr<OA::MemRefExpr> actualMre;
	
	if ( ( handleThisExp == true ) && ( isDotExp == true ) ) {
	  
	  // We are returning a MemRefExpr representing the object b
	  // upon which a method is invoked in the expression b.foo()
	  // (i.e., a dot expression).  This pair is intended to
	  // represent the implicit binding between the 'this' pointer
	  // and this object.  Since 'this' is a pointer, we should
	  // bind it to the address of b.
	  
	  OA::OA_ptr<OA::MemRefExpr> curr = actualMreIterPtr->current();
	  actualMre = curr->clone();
	  actualMre->setAddressTaken(true);
	  
	} else { 
	  
	  actualMre = actualMreIterPtr->current();
	  
	}
	
	
	mPairList.push_back(pair<int, OA::OA_ptr<OA::MemRefExpr> >(paramNum, actualMre));
	
      }
      
    }
    
    (*actualsIter)++; 
    paramNum++;

  }
    
  // Handle the implicit this parameter outside of the loop.
  // Note that the implicit actual is included in actualsIter,
  // though the formal is not represented in typePtrList.

  // Simultaneously iterate over both formals and actuals.
  // If the formal is a reference parameter (i.e., 
  // a pointer or a reference) store it in the iterator's list.
  SgTypePtrList::iterator formalIt = typePtrList.begin();
  for ( ; actualsIter->isValid(); (*actualsIter)++ ) { 
    
    bool handleThisExp = false;
    bool treatAsPointerParam = false;

    SgType *type = NULL;
    if ( formalIt != typePtrList.end() ) {
      type = *formalIt;
    }
    
    if ( isSgReferenceType(type) || isSgPointerType(type) ) {
      treatAsPointerParam = true;
    }
    
    if ( treatAsPointerParam ) {

      OA::ExprHandle actualExpr = actualsIter->current(); 
      SgNode *actualNode = mIR->getNodePtr(actualExpr);
      ROSE_ASSERT(actualNode != NULL);

      // We fold the object upon which a method is invoked into
      // the argument list (as the first actual) of a method
      // invocation.  If this is the first arg of a method
      // invocation we need special processing for the 'this'
      // expression.

      OA::MemRefHandle actualMemRefHandle = (OA::MemRefHandle)0;

      bool isArrowExp = false;

      // Ensure that this expression is actually represented 
      // by a MemRefHandle.  
      actualNode = mIR->lookThroughCastExpAndAssignInitializer(actualNode);
      //	ROSE_ASSERT(mIR->isMemRefNode(actualNode));
      
      if ( mIR->isMemRefNode(actualNode) )
	actualMemRefHandle = mIR->getNodeNumber(actualNode);
      
      if ( actualMemRefHandle == (OA::MemRefHandle)0 ) {

	// This isn't true.  Consider that printf's first formal is
	// a const char *.  We may pass it an actual string, which
	// is not a MemRefNode.

      } else {

	OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
	  = mIR->getMemRefExprIterator(actualMemRefHandle);
	
	// for each mem-ref-expr associated with this memref
	for (; actualMreIterPtr->isValid(); (*actualMreIterPtr)++) {
	  
	  OA::OA_ptr<OA::MemRefExpr> actualMre;
	  
	  actualMre = actualMreIterPtr->current();
	  
	  mPairList.push_back(pair<int, OA::OA_ptr<OA::MemRefExpr> >(paramNum, actualMre));
	  
	}

      }

    }

    if (formalIt != typePtrList.end())
      ++formalIt;
    paramNum++;

  }

}
#else
// Create iterator consisting of pairs of procedure formal SymHandles
// and procedure call actual MemRefHandles.  A pair is created
// only for pointer or reference formals.
void SgParamBindPtrAssignIterator::create(OA::ExprHandle call)
{
  SgNode *node = mIR->getNodePtr(call);
  ROSE_ASSERT(node != NULL);
  
  SgFunctionDeclaration *functionDeclaration = NULL;
  SgNode *lhs = NULL;

  bool isDotExp     = false;
  bool isMethodCall = false;

  switch(node->variantT()) {

  case V_SgFunctionCallExp:
    {
      
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      ROSE_ASSERT(functionCallExp != NULL);
      
      // Simply given the call expression we can not resolve the
      // method (since it may be a virtual method) or function
      // (which may be a function pointer).  Though we can not
      // distinguish between virtual methods, we know that each
      // virtual method must have the same number of parameters
      // and each must be a pointer, reference, or object consistently
      // across all of the methods.  Therefore, any valid
      // virtual method will do.  We aren't so lucky with function
      // pointers.  In this case, we can not get a function declaration
      // to determine the type of the parameters.  Conservatively
      // assume they are all pointers/references.
      
      functionDeclaration = 
	getFunctionDeclaration(functionCallExp); 

      SgExpression *function =
	functionCallExp->get_function();
      ROSE_ASSERT(function != NULL);
      
      SgDotExp *dotExp = isSgDotExp(function);
      SgArrowExp *arrowExp = isSgArrowExp(function);

      if ( dotExp || arrowExp ) {
	isMethodCall = true;
      }

#if 0
      if ( dotExp ) {
	  
	lhs = dotExp->get_lhs_operand();
	ROSE_ASSERT(lhs != NULL);
	  
	SgPointerDerefExp *pointerDerefExp =
	  isSgPointerDerefExp(lhs);
	  
	if ( pointerDerefExp != NULL ) {
	    
	  // This is (*b).foo() == b->foo();
	  lhs = pointerDerefExp->get_operand_i();
	  
	} else {

	  isDotExp = true;

	}

      } else if ( arrowExp ) {

	lhs = arrowExp->get_lhs_operand();
	ROSE_ASSERT(lhs != NULL);
	
	lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);

      }
#endif

      break;
    }
#if 0
  case V_SgNewExp:
    {
      SgNewExp *newExp = isSgNewExp(node);
      ROSE_ASSERT(newExp != NULL);

      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      functionDeclaration = ctorInitializer->get_declaration();

      isMethodCall = true;

#if 0
      lhs = mIR->getNewLhs(newExp);

      if ( lhs != NULL ) {

	lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);

      }
#endif

      break;

    }
#endif
  case V_SgConstructorInitializer:
    {
      SgConstructorInitializer *ctorInitializer =
	isSgConstructorInitializer(node);
      ROSE_ASSERT(ctorInitializer != NULL);

      functionDeclaration = ctorInitializer->get_declaration();

      isMethodCall = true;

#if 0
      lhs = mIR->getNewLhs(newExp);

      if ( lhs != NULL ) {

	lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);

      }
#endif

      break;

    }
  default:
    {
      //      cerr << "Call must be a SgFunctionCallExp or a SgNewExp, got a" << endl;
      cerr << "Call must be a SgFunctionCallExp or a SgConstructorInitializer, got a" << endl;
      cerr << node->sage_class_name() << endl;
      ROSE_ABORT();
      break;
    }
  }

  OA::OA_ptr<OA::IRFormalParamIterator> formalsIter;
  formalsIter = NULL;

  ROSE_ASSERT(functionDeclaration != NULL);

#if 1
  OA::SymHandle procSymHandle = mIR->getProcSymHandle(functionDeclaration);
  formalsIter = mIR->getFormalParamIterator(procSymHandle);
#else 
  SgFunctionDefinition *functionDefinition = NULL;
  
  if ( functionDeclaration != NULL ) {
	
#if 1
    functionDefinition = functionDeclaration->get_definition();
    if ( functionDefinition != NULL ) {
      //    OA::ProcHandle callee = mIR->getNodeNumber(functionDeclaration);
      //    OA::SymHandle procSymHandle = mIR->getProcSymHandle(callee);
      OA::ProcHandle callee = mIR->getProcHandle(functionDefinition);
      OA::SymHandle procSymHandle = mIR->getProcSymHandle(callee);
      formalsIter = mIR->getFormalParamIterator(procSymHandle);
    }
#else
    OA::ProcHandle callee = mIR->getProcHandle(functionDeclaration);
    OA::SymHandle procSymHandle = mIR->getProcSymHandle(callee);
    formalsIter = mIR->getFormalParamIterator(procSymHandle);
#endif
  }
#endif

  OA::OA_ptr<OA::IRCallsiteParamIterator> actualsIter = 
    mIR->getCallsiteParams(call);

#if 0
  // getCallsiteParams already handles folding the object into the
  // param list.  We don't have to do that here.

  // Simultaneously iterate over both formals and actuals.
  // If the formal is a reference parameter (i.e., 
  // a pointer or a reference) store it in the iterator's list.
  int paramNum = 0;
  for ( ; actualsIter->isValid(); (*actualsIter)++, paramNum++ ) { 
    
    bool handleThisExp = false;
    bool treatAsPointerParam = false;

    if ( formalsIter.ptrEqual(0) ) {

      // We don't know any better.  So be conservative ...
      treatAsPointerParam = true;

    } else {

      ROSE_ASSERT(formalsIter->isValid());

      OA::SymHandle formal = formalsIter->current();

      if ( mIR->isRefParam(formal) || mIR->isPointerVar(formal) ) {
	treatAsPointerParam = true;
      }
      
      if ( ( paramNum == 0 ) && ( isMethodCall ) ) {
 	treatAsPointerParam = true;
	handleThisExp = true;
      }

    }

    OA::MemRefHandle actualMemRefHandle = (OA::MemRefHandle)0;

    if ( treatAsPointerParam ) {

      OA::ExprHandle actualExpr = actualsIter->current(); 
      SgNode *actualNode = mIR->getNodePtr(actualExpr);
      ROSE_ASSERT(actualNode != NULL);

      // We fold the object upon which a method is invoked into
      // the argument list (as the first actual) of a method
      // invocation.  If this is the first arg of a method
      // invocation we need special processing for the 'this'
      // expression.  But this is handled in getCallsiteParams

      // Ensure that this expression is actually represented 
      // by a MemRefHandle.  
      actualNode = mIR->lookThroughCastExpAndAssignInitializer(actualNode);
      //	ROSE_ASSERT(mIR->isMemRefNode(actualNode));
      
      if ( mIR->isMemRefNode(actualNode) )
	actualMemRefHandle = mIR->getNodeNumber(actualNode);
	
    }

    if ( actualMemRefHandle == (OA::MemRefHandle)0 ) {
      
#if 0
      // If we didn't assign to actualMemRefHandle, it must
      // be because the SgNode wasn't a MemRefNode.  We
      // should have detected this above from the type signature
      // and skipped this iteration, unless the formals 
      // were not available.
      //	ROSE_ASSERT(functionDefinition == NULL);
      ROSE_ASSERT(formalsIter.ptrEqual(0));
#else
      // This isn't true.  Consider that printf's first formal is
      // a const char *.  We may pass it an actual string, which
      // is not a MemRefNode.
#endif
      
    } else {

      OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
	= mIR->getMemRefExprIterator(actualMemRefHandle);
	
      // for each mem-ref-expr associated with this memref
      for (; actualMreIterPtr->isValid(); (*actualMreIterPtr)++) {
	
	OA::OA_ptr<OA::MemRefExpr> actualMre;
	
	if ( ( handleThisExp == true ) && ( isDotExp == true ) ) {
	  
	  // We are returning a MemRefExpr representing the object b
	  // upon which a method is invoked in the expression b.foo()
	  // (i.e., a dot expression).  This pair is intended to
	  // represent the implicit binding between the 'this' pointer
	  // and this object.  Since 'this' is a pointer, we should
	  // bind it to the address of b.
	  
	  OA::OA_ptr<OA::MemRefExpr> curr = actualMreIterPtr->current();
	  actualMre = curr->clone();
	  actualMre->setAddressTaken(true);
	  
	} else { 
	  
	  actualMre = actualMreIterPtr->current();
	  
	}
	
	
	mPairList.push_back(pair<int, OA::OA_ptr<OA::MemRefExpr> >(paramNum, actualMre));
	
      }
      
    }

    if ( !formalsIter.ptrEqual(0) )
      (*formalsIter)++;
    paramNum++;
    
  }

#else
  int paramNum = 0;

  // Simultaneously iterate over both formals and actuals.
  // If the formal is a reference parameter (i.e., 
  // a pointer or a reference) store it in the iterator's list.
  for ( ; actualsIter->isValid(); (*actualsIter)++ ) { 
    
    bool handleThisExp = false;
    bool treatAsPointerParam = false;

    if ( formalsIter.ptrEqual(0) ) {

      // We don't know any better.  So be conservative ...
      treatAsPointerParam = true;

    } else {

      ROSE_ASSERT(formalsIter->isValid());

      OA::SymHandle formal = formalsIter->current();

      if ( mIR->isRefParam(formal) || mIR->isPointerVar(formal) ) {
	treatAsPointerParam = true;
      }
      
      if ( ( paramNum == 0 ) && ( isMethodCall ) ) {
 	treatAsPointerParam = true;
	handleThisExp = true;
      }

    }

    if ( treatAsPointerParam ) {

      OA::ExprHandle actualExpr = actualsIter->current(); 
      SgNode *actualNode = mIR->getNodePtr(actualExpr);
      ROSE_ASSERT(actualNode != NULL);

      // We fold the object upon which a method is invoked into
      // the argument list (as the first actual) of a method
      // invocation.  If this is the first arg of a method
      // invocation we need special processing for the 'this'
      // expression.

      OA::MemRefHandle actualMemRefHandle = (OA::MemRefHandle)0;

      bool isArrowExp = false;
      if ( handleThisExp == true ) {
	
	if ( lhs != NULL ) {
	  if ( mIR->isMemRefNode(lhs) )
	    actualMemRefHandle = mIR->getNodeNumber(lhs);
	} 

      } else { 

	// Ensure that this expression is actually represented 
	// by a MemRefHandle.  
	actualNode = mIR->lookThroughCastExpAndAssignInitializer(actualNode);
	//	ROSE_ASSERT(mIR->isMemRefNode(actualNode));

	if ( mIR->isMemRefNode(actualNode) )
	  actualMemRefHandle = mIR->getNodeNumber(actualNode);
	
      }

      if ( actualMemRefHandle == (OA::MemRefHandle)0 ) {

#if 0
	// If we didn't assign to actualMemRefHandle, it must
	// be because the SgNode wasn't a MemRefNode.  We
	// should have detected this above from the type signature
	// and skipped this iteration, unless the formals 
	// were not available.
	//	ROSE_ASSERT(functionDefinition == NULL);
	ROSE_ASSERT(formalsIter.ptrEqual(0));
#else
	// This isn't true.  Consider that printf's first formal is
	// a const char *.  We may pass it an actual string, which
	// is not a MemRefNode.
#endif

      } else {

	OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
	  = mIR->getMemRefExprIterator(actualMemRefHandle);
	
	// for each mem-ref-expr associated with this memref
	for (; actualMreIterPtr->isValid(); (*actualMreIterPtr)++) {
	  
	  OA::OA_ptr<OA::MemRefExpr> actualMre;
	  
	  if ( ( handleThisExp == true ) && ( isDotExp == true ) ) {
	    
	    // We are returning a MemRefExpr representing the object b
	    // upon which a method is invoked in the expression b.foo()
	    // (i.e., a dot expression).  This pair is intended to
	    // represent the implicit binding between the 'this' pointer
	    // and this object.  Since 'this' is a pointer, we should
	    // bind it to the address of b.
	    
	    OA::OA_ptr<OA::MemRefExpr> curr = actualMreIterPtr->current();
	    actualMre = curr->clone();
	    actualMre->setAddressTaken(true);
	    
	  } else { 
	    
	    actualMre = actualMreIterPtr->current();
	    
	  }
	  
	  
	  mPairList.push_back(pair<int, OA::OA_ptr<OA::MemRefExpr> >(paramNum, actualMre));
	  
	}

      }

    }

    if ( !formalsIter.ptrEqual(0) )
      (*formalsIter)++;
    paramNum++;

  }

#endif


#if 0
  OA::SymHandle procSymHandle = mIR->getProcSymHandle(callee);

  OA::OA_ptr<OA::IRFormalParamIterator> formalsIter =
    mIR->getFormalParamIterator(procSymHandle);

  OA::OA_ptr<OA::IRCallsiteParamIterator> actualsIter = 
    mIR->getCallsiteParams(call);

  int paramNum = 0;
  bool handleThisExp = false;

  // Simultaneously iterate over both formals and actuals.
  // If the formal is a reference parameter (i.e., 
  // a pointer or a reference) store it in the iterator's list.
  for ( ; formalsIter->isValid(); (*formalsIter)++ ) { 
    
    handleThisExp = false;

    ROSE_ASSERT(actualsIter->isValid());

    OA::SymHandle formal = formalsIter->current();

    if ( mIR->isRefParam(formal) || mIR->isPointerVar(formal) ) {

      OA::ExprHandle actualExpr = actualsIter->current(); 
      SgNode *actualNode = mIR->getNodePtr(actualExpr);
      ROSE_ASSERT(actualNode != NULL);

      SgExpression *function = NULL;

      // We fold the object upon which a method is invoked into
      // the argument list (as the first actual) of a method
      // invocation.  If this is the first arg of a method
      // invocation we need special processing for the 'this'
      // expression.
      if ( paramNum == 0 ) {

	SgNode *node = mIR->getNodePtr(call);
	ROSE_ASSERT(node != NULL);

	SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
	ROSE_ASSERT(functionCallExp != NULL);

	function = functionCallExp->get_function();
	ROSE_ASSERT(function != NULL);
	
	if ( isSgDotExp(function) || isSgArrowExp(function) ) 
	  handleThisExp = true;

      }

      OA::MemRefHandle actualMemRefHandle;

      bool isArrowExp = false;
      if ( handleThisExp == true ) {

	SgNode *lhs = NULL;

	SgArrowExp *arrowExp = isSgArrowExp(function);

	if ( arrowExp != NULL ) {

	  lhs = arrowExp->get_lhs_operand();
	  ROSE_ASSERT(lhs != NULL);

	  lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);
	  ROSE_ASSERT(mIR->isMemRefNode(lhs));

	  isArrowExp = true;
	  
	} else {
	    
	  SgDotExp *dotExp = isSgDotExp(function);
	  ROSE_ASSERT(dotExp != NULL);
	  
	  lhs = dotExp->get_lhs_operand();
	  ROSE_ASSERT(lhs != NULL);
	  
	  SgPointerDerefExp *pointerDerefExp =
	    isSgPointerDerefExp(lhs);
	  
	  if ( pointerDerefExp != NULL ) {
	    
	    // This is (*b).foo() == b->foo();
	    isArrowExp = true;
	    lhs = pointerDerefExp->get_operand_i();
	    
	  }
	  
	  lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);
	  ROSE_ASSERT(mIR->isMemRefNode(lhs));

	}

	actualMemRefHandle = mIR->getNodeNumber(lhs);

      } else { 

	// Ensure that this expression is actually represented 
	// by a MemRefHandle.  
	actualNode = mIR->lookThroughCastExpAndAssignInitializer(actualNode);
	ROSE_ASSERT(mIR->isMemRefNode(actualNode));

	actualMemRefHandle = mIR->getNodeNumber(actualNode);
	
      }

      //      mPairList.push_back(pair<OA::SymHandle, OA::MemRefHandle>(formal, actual));
      OA::OA_ptr<OA::MemRefExprIterator> actualMreIterPtr 
	= mIR->getMemRefExprIterator(actualMemRefHandle);
      // for each mem-ref-expr associated with this memref
      for (; actualMreIterPtr->isValid(); (*actualMreIterPtr)++) {
	
	OA::OA_ptr<OA::MemRefExpr> actualMre;

	if ( ( handleThisExp == true ) && ( isArrowExp == false ) ) {

	  // We are returning a MemRefExpr representing the object b
	  // upon which a method is invoked in the expression b.foo()
	  // (i.e., a dot expression).  This pair is intended to
	  // represent the implicit binding between the 'this' pointer
	  // and this object.  Since 'this' is a pointer, we should
	  // bind it to the address of b.

	  OA::OA_ptr<OA::MemRefExpr> curr = actualMreIterPtr->current();
	  actualMre = curr->clone();
	  actualMre->setAddressTaken(true);

	} else { 

	  actualMre = actualMreIterPtr->current();

	}
	

	mPairList.push_back(pair<OA::SymHandle, OA::OA_ptr<OA::MemRefExpr> >(formal, actualMre));
	
      }

    }

    (*actualsIter)++;
    paramNum++;

  }
#endif
}
#endif


// initializerHasPtrAssign returns true if the SgInitializedName
// is a pointer or reference and it has an initializer.
bool 
SageIRInterface::initializerHasPtrAssign(SgInitializedName *initName,
				     bool collectPtrAssigns,
				     std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{
  ROSE_ASSERT(initName != NULL);

  SgType *lhsType = initName->get_type();
  ROSE_ASSERT(lhsType != NULL);
  
  // If the LHS is not a pointer or reference type, skip it.
  SgType *baseType = getBaseType(lhsType);
  ROSE_ASSERT(baseType != NULL);
  if ( !isSgPointerType(baseType) && !isSgReferenceType(baseType) ) 
    return false;
    
  SgNode *lhs = initName;
  SgNode *rhs = initName->get_initializer();
  
  // There is no rhs.  Skip it.
  if ( rhs == NULL ) 
    return false;
    
  // We have a lhs/rhs pair involved in a pointer assigment.
  // Ensure that these are the type of nodes to which we
  // normally assign MemRefHandles.
  
  if ( !collectPtrAssigns )
    return true;
  
  lhs = lookThroughCastExpAndAssignInitializer(lhs);
  ROSE_ASSERT(isMemRefNode(lhs));
  
  rhs = lookThroughCastExpAndAssignInitializer(rhs);
  ROSE_ASSERT(isMemRefNode(rhs));
  
  // Create the lhs/rhs MemRefHandles.
  OA::MemRefHandle lhsHandle = getNodeNumber(lhs);
  OA::MemRefHandle rhsHandle = getNodeNumber(rhs);
  
  // Save this lhs/rhs pair.
  
  OA::OA_ptr<OA::MemRefExprIterator> lhsMreIterPtr 
    = getMemRefExprIterator(lhsHandle);
  // for each mem-ref-expr associated with this memref
  for (; lhsMreIterPtr->isValid(); (*lhsMreIterPtr)++) {
    
    OA::OA_ptr<OA::MemRefExpr> lhsMre = lhsMreIterPtr->current();
    
    OA::OA_ptr<OA::MemRefExprIterator> rhsMreIterPtr 
      = getMemRefExprIterator(rhsHandle);
    
    for (; rhsMreIterPtr->isValid(); (*rhsMreIterPtr)++) {
      
      OA::OA_ptr<OA::MemRefExpr> rhsMre = rhsMreIterPtr->current();
      
      memRefList->push_back(pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> >(lhsMre, rhsMre));
      
      createImplicitPtrAssigns(lhsMre, rhsMre, rhs, memRefList); 

    }
    
  }
  
  return true;
}

/** \brief Return the declaration of the class with a given type.
 *  \param type  a SgNode representing a type.
 *  \return a SgClassDeclaration that is the class declaration
 *          for the given type, or NULL if type does not
 *          correspond to a class.
 */
SgClassDeclaration *
SageIRInterface::getClassDeclaration(SgType *type)
{
  SgClassDeclaration *classDeclaration = NULL;

  if ( type == NULL ) 
    return NULL;
  
  switch(type->variantT()) {
    
  case V_SgTypedefType:
    {
      SgTypedefType *typedefType = isSgTypedefType(type);
      ROSE_ASSERT(typedefType != NULL);
      
      SgDeclarationStatement *declStmt = typedefType->get_declaration();
      ROSE_ASSERT(declStmt != NULL);
      
      SgTypedefDeclaration *typedefDeclaration = isSgTypedefDeclaration(declStmt);
      ROSE_ASSERT(typedefDeclaration != NULL);
      
      SgDeclarationStatement *innerDecl = 
	typedefDeclaration->get_declaration();
      ROSE_ASSERT(innerDecl != NULL);
      
      classDeclaration = isSgClassDeclaration(innerDecl);
      break;
      
    }
    
  case V_SgClassType:
    {
      SgClassType *classType = isSgClassType(type);
      ROSE_ASSERT(classType != NULL);
      
      SgDeclarationStatement *declStmt = classType->get_declaration();
      ROSE_ASSERT(declStmt != NULL);
      
      classDeclaration = isSgClassDeclaration(declStmt);
      ROSE_ASSERT(classDeclaration != NULL);
      
      break;
    }
    
  default:
    {
      break;
    }
  }

  return classDeclaration;
}

bool 
SageIRInterface::isVirtual(SgFunctionDeclaration *functionDeclaration)
{
  if ( functionDeclaration == NULL ) return false;

  if ( functionDeclaration->get_functionModifier().isVirtual() ) 
    return true;

  SgDeclarationStatement *firstNondefiningDeclaration =
    functionDeclaration->get_firstNondefiningDeclaration();

  if ( firstNondefiningDeclaration == NULL )
    return false;

  SgFunctionDeclaration *firstNondefiningFuncDeclaration =
    isSgFunctionDeclaration(firstNondefiningDeclaration);
  ROSE_ASSERT(firstNondefiningFuncDeclaration != NULL);

  return firstNondefiningFuncDeclaration->get_functionModifier().isVirtual();
}

// Returns true if the function declaration is declared virtual in
// some parent class, regardless of whether it is declared virtual
// in its own class.
bool 
SageIRInterface::isDeclaredVirtualWithinAncestor(SgFunctionDeclaration *functionDeclaration)
{
  SgMemberFunctionDeclaration *memberFunctionDeclaration =
    isSgMemberFunctionDeclaration(functionDeclaration);
  if ( memberFunctionDeclaration == NULL )
    return false;

  SgClassDefinition *classDefinition = 
    isSgClassDefinition(memberFunctionDeclaration->get_scope());
  ROSE_ASSERT(classDefinition != NULL);

  return isDeclaredVirtualWithinClassAncestry(functionDeclaration,
					      classDefinition);
}

bool 
SageIRInterface::isDeclaredVirtualWithinClassAncestry(SgFunctionDeclaration *functionDeclaration, SgClassDefinition *classDefinition)
{
  SgType *functionType =
    functionDeclaration->get_type();
  ROSE_ASSERT(functionType != NULL);

  // Look in each of the class' parent classes.
  SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances(); 
  for (SgBaseClassPtrList::iterator i = baseClassList.begin(); 
       i != baseClassList.end(); ++i) {
 
    SgBaseClass *baseClass = *i;
    ROSE_ASSERT(baseClass != NULL);

    SgClassDeclaration *classDeclaration = baseClass->get_base_class(); 
    ROSE_ASSERT(classDeclaration != NULL);

    SgClassDefinition  *parentClassDefinition  = 
      classDeclaration->get_definition(); 

    // Visit all methods in the parent class.
    SgDeclarationStatementPtrList &members = 
      parentClassDefinition->get_members(); 

    bool isDeclaredVirtual = false;

    for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
	 it != members.end(); ++it) { 
    
      SgDeclarationStatement *declarationStatement = *it; 
      ROSE_ASSERT(declarationStatement != NULL);
      
      switch(declarationStatement->variantT()) {
      
      case V_SgMemberFunctionDeclaration:
	{
	  SgMemberFunctionDeclaration *memberFunctionDeclaration =  
	    isSgMemberFunctionDeclaration(declarationStatement); 

	  if ( isVirtual(memberFunctionDeclaration) ) {

	    SgType *parentMemberFunctionType =
	      memberFunctionDeclaration->get_type();
	    ROSE_ASSERT(parentMemberFunctionType != NULL);

	    if ( parentMemberFunctionType == functionType ) {
	      return true;
	    }

	  }
	  break;

	}
      default:
	{
	  break;
	}

      }

    }

    if ( isDeclaredVirtualWithinClassAncestry(functionDeclaration, 
					      parentClassDefinition) ) {
      return true;
    }

  }

  return false;
}

// If astNode is an allocation site (new or malloc) of
// a class instance, return the declaration of that class.
SgClassDeclaration *
SageIRInterface::findAllocatedClass(SgNode *astNode)
{
  if ( astNode == NULL ) return NULL;

  SgClassDeclaration *classDeclaration = NULL;
  SgType *type = NULL;

  if ( isAllocation(astNode, type) ) {

    ROSE_ASSERT(type != NULL);
    classDeclaration = getClassDeclaration(type);

  }

  return classDeclaration;

}

// Use the vtable optimization.  Rather than
// create implicit ptr assigns for each method of an object a 
// of class A, we create a single implicit assignment:
// < (*a).FieldHandle(OA_VTABLE_STR), &A >
// This is effectively a pointer to a (virtual) table,
// though we create it whenever there are methods, virtual
// or otherwise.
// At each declaration of class A we create implicit pairs
// for each method of the form:
// < A.method, &A::method >
// where the first MRE is a FieldAccess and the second is a NamedRef.
// Thus because of the implicit pair 
// ((*a).FieldHandle(OA_VTABLE_STR), &A) FIAlias unions
// (*(*a).FieldHandle(OA_VTABLE_STR) and A.  Because of the
// pair (A.method, &A::method) returned here, FIAlias unions
// *A.method and A::method.  Finally, when getCallMemRefExpr
// returns *(*(*a).FieldHandle(OA_VTABLE_STR).method), FIAlias
// sees that *(*(*a).FieldHandle(OA_VTABLE_STR).method) is aliased
// to *A.method is aliased to A::method.
bool 
SageIRInterface::
createImplicitVTablePtrAssignFromDefinition(SgClassDefinition *classDefinition,
					    bool collectPtrAssigns,
					    std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{

  bool hasImplicitPtrAssign = false;

#ifndef VTABLE_OPT
  return hasImplicitPtrAssign;
#endif

  ROSE_ASSERT(classDefinition != NULL);

  // Create an implicit ptr assignment pair for all methods in this
  // class definition and for all methods in all superclasses.

  // First, visit all methods in this class.
  SgDeclarationStatementPtrList &members = classDefinition->get_members(); 
  for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
       it != members.end(); ++it) { 
    
    SgDeclarationStatement *declarationStatement = *it; 
    ROSE_ASSERT(declarationStatement != NULL);
    
    switch(declarationStatement->variantT()) {
      
    case V_SgMemberFunctionDeclaration:
      {
	SgMemberFunctionDeclaration *functionDeclaration =  
	  isSgMemberFunctionDeclaration(declarationStatement); 
	
	// Base case: 
	// Create implicit pointer assignment pairs for any methods
	// declared in this class.
	if ( ( functionDeclaration != NULL ) && 
	     ( ( isVirtual(functionDeclaration) ) ||
	       ( isDeclaredVirtualWithinAncestor(functionDeclaration) ) ) ) {


	  hasImplicitPtrAssign = true;

	  if ( !collectPtrAssigns ) break;

	  bool addressTaken = false;
	  bool fullAccuracy = true;
	  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
	  OA::SymHandle symHandle;
	  symHandle = getNodeNumber(classDefinition);

	  OA::OA_ptr<OA::NamedRef> classMRE;
	  classMRE = new OA::NamedRef(addressTaken, 
				      fullAccuracy,
				      memRefType,
				      symHandle);

	  string mangledMethodName = 
	    functionDeclaration->get_mangled_name().str();
	  memRefType = OA::MemRefExpr::DEF;

	  addressTaken = false;
	  OA::OA_ptr<OA::FieldAccess> fieldAccess;
#if 0
	  ROSE_ASSERT(!classMRE->isDef());
	  ROSE_ASSERT(!classMRE->isDefUse());
	  ROSE_ASSERT(!classMRE->isUseDef());
#endif
	  fieldAccess = new OA::FieldAccess(addressTaken, 
					    fullAccuracy,
					    memRefType,
					    classMRE,
					    mangledMethodName);

	  // Create the rhs to represent the method declaration.
	  memRefType = OA::MemRefExpr::USE;

	  symHandle = getProcSymHandle(functionDeclaration);

	  addressTaken = true;
	  OA::OA_ptr<OA::NamedRef> method;
	  method = new OA::NamedRef(addressTaken, 
				    fullAccuracy,
				    memRefType,
				    symHandle);

	  // Create the pair.
	  memRefList->push_back(pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> >(fieldAccess, method));
	
	}
	break;
      }
    default:
      {
	break;
      }
    }
    
    if ( hasImplicitPtrAssign && !collectPtrAssigns ) break;
  }

  return hasImplicitPtrAssign;
}

/** \brief Return true if a class or any of its base classes 
 *         has a virtual method.
 *  \param classDefinition  a SgNode representing a class definition.
 *  \return boolean indicating whether the class represented by
 *                  classDefinition, or any of its base classes,
 *                  define a virtual method.
 */
bool
SageIRInterface::
classHasVirtualMethods(SgClassDefinition *classDefinition)
{
  bool hasMethods = false;

  if ( classDefinition == NULL ) return hasMethods;

  SgDeclarationStatementPtrList &members = classDefinition->get_members(); 
  for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
       it != members.end(); ++it) { 
    
    SgDeclarationStatement *declarationStatement = *it; 
    ROSE_ASSERT(declarationStatement != NULL);
    
    switch(declarationStatement->variantT()) {
      
    case V_SgMemberFunctionDeclaration:
      {
	SgMemberFunctionDeclaration *functionDeclaration =  
	  isSgMemberFunctionDeclaration(declarationStatement); 
	
	if ( ( functionDeclaration != NULL ) &&
	     ( ( isVirtual(functionDeclaration) ) ||
	       ( isDeclaredVirtualWithinAncestor(functionDeclaration) ) ) ) {
	  return true;
	}

	break;

      }
    default:
      {
	break;
      }
    }
  }

  if ( hasMethods ) 
    return true;

  // The class did not directly define any virtual methods, look in
  // its base classes.
  SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances(); 
  for (SgBaseClassPtrList::iterator i = baseClassList.begin(); 
       i != baseClassList.end(); ++i) {
    
    SgBaseClass *baseClass = *i;
    ROSE_ASSERT(baseClass != NULL);
    
    SgClassDeclaration *classDeclaration = baseClass->get_base_class(); 
    ROSE_ASSERT(classDeclaration != NULL);
    
    SgClassDefinition  *parentClassDefinition  = 
      classDeclaration->get_definition(); 
    
    if ( parentClassDefinition != NULL ) {
      if ( classHasVirtualMethods(parentClassDefinition) ) {
	return true;
      }
    }
  }

  return hasMethods;
}

/**
 * \brief Return true if methodDecl overrides virtualMethodDecl.
 * \param methodDecl  a method declaration.
 * \param virtualMethodDecl a method declaration.
 * \return Returns true if virtualMethodDecl is declared as a virtual
 *         method and methodDecl has the same type signature and name
 *         as virtualMethodDecl.  
 * 
 * NB:  It is assumed that the class defining virtualMethodDecl is a base
 *      class of the class defining methodDecl.
 */
bool
SageIRInterface::methodOverridesVirtualMethod(SgMemberFunctionDeclaration *methodDecl, 
					      SgMemberFunctionDeclaration *virtualMethodDecl)
{
  if ( !isVirtual(virtualMethodDecl) )
    return false;

#if 1
  // Hmmm ... couldn't we just compare mangled names?
  return ( methodDecl->get_mangled_name() == virtualMethodDecl->get_mangled_name() );

#else
  if ( methodDecl->get_name() != virtualMethodDecl->get_name() )
    return false;
  
  SgType *methodReturnType = methodDecl->get_orig_return_type();
  SgType *virtualMethodReturnType = virtualMethodDecl->get_orig_return_type();

  if ( methodReturnType != virtualMethodReturnType )
    return false;

  int numMethodParams = 0;
  int numVirtualMethodParams = 0;

  SgFunctionParameterList *methodParameterList = 
    methodDecl->get_parameterList(); 

  if (methodParameterList != NULL) {
    numMethodParams = methodParameterList->get_args().size();
  }

  SgFunctionParameterList *virtualMethodParameterList = 
    virtualMethodDecl->get_parameterList(); 

  if (virtualMethodParameterList != NULL) {
    numVirtualMethodParams = virtualMethodParameterList->get_args().size();
  }

  if ( numMethodParams != numVirtualMethodParams )
    return false;

  if ( numMethodParams == 0 )
    return true;

  const SgInitializedNamePtrList &methodFormalParams = 
    methodParameterList->get_args(); 
  const SgInitializedNamePtrList &virtualMethodFormalParams = 
    virtualMethodParameterList->get_args(); 
  SgInitializedNamePtrList::const_iterator methodIt;
  SgInitializedNamePtrList::const_iterator virtualMethodIt;
  for(methodIt = methodFormalParams.begin(), 
	virtualMethodIt = virtualMethodFormalParams.begin();
      methodIt != methodFormalParams.end(); ++methodIt, ++virtualMethodIt) { 
      
      SgInitializedName* methodFormalParam = *methodIt;  
      ROSE_ASSERT(methodFormalParam != NULL); 

      SgInitializedName* virtualMethodFormalParam = *virtualMethodIt;  
      ROSE_ASSERT(virtualMethodFormalParam != NULL); 
      
      if ( methodFormalParam->get_type() != 
	   virtualMethodFormalParam->get_type() )
	return false;

  }

  return true;
#endif
}

// Create an implicit ptr assign pair for each method declared in 
// classDefinition.  These pairs are of the form:
// < (*lhsMre).m, &C::m >
// where method m is defined in class C and where the first
// element of the pair is a FieldAccess, while the second is a NamedRef.
// Returns true if any pairs are/would be created.  Only creates
// pairs if collectPtrAssigns is true, in which case they are 
// stored in memRefList.
// When getCallMemRefExpr() returns *((*lhsMre).m), FIAlias uses
// this implicit pair to bind it to C::m (after dereferencing)
// to perform virtual method resolution.
bool 
SageIRInterface::
createImplicitPtrAssignForMethods(OA::OA_ptr<OA::MemRefExpr> lhsMRE,
				  OA::OA_ptr<OA::MemRefExpr> rhsMRE,
				  SgClassDefinition *classDefinition,
				  bool collectPtrAssigns,
				  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList,
				  std::list<SgMemberFunctionDeclaration *> &visitedVirtualMethods)
{

  bool hasImplicitPtrAssign = false;
  
#if 1
  // A lhs need not be a pointer.  We now create implicit ptr assignments
  // for object allocations, e.g., A a;.  While it is true that 
  // we can statically determine virtual methods invocations for 
  // objects (as opposed to pointers), we make take the address of
  // the object and assign it to a pointer.  A *b = &a.  
  // We need b to have access to these implicit ptr assignments.
#else

  // We expect that the lhs has already been passed in 
  // as a Deref.
  ROSE_ASSERT(lhsMRE->isaRefOp());
  
  OA::OA_ptr<OA::RefOp> refOp = lhsMRE.convert<OA::RefOp>();
  ROSE_ASSERT(!refOp.ptrEqual(0));

  // This is always a Deref (i.e., a pointer).  Yes, virtual methods,
  // etc. are an issue for references as well.  However, we only
  // create implicit ptr assignments for allocations, not
  // for assignments.  The former only apply to pointers.
  
  ROSE_ASSERT(refOp->isaDeref());
#endif  

  ROSE_ASSERT(classDefinition != NULL);


  // Create an implicit ptr assignment pair for all methods in this
  // class definition and for all methods in all superclasses.

#ifndef VTABLE_OPT
  // First, visit all methods in this class.
  SgDeclarationStatementPtrList &members = classDefinition->get_members(); 
  for (SgDeclarationStatementPtrList::iterator it = members.begin(); 
       it != members.end(); ++it) { 
    
    SgDeclarationStatement *declarationStatement = *it; 
    ROSE_ASSERT(declarationStatement != NULL);
    
    switch(declarationStatement->variantT()) {
      
    case V_SgMemberFunctionDeclaration:
      {
	SgMemberFunctionDeclaration *functionDeclaration =  
	  isSgMemberFunctionDeclaration(declarationStatement); 
	
	// Base case: 
	// Create implicit pointer assignment pairs for any methods
	// declared in this class.
	if ( ( functionDeclaration != NULL ) &&
	     ( ( isVirtual(functionDeclaration) ) ||
	       ( isDeclaredVirtualWithinAncestor(functionDeclaration) ) ) ) {

	  // Don't visit a virtual method if we have already
	  // visited a virtual method which overrides it.
	  bool visitedAnOverridingMethod = false;
	  std::list<SgMemberFunctionDeclaration *>::iterator it = visitedVirtualMethods.begin();
	  for(; it != visitedVirtualMethods.end(); ++it) {
	    SgMemberFunctionDeclaration *memberFunctionDecl = *it;
	    ROSE_ASSERT(memberFunctionDecl != NULL);
	    if ( methodOverridesVirtualMethod(memberFunctionDecl, functionDeclaration) ) {
	      visitedAnOverridingMethod = true;
	      break;
	    }
	  }
	  

	  if ( !visitedAnOverridingMethod ) {
	    visitedVirtualMethods.push_back(functionDeclaration);

	    hasImplicitPtrAssign = true;
	    
	    if ( !collectPtrAssigns ) break;
	    
	    // Create an implicit pointer assignment pair for this
	    // method m of class C of the form:
	    // < (*lhs).m, &C::m >
	    // Since we do not accurately model structures, we
	    // collapse the lhs 'a.b.c' to 'a' and flag it is an
	    // inaccurate.
	    // 1/15/06:  If lhsMRE is inaccurate, it will already
	    // have been marked as such.
	    
	    // Create the lhs of the pair, ensuring that it is marked
	    // inaccurate.
	    OA::OA_ptr<OA::MemRefExpr> baseLHS;
#define USELHS
#ifdef USELHS
	    baseLHS = lhsMRE->clone();
#else
	    baseLHS = rhsMRE->clone();
#endif
	    
#if 0
	    if ( baseLHS->hasFullAccuracy() ) {
	      baseLHS->setAccuracy(false);
	    }
#endif
	    
	    bool addressTaken = false;
	    bool fullAccuracy = true;
	    
#ifdef USELHS
	    if ( !baseLHS->hasFullAccuracy() ) {
	      fullAccuracy = false;
	    }
#else
	    if ( baseLHS->isaUnnamed() ) {
	      ROSE_ASSERT(baseLHS->hasAddressTaken());
	      baseLHS->setAddressTaken(false);
	    } 
	    if ( !baseLHS->hasFullAccuracy() ) {
	      fullAccuracy = false;
	    }
#endif
	    
	    OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::DEF;
	    string mangledMethodName = functionDeclaration->get_mangled_name().str();
	    
	    OA::OA_ptr<OA::FieldAccess> fieldAccess;
#if 0
	    ROSE_ASSERT(!baseLHS->isDef());
	    ROSE_ASSERT(!baseLHS->isDefUse());
	    ROSE_ASSERT(!baseLHS->isUseDef());
#endif
	    fieldAccess = new OA::FieldAccess(addressTaken, 
					      fullAccuracy,
					      memRefType,
					      baseLHS,
					      mangledMethodName);
	    
	    // Create the rhs to represent the method declaration.
	    memRefType = OA::MemRefExpr::USE;
	    OA::SymHandle symHandle;
	    symHandle = getProcSymHandle(functionDeclaration);
	    
	    addressTaken = true;
	    OA::OA_ptr<OA::NamedRef> method;
	    method = new OA::NamedRef(addressTaken, 
				      fullAccuracy,
				      memRefType,
				      symHandle);
	    
	    // Create the pair.
	    memRefList->push_back(pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> >(fieldAccess, method));
	  }
	}
	break;
      }
    default:
      {
	break;
      }
    }
    
    if ( hasImplicitPtrAssign && !collectPtrAssigns ) break;
  }

#endif /* VTABLE_OPT */

#ifdef VTABLE_OPT
  bool hasMethods = classHasVirtualMethods(classDefinition);

  if ( hasMethods == true ) {
    
    // Use the vtable optimization.  In this case, we have not
    // created any implicit ptr assigns above.  Rather than
    // create implicit ptr assigns for each method of an object a 
    // of class A, we create a single implicit assignment:
    // < (*a).FieldHandle(OA_VTABLE_STR), &A >
    // This is effectively a pointer to a (virtual) table,
    // though we create it whenever there are methods, virtual
    // or otherwise.
    // 1/15/06:  No longer true, now we just create it if there
    // are virtual methods.  Why were we every creating for
    // non-virtual methods.

    // At each declaration of class A we create implicit pairs
    // for each method of the form:
    // < A.method, &A::method >
    // where the first MRE is a FieldAccess and the second is a NamedRef.
    // Thus *((*(*a).FieldHandle(OA_VTABLE_STR)).method), as
    // returned by getCallMemRefExpr(), is aliased to *A.method 
    // is aliased to A::method
    // via FIAlias, which resolves the string-based FieldAccess to
    // the symbol-based NamedRef which unambiguously specifies the method.

    // Create the implicit ptr assignment.
    bool addressTaken = false;
    bool fullAccuracy = true;
    OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::DEF;
    string mangledMethodName = OA_VTABLE_STR;
    
    // Notice that lhsMRE is a deref.  We asserted that above.
    OA::OA_ptr<OA::FieldAccess> fieldAccess;
#if 0
    ROSE_ASSERT(!lhsMRE->isDef());
    ROSE_ASSERT(!lhsMRE->isDefUse());
    ROSE_ASSERT(!lhsMRE->isUseDef());
#endif
    fieldAccess = new OA::FieldAccess(addressTaken, 
				      fullAccuracy,
				      memRefType,
				      lhsMRE,
				      mangledMethodName);

    // Create the rhs to represent the class of the lhs.
    memRefType = OA::MemRefExpr::USE;
    OA::SymHandle symHandle;
    symHandle = getNodeNumber(classDefinition);

    OA::OA_ptr<OA::NamedRef> classMRE;
    classMRE = new OA::NamedRef(addressTaken, 
				fullAccuracy,
				memRefType,
				symHandle);

    // Create the pair.
    memRefList->push_back(pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> >(fieldAccess, classMRE));

  }
#endif

  // Recursively call this method on each base class.
  if ( !hasImplicitPtrAssign || collectPtrAssigns ) {

    SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances(); 
    for (SgBaseClassPtrList::iterator i = baseClassList.begin(); 
	 i != baseClassList.end(); ++i) {
 
        SgBaseClass *baseClass = *i;
	ROSE_ASSERT(baseClass != NULL);

	SgClassDeclaration *classDeclaration = baseClass->get_base_class(); 
	ROSE_ASSERT(classDeclaration != NULL);

	SgClassDefinition  *parentClassDefinition  = 
	  classDeclaration->get_definition(); 

	if ( parentClassDefinition != NULL ) {

	  bool hasPtrAssign;
	  hasPtrAssign = createImplicitPtrAssignForMethods(lhsMRE,
							   rhsMRE,
							   parentClassDefinition,
							   collectPtrAssigns,
							   memRefList,
							   visitedVirtualMethods);
	  if ( hasPtrAssign )
	    hasImplicitPtrAssign = true;
	  
	}
	    
	if ( hasImplicitPtrAssign && !collectPtrAssigns ) break;

    }

  }

  return hasImplicitPtrAssign;
}

// Creates implicit ptr assign pairs given a MemRefExpr representing the
// lhs and a SgNode rhs.  Any such pairs are returned in memRefList.
void
SageIRInterface::
createImplicitPtrAssigns(OA::OA_ptr<OA::MemRefExpr> lhs,
			 OA::OA_ptr<OA::MemRefExpr> rhsMRE,
			   SgNode *rhs,
			   std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{
  ROSE_ASSERT(rhs != NULL);

  SgNode *castLessRhs = lookThroughCastExpAndAssignInitializer(rhs);
  ROSE_ASSERT(castLessRhs != NULL);

  SgNewExp *newExp = isSgNewExp(castLessRhs);
  // If the Rhs allocates an object, we need to create 
  // implicit assignments for the methods.
  if ( newExp ) {
    
    // NB:  this only makes sense for a _named_ type, not a basic type.
    SgType *type = newExp->get_type();
    ROSE_ASSERT(type != NULL);

    while ( isSgPointerType(type) ) {
      type = isSgPointerType(type)->get_base_type();
      ROSE_ASSERT(type != NULL);
    }

    if ( isSgNamedType(type) ) {

      // createImplicitPtrAssignFromObjectAllocation creates
      // implicit assignments of the form
      // < FieldAccess(.. "mangledMethodName" ..), NamedRef(method) >
      // Because the FieldAccess describes access to an object,
      // not a pointer, we should defer the MRE that we pass to
      // this method:
      
      OA::OA_ptr<OA::MemRefExpr> derefedLhs;
      derefedLhs = dereferenceMre(lhs);
      ROSE_ASSERT(!derefedLhs.ptrEqual(0));
      
      // Extract the class being allocated.
      SgClassDeclaration *classDeclaration =
	findAllocatedClass(castLessRhs);
      ROSE_ASSERT(classDeclaration != NULL);
      
      SgClassDefinition *classDefinition = 
	classDeclaration->get_definition();
      
      if ( classDefinition != NULL ) {
	
	SgConstructorInitializer *ctorInitializer =
	  newExp->get_constructor_args();
	ROSE_ASSERT(ctorInitializer != NULL);
	
	std::set<SgClassDefinition *> examinedClasses;
	std::set<SgConstructorInitializer *> examinedCtors;
	
	bool collectPtrAssigns = true;
	
	createImplicitPtrAssignFromObjectAllocation(derefedLhs,
						    rhsMRE,
						    classDefinition,
						    ctorInitializer,
						    collectPtrAssigns,
						    examinedClasses,
						    examinedCtors,
						    memRefList);
      }
    }
  }
}

// Create any implicit ptr assign pairs resulting from an object allocation
// of class classDefinition, creating using the constructor represented
// by ctorInitializer.  Creation only occurs if collectPtrAssigns is true,
// in which case the results are returned in memRefList.  If such
// pairs would be created, the method returns true.  lhsMRE represents
// the lhs of the allocation:  lhs = new ... This method expects lhs
// to be a Deref.  examindedClasses and examinedCtors aggregate classes
// constructors upon which this method has already been invoked.
bool 
SageIRInterface::
createImplicitPtrAssignFromObjectAllocation(OA::OA_ptr<OA::MemRefExpr> lhsMRE,
					    OA::OA_ptr<OA::MemRefExpr> rhsMRE,
					    SgClassDefinition *classDefinition,
					    SgConstructorInitializer *ctorInitializer,
					    bool collectPtrAssigns,
					    std::set<SgClassDefinition *> examinedClasses,
					    std::set<SgConstructorInitializer *> examinedCtors,
					    std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{
  bool hasImplicitPtrAssign = false;
  
  ROSE_ASSERT(classDefinition != NULL);
  ROSE_ASSERT(ctorInitializer != NULL);

  // We do not accurately model field accesses to structs/classes,
  // but instead collapse the lhs of a pair to an inaccurate reference
  // to the top mem reference (i.e., a.b.c -> a, with inaccurate flag set).
  // Therefore, there is no need to distinguish between a.b.c and
  // a.b.d if c and d have the same type.  i.e., if we have seen this
  // class, don't do anything.  Were we to model field accesses with
  // greater accuracy the lhs would distinguish such pairs and we
  // would remove this conditional (and examinedClasses).

  // This isn't quite true since we could might allocate the same class
  // with different constructors and we do something on a per-constructor
  // basis.

  if ( examinedClasses.find(classDefinition) == examinedClasses.end() ) {
    std::list<SgMemberFunctionDeclaration *> visitedVirtualMethods;
    hasImplicitPtrAssign = createImplicitPtrAssignForMethods(lhsMRE,
							     rhsMRE,
							     classDefinition,
							     collectPtrAssigns,
							     memRefList,
							     visitedVirtualMethods);
    examinedClasses.insert(classDefinition);
  }
#if 0
  if ( examinedCtors.find(ctorInitializer) == examinedCtors.end() ) {
    if ( createImplicitPtrAssignForCtor(lhsMRE,
					classDefinition,
					ctorInitializer,
					collectPtrAssigns,
					examinedClasses,
					examinedCtors,
					memRefList) )
      hasImplicitPtrAssign = true;
    examinedCtors.insert(ctorInitializer);
  }
#endif

  return hasImplicitPtrAssign;

}

// cotrInitListHasPtrAssign returns true if the constructor
// initializer list initializes any pointers or references.
// If collectPtrAssigns is true, it returns the corresponding 
// MemRefHandle/MemRefExpr pairs in memRefList.
bool 
SageIRInterface::ctorInitListHasPtrAssign(SgCtorInitializerList *initializerList,
					  bool collectPtrAssigns,
					  std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{
  bool hasPtrAssign = false;

  SgInitializedNamePtrList &variables =
    initializerList->get_ctors();
  SgInitializedNamePtrList::iterator varIter;
  for (varIter = variables.begin(); varIter != variables.end(); ++varIter) {
    SgNode *lhs = *varIter;
    ROSE_ASSERT(lhs != NULL);
    
    SgInitializedName *initName = isSgInitializedName(lhs);
    ROSE_ASSERT(initName != NULL);
    
    if ( initializerHasPtrAssign(initName,
				 collectPtrAssigns,
				 memRefList) )
      hasPtrAssign = true;

    if ( hasPtrAssign && !collectPtrAssigns )
      return true;
        
  }

  return hasPtrAssign;
}

// varDeclHasPtrAssign returns true if the variable declaration
// assigns values to a pointer or reference.  If collectPtrAssigns is true,
// it returns the corresponding MemRefHandle/MemRefExpr pairs
// in memRefList.
bool 
SageIRInterface::varDeclHasPtrAssign(SgVariableDeclaration *varDeclaration,
				     bool collectPtrAssigns,
				     std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{
  bool hasPtrAssign = false;

  SgInitializedNamePtrList &variables =
    varDeclaration->get_variables();
  SgInitializedNamePtrList::iterator varIter;
  for (varIter = variables.begin(); varIter != variables.end(); ++varIter) {
    SgNode *lhs = *varIter;
    ROSE_ASSERT(lhs != NULL);
    
    SgInitializedName *initName = isSgInitializedName(lhs);
    ROSE_ASSERT(initName != NULL);
    
    if ( initializerHasPtrAssign(initName,
				 collectPtrAssigns,
				 memRefList) )
      hasPtrAssign = true;

    if ( hasPtrAssign && !collectPtrAssigns )
      return true;
        
  }

  return hasPtrAssign;
}

/** \brief Returns true and provides implicit pointer assign pairs
 *         if the variable declaration declares (and instantiates) an object.  
 *  \param varDeclaration  a SgNode representing a variable declaration.
 *  \param collectPtrAssigns  boolean indicating whether we should
 *                            collect implicit ptr assign pairs.
 *  \param memRefList  if collectPtrAssigns is true, place any 
 *                     implicit ptr assigns resulting from this
 *                     declaration into the list.
 *  \return boolean indicating whether the object declaration would
 *                  give rise to implicit ptr assign pairs (whether
 *                  or not they were actually stored in memRefList).
 */
bool 
SageIRInterface::isObjectDeclaration(SgVariableDeclaration *varDeclaration,
				     bool collectPtrAssigns,
				     std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{
  bool hasPtrAssign = false;

  SgInitializedNamePtrList &variables =
    varDeclaration->get_variables();
  SgInitializedNamePtrList::iterator varIter;
  for (varIter = variables.begin(); varIter != variables.end(); ++varIter) {
    SgNode *lhs = *varIter;
    ROSE_ASSERT(lhs != NULL);
    
    SgInitializedName *initName = isSgInitializedName(lhs);
    ROSE_ASSERT(initName != NULL);
    
    if ( isObjectDeclaration(initName,
			     collectPtrAssigns,
			     memRefList) )
      hasPtrAssign = true;

    if ( hasPtrAssign && !collectPtrAssigns )
      return true;
        
  }

  return hasPtrAssign;
}

/** \brief Returns true and provides implicit pointer assign pairs
 *         if the initialized name declares (and instantiates) an object.  
 *  \param initName  a SgNode representing an initialized name from
 *                   a declaration.
 *  \param collectPtrAssigns  boolean indicating whether we should
 *                            collect implicit ptr assign pairs.
 *  \param memRefList  if collectPtrAssigns is true, place any 
 *                     implicit ptr assigns resulting from this
 *                     declaration into the list.
 *  \return boolean indicating whether the object declaration would
 *                  give rise to implicit ptr assign pairs (whether
 *                  or not they were actually stored in memRefList).
 */
bool 
SageIRInterface::isObjectDeclaration(SgInitializedName *initName,
				     bool collectPtrAssigns,
				     std::list<std::pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> > > *memRefList)
{
  bool hasPtrAssign = false;

  ROSE_ASSERT(initName != NULL);

  SgType *type = initName->get_type();
  ROSE_ASSERT(type != NULL);
   
  // We are only interested in object instantiations, 
  // not pointer or reference declarations.
  if ( isSgReferenceType(type) || isSgPointerType(type) ) {
    return hasPtrAssign;
  }

  // Could replace the above conditional with this;
  // just being explicit.
  if ( !isSgNamedType(type) ) {
    return hasPtrAssign;
  }

  SgClassDeclaration *classDeclaration =
    getClassDeclaration(type);

  if ( classDeclaration == NULL ) {
    return hasPtrAssign;
  }

  SgClassDefinition *classDefinition = 
    classDeclaration->get_definition();
      
  if ( classDefinition == NULL ) {
    return hasPtrAssign;
  }

  hasPtrAssign = classHasVirtualMethods(classDefinition);

  if ( !hasPtrAssign || !collectPtrAssigns ) {
    return hasPtrAssign;
  }

  OA::OA_ptr<OA::MemRefExpr> rhsMRE;
  OA::OA_ptr<OA::MemRefExpr> lhsMRE;

  bool addressTaken = false;
  bool fullAccuracy = true;
  OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;

  OA::SymHandle symHandle = getVarSymHandle(initName);

  lhsMRE = new OA::NamedRef(addressTaken, 
			    fullAccuracy,
			    memRefType,
			    symHandle);

  std::list<SgMemberFunctionDeclaration *> visitedVirtualMethods;
  createImplicitPtrAssignForMethods(lhsMRE,
				    rhsMRE,
				    classDefinition,
				    collectPtrAssigns,
				    memRefList,
				    visitedVirtualMethods);

  return hasPtrAssign;
}


// Create iterator consisting of lhs/rhs pairs from pointer
// assignments in stmt.
void SgPtrAssignPairStmtIterator::create(OA::StmtHandle stmt)
{
  if ( mIR->getAliasStmtType(stmt) != OA::Alias::PTR_ASSIGN_STMT )
    return;

  SgNode *node = mIR->getNodePtr(stmt);
  ROSE_ASSERT(node != NULL);

  bool collectPtrAssigns = true;

  // We expect this to be either a SgVariableDeclaration or an assign.
  switch(node->variantT()) {

#ifdef VTABLE_OPT
  case V_SgClassDefinition:
    {
      // Create implicit ptr assign pairs based on a class definition,
      // if we are using the virtual table optimization.

      SgClassDefinition *classDefinition =
	isSgClassDefinition(node);

      ROSE_ASSERT(classDefinition != NULL);

      bool collectPtrAssigns = true;

      createImplicitVTablePtrAssignFromDefinition(classDefinition,
						  collectPtrAssigns,
						  &mMemRefList);

      break;
    }
#endif

  case V_SgCtorInitializerList: 
    {
      SgCtorInitializerList *initializerList =
	isSgCtorInitializerList(node);
      ROSE_ASSERT(initializerList != NULL);

      bool collectPtrAssigns = true;

      mIR->ctorInitListHasPtrAssign(initializerList, 
				    collectPtrAssigns, 
				    &mMemRefList);
      
      break;
    }
  case V_SgVariableDeclaration:
    {
      SgVariableDeclaration *varDecl = isSgVariableDeclaration(node);
      ROSE_ASSERT(varDecl != NULL);

      bool collectPtrAssigns = true;

      mIR->varDeclHasPtrAssign(varDecl, collectPtrAssigns, &mMemRefList);

      mIR->isObjectDeclaration(varDecl, collectPtrAssigns, &mMemRefList);

      break;
    }
  case V_SgReturnStmt:
    {
      SgReturnStmt *returnStmt = isSgReturnStmt(node);
      ROSE_ASSERT(returnStmt != NULL);

      createPtrAssignPairsFromReturnStmt(returnStmt);
      
      break;
    }
  case V_SgExprStatement:
    {
      SgExprStatement *exprStatement = isSgExprStatement(node);
      ROSE_ASSERT(exprStatement != NULL);
      
      SgExpression *expression = exprStatement->get_the_expr();

      createPtrAssignPairsFromAssignment(expression);

      break;
    }
  default:
    {
      cerr << "SgPtrAssignPairStmtIterator::create expected stmt" << endl;
      cerr << "to be either a SgVariableDeclaration or an assign." << endl;
      cerr << "Instead got a " << node->sage_class_name() << endl;
      ROSE_ABORT();
      break;
    }

  }
  
}

//! Return the symbol handle for the nth formal parameter to proc 
//! Number starts at 0 and implicit parameters should be given 
//! a number in the order as well.  This number should correspond 
//! to the number provided in getParamBindPtrAssign pairs 
OA::SymHandle SageIRInterface::getFormalSym(OA::ProcHandle procHandle, 
					    int n)
{
  OA::SymHandle formal = 0;

  OA::SymHandle procSymHandle = getProcSymHandle(procHandle);

  OA::OA_ptr<OA::IRFormalParamIterator> formalsIter =
    getFormalParamIterator(procSymHandle);

  int paramNum = 0;
  for ( ; formalsIter->isValid(); (*formalsIter)++ ) { 
    
    if ( paramNum == n )
      formal = formalsIter->current();

    paramNum++;

  }

  return formal;

}

//! Returns true if function is a SgArrowExp or a SgDotExp
//! whose operand is a SgPointerDerefExp or a SgDotExp
//! whose lhs is a reference.  In this latter case
//! we will convert the lhs to appear as a pointer.  
//! i.e., returns true if function represents a->method() or (*a).method().
bool SageIRInterface::isArrowExp(SgExpression *function)
{
  if ( isSgArrowExp(function) )
    return true;

  if ( isSgDotExp(function) ) {

    SgBinaryOp *dotOrArrow = isSgBinaryOp(function);
    ROSE_ASSERT(dotOrArrow != NULL);

    SgExpression *lhs = dotOrArrow->get_lhs_operand();
    ROSE_ASSERT(lhs != NULL);

    SgDotExp *dotExp = isSgDotExp(function);
    if ( dotExp != NULL ) {
      
      SgPointerDerefExp *pointerDerefExp =
	isSgPointerDerefExp(lhs);

      if ( pointerDerefExp != NULL ) {
	return true;
      }
#if 0
      if ( isSgReferenceType(lhs->get_type()) ) {
	return true;
      }
#endif
    }

  }

  return false;
}

//! Given a procedure call create a memory reference expression 
//! to describe that call.  For example, a normal call is 
//! a NamedRef.  A call involving a function ptr is a Deref. 
OA::OA_ptr<OA::MemRefExpr> SageIRInterface::getCallMemRefExpr(OA::CallHandle h)
{
  OA::OA_ptr<OA::MemRefExpr> mre;
  mre = NULL;

  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  SgNode *memRefNode = NULL;

  bool useGeneralMRERoutines = false;

  //  // A call handle (i.e., a SgFunctionCallExp or a SgNewExp),
  // A call handle (i.e., a SgFunctionCallExp or a SgConstructorInitializer),
  // may have multiple MemRefExprs.  It certainly has one
  // representing the function/method called.  It may also have
  // one for the return type and args.  Below we judiciously
  // pick out the SgNode that will give us the MRE representing
  // the invocation itself.  This requires intimate knowledge
  // of that process.

  switch(node->variantT()) {
    
  case V_SgFunctionCallExp:
    {

      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      ROSE_ASSERT(functionCallExp != NULL);

      SgExpression *function = functionCallExp->get_function();
      ROSE_ASSERT(function != NULL);

#if 1
      // The general MRE methods will create an MRE if the 
      // function returns an address; is a virtual method call;
      // or is an invocation through a pointer.  In all other
      // cases we should expicitly create the MRE here.
      if ( returnsAddress(functionCallExp) || 
	   isAmbiguousCallThroughVirtualMethod(functionCallExp) ||
	   isSgPointerDerefExp(function) ) {

	// Either this is a methd invocation or an invocation through
	// a function pointer.  In either case the general routines
	// will create an MRE given the SgExpression function:

	// This is a method invocation.

	// We know that the handle to the _method_ invoked represents the
	// the SgDotExp/SgArrowExp of a SgFunctionCallExp.  In the
	// case of a method invocation, we therefore rely on the general
	// getMemRefExprIterator routines to create the MRE.  We pass
	// it the MemRefHandle for the SgDotExpr/SgArrowExp to ensure
	// we get back the MRE that represents the method handle.
	
	// -or-

	// This is an invocation through a function pointer.
	// Again, we can rely on the general methods.

	memRefNode = function;
	useGeneralMRERoutines = true;

      } else {

	// This is a function or a non-virtual method.
	// Create the MRE explicitly.

	// For function invocations, the general routines do not
	// create an MRE (since a function invocation is not an access
	// of program state, the defining criterion for an MRE).  
	// Therefore, we explicitly create the MRE
	// here:  creating a NamedRef based on the SgFunctionRefExp.
	// We do the same for a method.  In this case, we actually
	// have a SgMemberFunctionRefExp.

	useGeneralMRERoutines = false;

	SgExpression *function = getFunction(functionCallExp);
	ROSE_ASSERT(function != NULL);
	SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(function);
	SgMemberFunctionRefExp *memberFunctionRefExp = isSgMemberFunctionRefExp(function);
	ROSE_ASSERT(functionRefExp || memberFunctionRefExp);

	bool addressTaken = false;

	// Access to a named function is a fully
	// accurate representation of that access.
	bool fullAccuracy = true;
	  
	// This is a USE ...
	OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
      
	// Get the declaration of the function.
	SgFunctionSymbol *functionSymbol = NULL;
	if ( functionRefExp )
	  functionSymbol = functionRefExp->get_symbol();
	else 
	  functionSymbol = memberFunctionRefExp->get_symbol();
	ROSE_ASSERT(functionSymbol != NULL);
      
	SgFunctionDeclaration *functionDeclaration = 
	  functionSymbol->get_declaration();
	ROSE_ASSERT(functionDeclaration != NULL);

	// Create an OpenAnalysis handle for this node.
	OA::SymHandle symHandle = getProcSymHandle(functionDeclaration);

	// Create a named memory reference expression.
	mre = new OA::NamedRef(addressTaken, fullAccuracy,
			       memRefType, symHandle);

      }
#else
      if ( isSgArrowExp(function) || isSgDotExp(function) ) {

	// This is a method invocation.

	// We know that the handle to the _method_ invoked represents the
	// the SgDotExp/SgArrowExp of a SgFunctionCallExp.  In the
	// case of a method invocation, we therefore rely on the general
	// getMemRefExprIterator routines to create the MRE.  We pass
	// it the MemRefHandle for the SgDotExpr/SgArrowExp to ensure
	// we get back the MRE that represents the method handle.

	memRefNode = function;
	useGeneralMRERoutines = true;

      } else if ( isSgPointerDerefExp(function) ) {

	// This is an invocation through a function pointer.
	// Again, we can rely on the general methods.

	memRefNode = function;
	useGeneralMRERoutines = true;

      } else {

	// This is a function.  Create the MRE explicitly.

	// For function invocations, the general routines do not
	// create an MRE (since a function invocation is not an access
	// of program state, the defining criterion for an MRE).  
	// Therefore, we explicitly create the MRE
	// here:  creating a NamedRef based on the SgFunctionRefExp.

	useGeneralMRERoutines = false;

	SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(function);
	ROSE_ASSERT(functionRefExp != NULL);

	bool addressTaken = false;

	// Access to a named function is a fully
	// accurate representation of that access.
	bool fullAccuracy = true;
	  
	// This is a USE ...
	OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::USE;
      
	// Get the declaration of the function.
	SgFunctionSymbol *functionSymbol = functionRefExp->get_symbol();
	ROSE_ASSERT(functionSymbol != NULL);
      
	SgFunctionDeclaration *functionDeclaration = 
	  functionSymbol->get_declaration();
	ROSE_ASSERT(functionDeclaration != NULL);

	// Create an OpenAnalysis handle for this node.
	OA::SymHandle symHandle = getProcSymHandle(functionDeclaration);

	// Create a named memory reference expression.
	mre = new OA::NamedRef(addressTaken, fullAccuracy,
			       memRefType, symHandle);

      }
#endif     
      break;
    }

#if 0
  case V_SgNewExp:
    {
      // If the call handle is a SgNewExp, we need to represent
      // the constructor invoked.  The general routines create
      // a USE given the SgConstructorInitializer.

      SgNewExp *newExp = isSgNewExp(node);
      ROSE_ASSERT(newExp != NULL);

      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      memRefNode = ctorInitializer;
      useGeneralMRERoutines = true;

      break;

    }
#endif
  case V_SgConstructorInitializer:
    {

      // If the call handle is a SgNewExp, we need to represent
      // the constructor invoked.  The general routines create
      // a USE given the SgConstructorInitializer.

      SgConstructorInitializer *ctorInitializer =
	isSgConstructorInitializer(node);
      ROSE_ASSERT(ctorInitializer != NULL);

      memRefNode = ctorInitializer;
      useGeneralMRERoutines = true;

      break;

    }

  default: 
    {
      cerr << "Expected a call handle to be a SgFunctionCallExp or" << endl;
      //      cerr << "a SgNewExp!" << endl;
      cerr << "a SgConstructorInitializer!" << endl;
      ROSE_ABORT();
      break;
    }
  }

  if ( useGeneralMRERoutines ) {

    SgStatement *stmt = getEnclosingStatement(node);
    ROSE_ASSERT(stmt != NULL);
    
    OA::StmtHandle stmtHandle = getNodeNumber(stmt);
    ROSE_ASSERT(stmtHandle != (OA::StmtHandle)0);
    
    // Call getMemRefIterator to initialize the sMemref2mreSetMap
    // data structure-- i.e., to ensure that we have a mapping between
    // the MemRefHandle corresponding to this method invocation
    // and its MemRefExpr.
    getMemRefIterator(stmtHandle);
    
    ROSE_ASSERT(memRefNode != NULL);
    OA::MemRefHandle memRefHandle = getNodeNumber(memRefNode);
    
    OA::OA_ptr<OA::MemRefExprIterator> mreIterPtr 
      = getMemRefExprIterator(memRefHandle);
    
    // for each mem-ref-expr associated with this memref
    int numMres = 0;
    for (; mreIterPtr->isValid(); (*mreIterPtr)++) {
      mre = mreIterPtr->current();
      numMres++;
    }
  
    // We are only expecting one MemRefExpr ...
    ROSE_ASSERT(numMres == 1);
  
  }

  // If not using general routines, we assume we have already created
  // the MRE above.
  ROSE_ASSERT(!mre.ptrEqual(0));

  // If we are returning a FieldAccess, it represents a virtual
  // method call.  In this case, the method field acts as a function
  // pointer.  Therefore, dereference the FieldAccess/function pointer:
  // this pointer dereference represents the method invoked.
  if ( isFieldAccess(mre) ) {
    mre = dereferenceMre(mre);
  }

  return mre;
}

//! Given the callee symbol returns the callee proc handle 
OA::ProcHandle SageIRInterface::getProcHandle(OA::SymHandle sym)
{
  // sym is the handle within the NamedRef representing a
  // call.  By design (above in getCallMemRefExpr), this
  // is a procHandle.
  SgNode *node = getNodePtr(sym);
  ROSE_ASSERT(node != NULL);

#if 0
  SgFunctionDefinition *functionDefinition = isSgFunctionDefinition(node);
  ROSE_ASSERT(functionDefinition != NULL);
  return getProcHandle(functionDefinition);
#else
  SgFunctionDeclaration *functionDeclaration = 
    isSgFunctionDeclaration(node);
  ROSE_ASSERT(functionDeclaration != NULL);
  SgFunctionDefinition *functionDefinition =
    functionDeclaration->get_definition();
  return getProcHandle(functionDefinition);
#endif

}

//! If this is a PTR_ASSIGN_STMT then return an iterator over MemRefHandle
//! pairs where there is a source and target such that target
OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator>
SageIRInterface::getPtrAssignStmtPairIterator(OA::StmtHandle stmt)
{   
  OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator> retval;
  retval = new SgPtrAssignPairStmtIterator(stmt, this);
  return retval;
}

OA::OA_ptr<OA::Alias::ParamBindPtrAssignIterator>
SageIRInterface::getParamBindPtrAssignIterator(OA::CallHandle call)
{
  OA::OA_ptr<OA::Alias::ParamBindPtrAssignIterator> retval;
  retval = new SgParamBindPtrAssignIterator(call, this);
  return retval;
}

// Returns true if given symbol is a pass by reference parameter.
// This no longer returns true for a pointer.  Please see isPointerVar.
bool 
SageIRInterface::isRefParam(OA::SymHandle h)
{
  SgNode *node   = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  bool    retVal = false;

  // SymHandle h represents a formal parameter.  We equate
  // SymHandles with SgInitializedNames and SgFunctionDeclarations,
  // but the latter is nonsensical here.
  // According to the note in getFormalParamIterator, a 
  // SymHandle parameter may also be a SgClassSymbol,
  // in which case the function invoked is actually a method
  // and the argument is intended to represent the object
  // upon which it was invoked.

  switch(node->variantT()) {

  case V_SgFunctionDefinition:
    {
      // We represent 'this' by it's methods SgFunctionDefinition.
      // However, it is a pointer param not a reference param.

      retVal = false;

      break;
    }

  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL); 

      SgType *type = initName->get_type(); 
      ROSE_ASSERT(type != NULL); 
      
      SgType *baseType = getBaseType(type);
      ROSE_ASSERT(baseType != NULL);

      retVal = ( isSgReferenceType(baseType) );

      break;
    }

  case V_SgVarRefExp:
    {

      SgVarRefExp *varRefExp = isSgVarRefExp(node); 
      ROSE_ASSERT(varRefExp != NULL);

      SgVariableSymbol *symbol = varRefExp->get_symbol(); 
      ROSE_ASSERT(symbol != NULL); 

      SgInitializedName *initName = symbol->get_declaration(); 
      ROSE_ASSERT(initName != NULL); 

      SgType *type = initName->get_type(); 
      ROSE_ASSERT(type != NULL); 
      
      SgType *baseType = getBaseType(type);
      ROSE_ASSERT(baseType != NULL);

      retVal = ( isSgReferenceType(baseType) );

      break;
    }

  default:
    {

      retVal = false;
      
      break;
    }

  }

  return retVal;
}

// Returns true if the given MemRefExpr represents a reference variable.
bool SageIRInterface::isReferenceExpr(OA::OA_ptr<OA::MemRefExpr> mre)
{
  // First look up the MemRefHandle associated with this MemRefExpr.
  if ( sMre2MemrefMap.find(mre) != sMre2MemrefMap.end() ) {
    OA::MemRefHandle memRefHandle = sMre2MemrefMap[mre];
    
    SgNode *node = getNodePtr(memRefHandle);
    ROSE_ASSERT(node != NULL);
    
    OA::SymHandle symHandle = getNodeNumber(node);
    return isRefParam(symHandle);
  }

  // Some MemRefExprs may not be registered in sMre2MemrefMap.  
  // e.g., ManagerFIAlias creates new MemRefExprs for various
  // reasons, including to represent formal parameters.
  // If the MemRefExpr is named and is fully accurate 
  // (i.e., does not represent a field access) and its
  // symbol corresponds to a reference variable, return true.
  if ( mre->isaNamed() ) {
    
    OA::OA_ptr<OA::NamedRef> namedRef = mre.convert<OA::NamedRef>();
    ROSE_ASSERT(!namedRef.ptrEqual(0));

    OA::SymHandle symHandle = namedRef->getSymHandle();
    return isRefParam(symHandle);
  }

  return false;
}

// Returns true if given symbol is a pointer parameter.
bool 
SageIRInterface::isPointerVar(OA::SymHandle h)
{
  SgNode *node   = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  bool    retVal = false;

  // SymHandle h represents a formal parameter.  We equate
  // SymHandles with SgInitializedNames and SgFunctionDeclarations,
  // but the latter is nonsensical here.
  // According to the note in getFormalParamIterator, a 
  // SymHandle parameter may also be a SgClassSymbol,
  // in which case the function invoked is actually a method
  // and the argument is intended to represent the object
  // upon which it was invoked.

  switch(node->variantT()) {

  case V_SgFunctionDefinition:
    {
      // We represent 'this' by it's methods SgFunctionDefinition.
      // It is a pointer param.

      retVal = true;

      break;
    }

  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL); 

      SgType *type = initName->get_type(); 
      ROSE_ASSERT(type != NULL); 
      
      SgType *baseType = getBaseType(type);
      ROSE_ASSERT(baseType != NULL);

      retVal = ( isSgPointerType(baseType) );

      break;
    }

  case V_SgFunctionParameterList:
    {
      // We represent the formal parameter corresponding to the
      // implicit 'this' pointer via a SgFunctionParameterList.

      retVal = true;

      break;
    }

  default:
    {

      cerr << "SageIRInterface::isPointerVar(OA::SymHandle h) didn't know" << endl;
      cerr << "how to handle a SymHandle of type: " << node->sage_class_name() << endl;
      ROSE_ABORT();
      
      break;
    }

  }

  return retVal;
}
  


//-------------------------------------------------------------------------
// SSAIRInterfaceDefault
//-------------------------------------------------------------------------


// Getsymhandle Given a LeafHandle containing a use or def, return
// the referened SymHandle.
OA::SymHandle SageIRInterface::getSymHandle(OA::LeafHandle h)
{
  // If this is a use or a def, I assume it must be either a 
  // SgVarRefExp or a SgInitializedName.  Notice that a LeafHandle
  // is an ExprHandle and a SgInitializedName is not a SgExpression.

  // We return a SymHandle, which is a handle to a SgInitializedName,
  // a SgFunctionDeclaration, or a SgValueExp.  However, in this
  // context only a SgInitializedName makes sense.

  SgNode *node            = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  OA::SymHandle retHandle = 0;

  switch(node->variantT()) {

  case V_SgVarRefExp:
    {
      SgVarRefExp *varRefExp = isSgVarRefExp(node); 
      ROSE_ASSERT(varRefExp != NULL);

      SgVariableSymbol *symbol = varRefExp->get_symbol(); 
      ROSE_ASSERT(symbol != NULL); 

      SgInitializedName *initName = symbol->get_declaration(); 
      ROSE_ASSERT(initName != NULL); 

      retHandle = (OA::irhandle_t)(getNodeNumber(initName));

      break;
    }

  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL); 

      retHandle = (OA::irhandle_t)(getNodeNumber(initName));

      break;
    }
  default:
    {
      cerr << "SageIRInterface::getSymHandle(OA::LeafHandle h) didn't know" << endl;
      cerr << "how to handle a LeafHandle of type: " << node->sage_class_name() << endl;
      ROSE_ABORT();
    }
  }

  return retHandle;

}


//-------------------------------------------------------------------------
// ParamBindingsIRInterface
//-------------------------------------------------------------------------

// return the formal parameter that an actual parameter is associated with 
OA::SymHandle SageIRInterface::getFormalForActual(OA::ProcHandle caller, 
						  OA::ExprHandle call, 
						  OA::ProcHandle callee, 
						  OA::ExprHandle param)
{
  OA::SymHandle retFormal = 0;

  // Get the ordered list of callsite parameters.
  OA::OA_ptr<OA::IRCallsiteParamIterator> callsiteParamIter =
    getCallsiteParams(call);

  // Determine the ordinal number of the actual parameter within the
  // list of parameters.
  int  n          = 1;
  bool foundParam = false;
  for ( ; callsiteParamIter->isValid(); n++, (*callsiteParamIter)++ ) { 
    
    OA::ExprHandle callsiteParam = callsiteParamIter->current(); 
    if ( callsiteParam == param ) {
      foundParam = true;
      break;
    }
  }
  
  if ( !foundParam ) {
    SgNode *node = getNodePtr(param);
    ROSE_ASSERT(node != NULL);
    cerr << "Could not find actual parameter (of type " << node->sage_class_name() << ") within list of parameters of types (";
    
    callsiteParamIter->reset();
    for ( ; callsiteParamIter->isValid(); (*callsiteParamIter)++ ) { 
    
      OA::ExprHandle callsiteParam = callsiteParamIter->current(); 
      node = getNodePtr(callsiteParam);
      ROSE_ASSERT(node != NULL);
      cerr << node->sage_class_name() << " ";
    }

    cerr << ")" << endl;
    ROSE_ABORT();

  }

  // At this point, we know that param is the nth actual parameter
  // of the call.  Now find and return the nth formal parameter.
  OA::SymHandle procSymHandle = getProcSymHandle(callee);
  OA::OA_ptr<OA::IRFormalParamIterator> formalParamIter =
    getFormalParamIterator(procSymHandle);

  int  m             = 1;
  bool foundNthParam = false;
  for ( ; formalParamIter->isValid(); m++, (*formalParamIter)++ ) { 
    
    retFormal = formalParamIter->current(); 
    if ( m == n ) {
      foundNthParam = true;
      break;
    }

  }

  if ( !foundNthParam ) {
    
    // There were less than n parameters!  If the last formal parameter
    // was a vararg, then return that.
    bool lastArgIsVarArg = false;
    if ( (int)retFormal != 0 ) {

      SgNode *node = getNodePtr(retFormal);
      ROSE_ASSERT(node != NULL);

      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL);

      SgType *type = initName->get_type();
      ROSE_ASSERT(type != NULL);

      SgType *baseType = getBaseType(type);
      ROSE_ASSERT(baseType != NULL);

      lastArgIsVarArg = ( isSgTypeEllipse(baseType) != NULL );
    }

    if (!lastArgIsVarArg) {

      cerr << "Failed to find parameter #" << n;
      cerr << " in a function with only " << m - 1 << " parameters" << endl;
      ROSE_ABORT();

    }
  }

  return retFormal;
}


/*
 * class ExprTreeTraversal traverses a tree of SgNodes to
 * create an ExprTree that mirrors it.  Nodes in the ExprTree
 * are of the following type and correspond to nodes of specific
 * types in the Sage Tree:
 *
 * ExprTree Node Type           Sage Tree Node Types
 * MemRefNode                   see isMemRefNode
 * OpNode                       SgUnaryOp, SgBinaryOp
 * CallNode                     SgFunctionCallExp, SgDeleteExp, SgNewExp, 
 *                              SgSizeOfExp, SgVarArgCopyOp, SgVarArgEndOp, 
 *                              SgVarArgOp, SgVarArgStartOneOperandOp, 
 *                              SgVarArgStartOp
 * ConstSymNode                 SgVarRefExp (with const type modifier),
 *                              SgThisExp (with const type modifier)
 * ConstValNode                 SgValueExp
 * MemRefNode                   see isMemRefNode
 * Node                         all others
 *
 * NB:  MemRefNode seems to overlap with some of the others.  e.g.,
 *      a SgPlusPlusOp is a MemRefNode, but is also an OpNode.
 *      Similarly, a SgFunctionCallExp can be a MemRefNode and
 *      is certainly a CallNode.  Perhaps we should have
 *      OpMemRefNodes and CallMemRefNodes?
 *      For now, we favor MemRefNodes.  i.e., we first check if
 *      the node is a MemRefNode, if not we look to the other cases.
 */

class ExprTreeTraversal
  : public AstTopDownProcessing<OA::OA_ptr<OA::ExprTree::Node> >
{ 
 public: 

  ExprTreeTraversal(OA::OA_ptr<OA::ExprTree> exprTree,
		    SageIRInterface *IR) 
    : mExprTree(exprTree), mIR(IR)  
  { }
   
  virtual  
  OA::OA_ptr<OA::ExprTree::Node>
  evaluateInheritedAttribute(SgNode *astNode, 
			     OA::OA_ptr<OA::ExprTree::Node> inheritedAttribute)
  { 

    // This is a top-down traversal and we are passing the parent
    // within the ExprTree as the inherited attribute.
    OA::OA_ptr<OA::ExprTree::Node> parent = inheritedAttribute;

    if ( isSgVarRefExp(astNode) || isSgThisExp(astNode) ) {

      // ConstSymNode (with const type modifier) or
      // MemRefNode (without const type modifier)
      SgType *type    = NULL;
      bool    isConst = false;
      
      SgVarRefExp *varRefExp = isSgVarRefExp(astNode);
      SgThisExp   *thisExp   = isSgThisExp(astNode);
      
      // Get the type of this AST Node.
      if (varRefExp != NULL) {
	type = varRefExp->get_type();
      } else if (thisExp != NULL) {
	type = thisExp->get_type();
      }
      
      // Determine whether the type has a const modifier.
      ROSE_ASSERT(type != NULL);
      
      SgModifierNodes *modifierNodes = type->get_modifiers(); 

      if (modifierNodes != NULL) {
	SgModifierTypePtrVector modifierVector = modifierNodes->get_nodes(); 
	for(SgModifierTypePtrVector::iterator it = modifierVector.begin(); 
	    it != modifierVector.end(); ++it) { 
	  
	  SgModifierType *modifierType = *it; 
	  ROSE_ASSERT(modifierType != NULL); 
	  
	  SgTypeModifier &typeModifier = modifierType->get_typeModifier(); 
	  
	  SgConstVolatileModifier &constVolatileModifier = 
	    typeModifier.get_constVolatileModifier(); 
	  
	  if ( constVolatileModifier.isConst() ) { 
	    
	    isConst = true;
	    break;
	    
	  } 
        }
      } 
      
      if ( isConst ) {
	
	// This is a ConstSymNode.
	OA::ConstSymHandle h = mIR->getNodeNumber(astNode);
	OA::OA_ptr<OA::ExprTree::ConstSymNode> node;
	node = new OA::ExprTree::ConstSymNode(h);
	if ( !parent.ptrEqual(NULL) ) {
	  mExprTree->connect(parent, node);
	} else {
	  mExprTree->addNode(node);
	}
	parent = node;
	
      } else { 
	
	// This is a MemRefNode.
	OA::MemRefHandle h = mIR->getNodeNumber(astNode);
	OA::OA_ptr<OA::ExprTree::MemRefNode> node;
	node = new OA::ExprTree::MemRefNode(h);
	if ( !parent.ptrEqual(NULL) ) {
	  mExprTree->connect(parent, node);
	} else {
	  mExprTree->addNode(node);
	}
	parent = node;
	
      }

    } else if ( mIR->isMemRefNode(astNode) ) {

      // This is a MemRefNode.
      OA::MemRefHandle h = mIR->getNodeNumber(astNode);
      OA::OA_ptr<OA::ExprTree::MemRefNode> node;
      node = new OA::ExprTree::MemRefNode(h);
      if ( !parent.ptrEqual(NULL) ) {
	mExprTree->connect(parent, node);
      } else {
	mExprTree->addNode(node);
      }
      parent = node;

    } else if ( isSgUnaryOp(astNode) || isSgBinaryOp(astNode) ) {
      // OpNode
      OA::OpHandle h = mIR->getNodeNumber(astNode);
      OA::OA_ptr<OA::ExprTree::OpNode> node;
      node = new OA::ExprTree::OpNode(h);
      if ( !parent.ptrEqual(NULL) ) {
	mExprTree->connect(parent, node);
      } else {
	mExprTree->addNode(node);
      }
	
      parent = node;
      
    } else if ( isSgDeleteExp(astNode) || isSgNewExp(astNode) || 
		isSgSizeOfOp(astNode) || isSgVarArgCopyOp(astNode) || 
		isSgVarArgEndOp(astNode) || isSgVarArgOp(astNode) || 
		isSgVarArgStartOneOperandOp(astNode) || 
		isSgVarArgStartOp(astNode) ) {
      // CallNode
      OA::ExprHandle h = mIR->getNodeNumber(astNode);
      OA::OA_ptr<OA::ExprTree::CallNode> node;
      node = new OA::ExprTree::CallNode(h);
      if ( !parent.ptrEqual(NULL) ) {
	mExprTree->connect(parent, node);
      } else {
	mExprTree->addNode(node);
      }
      parent = node;
    
    } else if ( isSgValueExp(astNode) ) {
    
      // ConstValNode
      OA::ConstValHandle h = mIR->getNodeNumber(astNode);
      OA::OA_ptr<OA::ExprTree::ConstValNode> node; 
      node = new OA::ExprTree::ConstValNode(h);
      if ( !parent.ptrEqual(NULL) ) {
	mExprTree->connect(parent, node);
      } else {
	mExprTree->addNode(node);
      }
      parent = node;
      
    } else {
#if 0
      // (vanilla) Node
      OA::ExprHandle h = mIR->getNodeNumber(astNode);
      OA::OA_ptr<OA::ExprTree::Node> node; 
      node = new OA::ExprTree::Node(h);
      if ( !parent->ptrEqual(NULL) ) {
	mExprTree->connect(parent, node);
      }
      parent = node;
#endif
      cerr << "Can not create a vanilla node because it is virtual." << endl;
      cerr << "Sage type " << astNode->sage_class_name() << " must correspond to a derived OA::ExprTree node type" << endl;
      ROSE_ABORT();
    }

    return parent;
  
  }

 private:
  OA::OA_ptr<OA::ExprTree>       mExprTree;
  SageIRInterface               *mIR;
};

// Given an ExprHandle, return an ExprTree 
OA::OA_ptr<OA::ExprTree> SageIRInterface::getExprTree(OA::ExprHandle h)
{
  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  OA::OA_ptr<OA::ExprTree> exprTree;
  exprTree = new OA::ExprTree();
  ExprTreeTraversal buildExprTree(exprTree, this);
  OA::OA_ptr<OA::ExprTree::Node> inheritedAttribute;
  inheritedAttribute = NULL;
  buildExprTree.traverse(node, inheritedAttribute);

  return exprTree;
}

// Get IRCallsiteParamIterator for a callsite. 
// Iterator visits actual parameters in called order. 
OA::OA_ptr<OA::IRCallsiteParamIterator> 
SageIRInterface::getCallsiteParams(OA::ExprHandle h)
{
  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  SgExprListExp* exprListExp = NULL;

  // Create a list to hold the handles to the actual arguments.
  OA::OA_ptr<std::list<OA::ExprHandle> > exprHandleList;
  exprHandleList = new std::list<OA::ExprHandle>;

  // A callsite is represented in Sage as a SgFunctionCallExp or
  //  // as a SgNewExp.
  // as a SgConstructorInitializer.

  switch(node->variantT()) {
    
  case V_SgFunctionCallExp:
    {
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
      ROSE_ASSERT(functionCallExp != NULL);

      // Get the list of actual arguments from the function call.
      exprListExp = functionCallExp->get_args();  
      ROSE_ASSERT (exprListExp != NULL);  
   
      // If this is a method call, fold the object upon which the
      // method is invoked into the argument list as the first argument.
      // OpenAnalysis doesn't know about methods and wouldn't be otherwise
      // able to account, e.g., for MODs and USEs of the 'this' object.
      SgExpression *function = functionCallExp->get_function();
      ROSE_ASSERT(function != NULL);
      
      if ( isSgDotExp(function) || isSgArrowExp(function) ) {
#if 1
	SgBinaryOp *dotOrArrow = isSgBinaryOp(function);
	ROSE_ASSERT(dotOrArrow != NULL);
	
	SgNode *lhs = dotOrArrow->get_lhs_operand();
	ROSE_ASSERT(lhs != NULL);
	
	SgDotExp *dotExp = isSgDotExp(function);
	if ( dotExp != NULL ) {
	  
	  SgPointerDerefExp *pointerDerefExp =
	    isSgPointerDerefExp(lhs);
	  
	  if ( pointerDerefExp != NULL ) {
	    
	    // The function is a dot expression whose lhs is
	    // a pointer dereference.  This is equivalent
	    // to an arrow expression.  Do not return the
	    // SgPointerDerefExp as a MemRefHandle since
	    // getMemRefExprIterator does not return a MemRefExpr
	    // for such a MemRefHandle, assuming instead that
	    // MemRefExprs are associated with the SgDotExp.
	    // Since this is equivalent to an arrow expression,
	    // we really want to return the lhs of the arrow--
	    // i.e., the operand of the SgPointerDerefExp.
	    lhs = pointerDerefExp->get_operand_i();
	    
	  }
	  
	}
	
	lhs = lookThroughCastExpAndAssignInitializer(lhs);
	ROSE_ASSERT( isMemRefNode(lhs) );
	OA::ExprHandle exprHandle = getNodeNumber(lhs);
#else
	// Represent the this actual parameter 
	OA::ExprHandle exprHandle = getNodeNumber(function);
#endif
	exprHandleList->push_back(exprHandle);

      }

      break;
    }

#if 0
  case V_SgNewExp:
    {
      SgNewExp *newExp = isSgNewExp(node);
      ROSE_ASSERT(newExp != NULL);

      SgConstructorInitializer *ctorInitializer =
	newExp->get_constructor_args();
      ROSE_ASSERT(ctorInitializer != NULL);

      exprListExp = ctorInitializer->get_args();
      ROSE_ASSERT (exprListExp != NULL);  
   
      // As above, this is a method, so fold the object upon which the
      // method is invoked into the argument list as the first argument.
      // This may seem a little strange since we may not consider
      // a constructor to be invoked on an object.

      SgNode *lhs = getNewLhs(newExp);
      if ( lhs != NULL ) {
	lhs = lookThroughCastExpAndAssignInitializer(lhs);
      }

      // NB:  lhs could be NULL here, e.g., 'return (new B)'
      OA::ExprHandle exprHandle = getNodeNumber(lhs);
      exprHandleList->push_back(exprHandle);

      break;
    }
#endif
  case V_SgConstructorInitializer:
    {
      SgConstructorInitializer *ctorInitializer =
	isSgConstructorInitializer(node);
      ROSE_ASSERT(ctorInitializer != NULL);

      exprListExp = ctorInitializer->get_args();
      ROSE_ASSERT (exprListExp != NULL);  
   
      // As above, this is a method, so fold the object upon which the
      // method is invoked into the argument list as the first argument.
      // This may seem a little strange since we may not consider
      // a constructor to be invoked on an object.
      SgNode *lhs = getConstructorInitializerLhs(ctorInitializer);
      if ( lhs != NULL ) {
	lhs = lookThroughCastExpAndAssignInitializer(lhs);
      }

      // NB:  lhs could be NULL here, e.g., 'return (new B)'
      OA::ExprHandle exprHandle = getNodeNumber(lhs);
      exprHandleList->push_back(exprHandle);

      break;
    }
  default:
    {
      cerr << "Expected a callHandle to be either a SgFunctionCallExp" << endl;
      //      cerr << "or a SgNewExp, instead got a " << node->sage_class_name() << endl;
      cerr << "or a SgConstructorInitializer, instead got a " << node->sage_class_name() << endl;
      ROSE_ABORT();
      break;
    }
  }

  ROSE_ASSERT(exprListExp != NULL);

  SgExpressionPtrList & actualArgs =  
    exprListExp->get_expressions();  

  // Iterate over the actual arguments as represented by
  // SgExpressions.  Convert them to OA::ExprHandle and put
  // them in the list of handles.
  for(SgExpressionPtrList::iterator actualIt = actualArgs.begin(); 
      actualIt != actualArgs.end(); ++actualIt) { 
 
    SgExpression *actualArg = *actualIt;
    ROSE_ASSERT(actualArg != NULL);

    OA::ExprHandle exprHandle = getNodeNumber(actualArg);
    exprHandleList->push_back(exprHandle);

  }

  OA::OA_ptr<OA::IRCallsiteParamIterator> retIter;
  retIter = new SageIRCallsiteParamIterator(exprHandleList);
  return retIter;

}

// Return the lhs of a constructor initializer.
SgNode *SageIRInterface::getConstructorInitializerLhs(SgConstructorInitializer *ctorInitializer)
{

  if ( ctorInitializer == NULL ) return NULL;

  SgNode *lhs = NULL;

  // Recurse up the parents of ctorInitializer.  Return the lhs
  // of an assignment or the SgInitializedName of a
  // SgAssignInitializer.  These handle a new expression.
  // If the parent of the constructor initializer is a 
  // SgInitializedName, return that.  This handles a constructor
  // invoked through a stack declaration.  Stop the recursion and return
  // NULL if we reach a SgStatement without first finding
  // any of these cases.

  SgNode *parent = ctorInitializer->get_parent();

  if ( isSgInitializedName(parent) )
    return parent;

  bool expectInit = false;

  while ( parent != NULL ) {

    if ( isSgStatement(parent) ) break;

    if ( isSgAssignInitializer(parent) ) {
      expectInit = true;
    } else if ( ( expectInit ) && ( isSgInitializedName(parent) ) ) {
      lhs = parent;
      break;
    } else {
      SgAssignOp *assignOp = isSgAssignOp(parent);
      if ( assignOp ) {
	lhs = assignOp->get_lhs_operand();
	break;
      }
    }

    parent = parent->get_parent();

  }

  return lhs;

}

// Return the lhs of a new expression.
SgNode *SageIRInterface::getNewLhs(SgNewExp *newExp) {

  if ( newExp == NULL ) return NULL;

  SgNode *lhs = NULL;

  // Recurse up the parents of newExp.  Return the lhs
  // of an assignment or the SgInitializedName of a
  // SgAssignInitialize.  Stop the recursion and return
  // NULL if we reach a SgStatement without first finding
  // either of the above.

  SgNode *parent = newExp->get_parent();

  bool expectInit = false;

  while ( parent != NULL ) {

    if ( isSgStatement(parent) ) break;

    if ( isSgAssignInitializer(parent) ) {
      expectInit = true;
    } else if ( ( expectInit ) && ( isSgInitializedName(parent) ) ) {
      lhs = parent;
      break;
    } else {
      SgAssignOp *assignOp = isSgAssignOp(parent);
      if ( assignOp ) {
	lhs = assignOp->get_lhs_operand();
	break;
      }
    }

    parent = parent->get_parent();

  }

  return lhs;

}
 
//! get iterator over formal parameters in a function declaration 
OA::OA_ptr<OA::IRFormalParamIterator> 
SageIRInterface::getFormalParamIterator(OA::SymHandle h)
{
  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  // SymHandle must be a function declaration in this context.
  // A function declaration is represented in Sage as a SgFunctionDeclaration.
  // NB:  constructors are also represented by SgFunctionDeclarations.
  SgFunctionDeclaration *functionDeclaration = isSgFunctionDeclaration(node);
  ROSE_ASSERT(functionDeclaration != NULL);

  // Create a list to hold the handles to the formal parameters.
  OA::OA_ptr<std::list<OA::SymHandle> > symHandleList;
  symHandleList = new std::list<OA::SymHandle>;

  // If this is a method (including a constructor), fold something to
  // correspond to the implicit actual 'this' into the argument list as 
  // the first formal.
  // OpenAnalysis doesn't know about methods and wouldn't be otherwise
  // able to account, e.g., for MODs and USEs of the 'this' object.

  SgMemberFunctionDeclaration *memberFunctionDeclaration =
    isSgMemberFunctionDeclaration(functionDeclaration);
  if ( memberFunctionDeclaration != NULL ) {
    OA::SymHandle symHandle = getThisExpSymHandle(memberFunctionDeclaration);
    symHandleList->push_back(symHandle);
    
  }
 
  // Get the list of formal parameters for the function.
  SgFunctionParameterList *parameterList = 
    functionDeclaration->get_parameterList(); 

  if (parameterList != NULL) {

    // Iterate over the formal parameters as represented by
    // SgInitializedNames.  Convert them to OA::SymHandles and put
    // them in the list of handles.
    const SgInitializedNamePtrList &formalParams = parameterList->get_args(); 
    for(SgInitializedNamePtrList::const_iterator formalIt = formalParams.begin();
	formalIt != formalParams.end(); ++formalIt) { 
      
      SgInitializedName* formalParam = *formalIt;  
      ROSE_ASSERT(formalParam != NULL); 
      
      OA::SymHandle symHandle = getNodeNumber(formalParam);
      symHandleList->push_back(symHandle);
      
    }

  }

  OA::OA_ptr<OA::SymHandleIterator> retIter;
  retIter = new SageIRFormalParamIterator(symHandleList);
  return retIter;
}

// For now I don't see any need for this method.
void SageIRInterface::currentProc(OA::ProcHandle p)
{

} 
 
//-------------------------------------------------------------------------
// output methods
//-------------------------------------------------------------------------

string SageIRInterface::refTypeToString(OA::OA_ptr<OA::MemRefExpr> memRefExp)
{
  string refTypeStr;

  if ( memRefExp->isUseDef() ) refTypeStr = "USEDEF";
  else if ( memRefExp->isDefUse() ) refTypeStr = "DEFUSE";
  else if ( memRefExp->isDef() ) refTypeStr = "DEF";
  else if ( memRefExp->isUse() ) refTypeStr = "USE";
  else ROSE_ABORT();

  return refTypeStr;
}

void SageIRInterface::dump(OA::OA_ptr<OA::NamedRef> memRefExp, 
			       std::ostream& os) 
{
  OA::SymHandle symHandle = memRefExp->getSymHandle();

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Named Ref: addressTaken = F, fullAccuracy = full.
  bool addressTaken = memRefExp->hasAddressTaken();
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "NamedRef(";
  os << refTypeToString(memRefExp);
  os << ", SymHandle(" << toString(symHandle) << ")";
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

void SageIRInterface::dump(OA::OA_ptr<OA::UnnamedRef> memRefExp, 
			       std::ostream& os) 
{
  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Unnamed Ref: addressTaken = T, fullAccuracy = partial.
  bool addressTaken = memRefExp->hasAddressTaken();
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "UnnamedRef(";
  os << refTypeToString(memRefExp);
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

void SageIRInterface::dump(OA::OA_ptr<OA::UnknownRef> memRefExp, 
			       std::ostream& os) 
{
  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Unknown Ref: addressTaken = F, fullAccuracy = partial.
  bool addressTaken = memRefExp->hasAddressTaken();
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "UnknownRef(";
  os << refTypeToString(memRefExp);
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

void SageIRInterface::dump(OA::OA_ptr<OA::Deref> memRefExp, 
			       std::ostream& os) 
{
  int numDerefs = memRefExp->getNumDerefs();
  OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = memRefExp->getMemRefExpr();

  // Only print addressTaken and fullAccuracy if they
  // don't have the default values.
  // Default values for Deref: addressTaken = F, fullAccuracy = partial.
  bool addressTaken = memRefExp->hasAddressTaken();
  bool fullAccuracy = memRefExp->hasFullAccuracy();

  os << "Deref(";
  os << refTypeToString(memRefExp) << ", ";
  dump(baseMemRefExpr, os);
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

void SageIRInterface::dump(OA::OA_ptr<OA::MemRefExpr> memRefExp, 
			       std::ostream &os)
{

  if ( memRefExp->isaNamed() ) {

    OA::OA_ptr<OA::NamedRef> namedRef = memRefExp.convert<OA::NamedRef>();
    ROSE_ASSERT(!namedRef.ptrEqual(0));

    dump(namedRef, os);

  } else if ( memRefExp->isaUnnamed() ) {

    OA::OA_ptr<OA::UnnamedRef> unnamedRef = memRefExp.convert<OA::UnnamedRef>();
    ROSE_ASSERT(!unnamedRef.ptrEqual(0));

    dump(unnamedRef, os);

  } else if ( memRefExp->isaUnknown() ) {

    OA::OA_ptr<OA::UnknownRef> unknownRef = memRefExp.convert<OA::UnknownRef>();
    ROSE_ASSERT(!unknownRef.ptrEqual(0));

    dump(unknownRef, os);

  } else if ( memRefExp->isaRefOp() ) {

    OA::OA_ptr<OA::RefOp> refOp = memRefExp.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    if ( refOp->isaDeref() ) {

      OA::OA_ptr<OA::MemRefExpr> baseMemRefExpr = refOp->getMemRefExpr();
      OA::OA_ptr<OA::Deref> deref = refOp.convert<OA::Deref>();
      ROSE_ASSERT(!deref.ptrEqual(0));

      dump(deref, os);

    } else {

      cerr << "Unknown memRefExp type!" << endl;
      ROSE_ABORT();

    }

  } else {

    cerr << "Unknown memRefExp type!" << endl;
    ROSE_ABORT();

  }
}

void SageIRInterface::dump(OA::OA_ptr<OA::NamedLoc> loc, ostream& os)
{
  OA::SymHandle symHandle = loc->getSymHandle();
  bool local = loc->isLocal();

  os << "NamedLoc(symHandle = " << toString(symHandle);
  os << ", mLocal = " << local << ")";
}

void SageIRInterface::dump(OA::OA_ptr<OA::UnnamedLoc> loc, ostream& os)
{
  OA::StmtHandle stmtHandle = loc->getStmtHandle();

  os << "UnnamedLoc(stmtHandle = " << toString(stmtHandle) << ")";
}

void SageIRInterface::dump(OA::OA_ptr<OA::InvisibleLoc> loc, ostream& os)
{
  OA::OA_ptr<OA::MemRefExpr> memRefExpr = loc->getMemRefExpr();
  os << "InvisibleLoc(memRefExpr = ";
  dump(memRefExpr, os);
  os << ")";
}

void SageIRInterface::dump(OA::OA_ptr<OA::UnknownLoc> loc, ostream& os)
{
  os << "UnknownLoc";
}

void SageIRInterface::dump(OA::OA_ptr<OA::Location> loc, ostream& os)
{
  if ( loc->isaNamed() ) {

    OA::OA_ptr<OA::NamedLoc> namedLoc = loc.convert<OA::NamedLoc>();
    ROSE_ASSERT(!namedLoc.ptrEqual(0));

    dump(namedLoc, os);

  } else if ( loc->isaUnnamed() ) {

    OA::OA_ptr<OA::UnnamedLoc> unnamedLoc = loc.convert<OA::UnnamedLoc>();
    ROSE_ASSERT(!unnamedLoc.ptrEqual(0));

    dump(unnamedLoc, os);

  } else if ( loc->isaUnknown() ) {

    OA::OA_ptr<OA::UnknownLoc> unknownLoc = loc.convert<OA::UnknownLoc>();
    ROSE_ASSERT(!unknownLoc.ptrEqual(0));

    dump(unknownLoc, os);

  } else if ( loc->isaInvisible() ) {

    OA::OA_ptr<OA::InvisibleLoc> invisibleLoc = loc.convert<OA::InvisibleLoc>();
    ROSE_ASSERT(!invisibleLoc.ptrEqual(0));

    dump(invisibleLoc, os);

  } else if ( loc->isaSubSet() ) {
    os << "Have not implemented SubSet output" << endl;
  }

}

//! Given a subprogram return an IRSymIterator for all
//! symbols that are referenced within the subprogram
OA::OA_ptr<OA::IRSymIterator> 
SageIRInterface::getRefSymIterator(OA::ProcHandle h)
{
  ROSE_ABORT();
}

//! returns true if given symbol is a parameter 
bool SageIRInterface::isParam(OA::SymHandle)
{
  ROSE_ABORT();
}

//! Given a statement, return uses (variables referenced)
OA::OA_ptr<OA::SSA::IRUseDefIterator> 
SageIRInterface::getUses(OA::StmtHandle h)
{
  ROSE_ABORT();
}

//! Given a statement, return defs (variables defined)
OA::OA_ptr<OA::SSA::IRUseDefIterator> 
SageIRInterface::getDefs(OA::StmtHandle h)
{
  ROSE_ABORT();
}

// Return the method in which node occurs.
SgFunctionDefinition *SageIRInterface::getEnclosingMethod(SgNode *node)
{
  if ( node == NULL ) return NULL;

  if ( isSgGlobal(node) ) return NULL;

  SgFunctionDefinition *functionDefinition =
    isSgFunctionDefinition(node);
  
  if ( functionDefinition != NULL )
    return functionDefinition;

  SgFunctionDeclaration *functionDeclaration =
    isSgFunctionDeclaration(node);

  if ( functionDeclaration != NULL )
    return getEnclosingMethod(functionDeclaration->get_definition());
  
  return getEnclosingMethod(node->get_parent());
}

// Return the statement in which node occurs.
SgStatement *SageIRInterface::getEnclosingStatement(SgNode *node)
{
  if ( node == NULL ) return NULL;

  SgStatement *stmt = isSgStatement(node);
  
  if ( stmt != NULL )
    return stmt;

  return getEnclosingStatement(node->get_parent());
}


// Return the class in which node occurs.
SgClassDefinition *SageIRInterface::getDefiningClass(SgScopeStatement *scope)
{
  if ( scope == NULL ) return NULL;

  // The node is not defined in a class.
  if ( isSgGlobal(scope) ) return NULL;

  if ( isSgFunctionDeclaration(scope) ) return NULL;

  if ( isSgFunctionDefinition(scope) ) return NULL;

  // In the following example:
  // class vector {
  //  public:
  //  void vecAdd(double v[NDIM]) {
  //    for(int i = 0; i < NDIM; i++)
  //      val[i] += v[i];
  //  }
  // };
  // i is technically defined within the class, since it
  // is defined within vecAdd which is defined within the
  // class.  But this method, getDefiningClass, really
  // wants to return the class of which a variable is a member.
  // therefore, if we find a scope along the way that is
  // a function/method, we should return false.

  SgClassDefinition *classDef = isSgClassDefinition(scope);

  if ( classDef ) 
    return classDef;

  return getDefiningClass(scope->get_scope());
}

// Given a SgNode return a symbol handle to represent the
// corresponding SgThisExp.  
#if 0
  // NB:  This is truly awful.  I'm stuffing a SgClassDefinition*
  //      in where a SymHandle is expected.  Other code needs to
  //      be wary that when it a SymHandle is converted to a
  //      SgClassDefinition, it should be interpreted as a this pointer.
  //      I would love to put a SgThisExp here instead.  Unfortunately,
  //      we aren't guaranteed to be able to find one within the
  //      method (e.g., if it does invoke any other methods or
  //      access any member variables).
#else
  // Now the symbol handle points to a SgClassSymbol.
  // Changed again:  now the symbol handle points to the 
  // SgFunctionParameterList.  This is an odd choice.
  // First, understand how this formal will be used:  alias analysis
  // will union it will the corresponding actuals and their alias
  // sets.  If the 'this' formal were represented by something that
  // was (only) class-specific, such as a SgClassSymbol, then
  // A *a1 = new A;
  // A *a2 = new A;
  // a->foo();
  // a->bar();
  // would result in *a1 and *a2 being aliased, since they both
  // would be bound to the same formal argument, represented by a 
  // SgClassSymbol, shared by foo and bar.  If instead the 'this'
  // formal for foo and bar were specific to each respective method,
  // then *a1 and *a2 would not be aliased above.  Though they
  // would be if both called the same method, e.g., a->foo().
  // The best choice is SgFunctionDeclaration.  Unfortunately, we
  // already use to represent the method (e.g., as returned by
  // getCallMemRefExpr) and within implicit pointer assignment
  // pairs.  Were we to return a SgFunctionDeclaration here,
  // it might collide with these and we would get unexpected
  // aliases.  
  // The next obvious choice is a SgFunctionDefinition.  Unfortunately,
  // the definition may not be visible/accessible.  Possibly
  // revisit this:  alias analysis must be a whole program analysis
  // to be correct.  Implies we have access to function/method
  // definitions.  Not true, e.g., consider sys calls which might
  // be represented by stubs.
  // Therefore, by default we use SgFunctionParameterList.
#endif
OA::SymHandle SageIRInterface::getThisExpSymHandle(SgNode *node)
{
#if 1

#if 0

  SgMemberFunctionDeclaration *memberFunctionDeclaration = NULL;

  switch(node->variantT()) {

  case V_SgMemberFunctionDeclaration:
    {
      memberFunctionDeclaration = isSgMemberFunctionDeclaration(node);

      break;
    }

  case V_SgThisExp:
    {
      SgThisExp *thisExp = isSgThisExp(node);
      ROSE_ASSERT(thisExp != NULL);

      SgFunctionDefinition *functionDefinition = 
	getEnclosingMethod(thisExp);
      ROSE_ASSERT(functionDefinition != NULL);

      memberFunctionDeclaration = 
	isSgMemberFunctionDeclaration(functionDefinition->get_declaration());

      break;
    }

  default:
    {
      cerr << "Do not know how to extract a SgFunctionParameterList from" << endl;
      cerr << "a " << node->sage_class_name();
      ROSE_ABORT();
      break;
    }

  }

  ROSE_ASSERT(memberFunctionDeclaration != NULL);

  SgCtorInitializerList *initializerList =
    memberFunctionDeclaration->get_CtorInitializerList();
  ROSE_ASSERT(initializerList != NULL);

  OA::SymHandle symHandle = getNodeNumber(initializerList);      

#else /* 1 */


  SgFunctionDeclaration *functionDeclaration = NULL;

  switch(node->variantT()) {

  case V_SgMemberFunctionDeclaration:
    {
      functionDeclaration = isSgFunctionDeclaration(node);

      break;
    }

  case V_SgThisExp:
    {
      SgThisExp *thisExp = isSgThisExp(node);
      ROSE_ASSERT(thisExp != NULL);

      SgFunctionDefinition *functionDefinition = 
	getEnclosingMethod(thisExp);
      ROSE_ASSERT(functionDefinition != NULL);

      functionDeclaration = functionDefinition->get_declaration();

      break;
    }

  default:
    {
      cerr << "Do not know how to extract a SgFunctionParameterList from" << endl;
      cerr << "a " << node->sage_class_name();
      ROSE_ABORT();
      break;
    }

  }

  ROSE_ASSERT(functionDeclaration != NULL);

  SgFunctionParameterList *paramList = 
    functionDeclaration->get_parameterList();
  ROSE_ASSERT(paramList != NULL);

  OA::SymHandle symHandle = getNodeNumber(paramList);      

#endif

#else
#if 0
  SgClassSymbol *classSymbol = NULL;

  switch(node->variantT()) {

  case V_SgMemberFunctionDeclaration:
    {
      SgMemberFunctionDeclaration *memberFunctionDeclaration =
	isSgMemberFunctionDeclaration(node);
      ROSE_ASSERT(memberFunctionDeclaration != NULL);

#if 0
      SgClassDefinition *classDefinition = 
	memberFunctionDeclaration->get_scope();
      ROSE_ASSERT(classDefinition != NULL);

      // Get the name of the class.
      SgName className = classDefinition->get_qualified_name();
      
      // Get the symbol table associated with this scope.
      SgSymbolTable *symbolTable = classDefinition->get_symbol_table();
      ROSE_ASSERT(symbolTable != NULL);
#else
      SgClassDefinition *classDefinition = 
	memberFunctionDeclaration->get_scope();
      ROSE_ASSERT(classDefinition != NULL);

      // Get the name of the class.
      SgName className = classDefinition->get_qualified_name();

      // Find the scope in which this class was declared.
      SgNode *parent = memberFunctionDeclaration->get_parent();
      ROSE_ASSERT(parent != NULL);
      
      SgScopeStatement *scope = isSgScopeStatement(parent);
      ROSE_ASSERT(scope != NULL);

      // Get the symbol table associated with this scope.
      SgSymbolTable *symbolTable = scope->get_symbol_table();
      ROSE_ASSERT(symbolTable != NULL);
#endif
      // Look up the name in the symbol table to get a handle
      // on the class symbol.
      classSymbol = symbolTable->findclass(className);
      ROSE_ASSERT(classSymbol != NULL);

      break;
    }

  case V_SgThisExp:
    {
      SgThisExp *thisExp = isSgThisExp(node);
      ROSE_ASSERT(thisExp != NULL);

      classSymbol = thisExp->get_class_symbol();
      ROSE_ASSERT(classSymbol != NULL);

      break;
    }

  default:
    {
      cerr << "Do not know how to extract a SgClassSymbol from" << endl;
      cerr << "a " << node->sage_class_name();
      ROSE_ABORT();
      break;
    }

  }

  OA::SymHandle symHandle = getNodeNumber(classSymbol);      
#else
  SgFunctionDefinition *functionDefinition = NULL;

  switch(node->variantT()) {

  case V_SgMemberFunctionDeclaration:
    {
      SgMemberFunctionDeclaration *memberFunctionDeclaration =
	isSgMemberFunctionDeclaration(node);
      ROSE_ASSERT(memberFunctionDeclaration != NULL);

      functionDefinition = memberFunctionDeclaration->get_definition();
      ROSE_ASSERT(functionDefinition != NULL);

      break;
    }

  case V_SgThisExp:
    {
      SgThisExp *thisExp = isSgThisExp(node);
      ROSE_ASSERT(thisExp != NULL);

      functionDefinition = getEnclosingMethod(thisExp);
      ROSE_ASSERT(functionDefinition != NULL);

      break;
    }

  default:
    {
      cerr << "Do not know how to extract a SgFunctionDefinition from" << endl;
      cerr << "a " << node->sage_class_name();
      ROSE_ABORT();
      break;
    }

  }

  OA::SymHandle symHandle = getNodeNumber(functionDefinition);      
#endif
#endif

  return symHandle;
}

/** \brief Return a SymHandle representing the return slot of 
 *         function.
 *  \param functionDeclaration  A SgFunctionDeclaration representing
 *                              a function declaration in the AST.
 *  \return A SymHandle to be used to represent the return slot
 *          of the function.
 *
 *  The SymHandle returned here should be used in the MRE on the
 *  lhs of the pointer assignment for returns, as in:
 *  int *returnPtr() { return somePtr; }
 *  and on the rhs of a pointer assignment assigning an addr
 *  returning by a function to a variable, as in:
 *  int *a = returnPtr();
 *
 *  Use a pointer to the function's parameter list AST node to
 *  represent the return slot.
 */
OA::SymHandle 
SageIRInterface::getFunctionReturnSlotSymbol(SgFunctionDeclaration *functionDeclaration)
{
  ROSE_ASSERT(functionDeclaration != NULL);

  SgFunctionParameterList *paramList = 
    functionDeclaration->get_parameterList();
  ROSE_ASSERT(paramList != NULL);

  OA::SymHandle symHandle = getNodeNumber(paramList);      
  return symHandle;
}

#if 0
// Given a SgFunctionCallExp representing a method invocation,
// return a MemRefHandle to the object upon which the method
// was invoked.  The returned MemRefHandle represents the
// invoking object when we fold it into the actual arguments
// of a method invocation.  Therefore, we only want the
// pseudo-MemRefExprs behind this MemRefHandle to be visible
// in the limited context of a call to getCallsiteParams.
// This would not be the case if the MemRefHandle represented
// some other MemRefExprs as well and would be generated
// (e.g., by findAllMemRefs) for that reason.  Therefore,
// return something outlandish that will never be considered
// a MemRefExpr, but that is nonetheless specific and accessible
// from this method invocation.  We'll return the methods
// argument list.
OA::MemRefHandle 
SageIRInterface::getDispatchingObjectMemRefHandle(SgFunctionCallExp *func)
{
  OA::MemRefHandle memRefHandle;

  ROSE_ASSERT(func != NULL);

  SgExpression *expression = functionCall->get_function();
  ROSE_ASSERT(expression != NULL);

  ROSE_ASSERT( isSgDotExp(expression) || isSgArrowExp(expression) );

  // Get the list of actual arguments from the function call.
  SgExprListExp* exprListExp = functionCallExp->get_args();  
  ROSE_ASSERT (exprListExp != NULL);  

  memRefHandle = getNodeNumber(exprListExp);

  return memRefHandle;
}
#endif

SgNode *
SageIRInterface::lookThroughCastExpAndAssignInitializer(SgNode *node)
{
  ROSE_ASSERT(node != NULL);

  SgNode *ret = NULL;

  switch(node->variantT()) {

  case V_SgCastExp:
    {
      SgCastExp *castExp = isSgCastExp(node);
      ROSE_ASSERT(castExp != NULL);

      // Do not look past the cast if the operand is a malloc.
      // In that case, we include the cast as part of the MemRefExpr.

      SgNode *operand = castExp->get_operand();
      SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(operand);

      // This cast is attached to a malloc.
      if ( ( functionCallExp != NULL ) && ( isMalloc(functionCallExp) ) ) {
	ret = node;
      } else {
	ret = lookThroughCastExpAndAssignInitializer(operand);
      }

      break;
    }

  case V_SgAssignInitializer:
    {
      SgAssignInitializer *assignInitializer = isSgAssignInitializer(node);
      ROSE_ASSERT(assignInitializer != NULL);

      ret = lookThroughCastExpAndAssignInitializer(assignInitializer->get_operand_i());

      
      break;
    }

  default:
    {
      ret = node;
      break;
    }

  }
  return ret;

}

#if 1
OA::ProcHandle SageIRInterface::getProcHandle(SgFunctionDefinition *node)
{
  OA::ProcHandle procHandle = getNodeNumber(node);
  return procHandle;
}
#else
OA::ProcHandle SageIRInterface::getProcHandle(SgFunctionDeclaration *node)
{
  OA::ProcHandle procHandle = getNodeNumber(node);
  return procHandle;
}
#endif

bool SageIRInterface::isFieldAccess(OA::OA_ptr<OA::MemRefExpr> mre)
{
  bool isFA = false;

  if ( mre->isaRefOp() ) {
		
    OA::OA_ptr<OA::RefOp> refOp = mre.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));
    
    if ( refOp->isaSubSetRef() ) {
      
      OA::OA_ptr<OA::SubSetRef> subSetRef = 
	refOp.convert<OA::SubSetRef>();
      ROSE_ASSERT(!subSetRef.ptrEqual(0));
      
      if ( subSetRef->isaFieldAccess() ) {
	isFA = true;
      }
      
    }
    
  }

  return isFA;
}

// getAttribute is a wrapper around the ROSE attribute mechanism
// which returns a node's attribute.  This was defined for 
// compatibility between older versions of ROSE in which
// attributes were an instance variable of a SgNode, and
// newer versions in which attributes are accessible via a method call.
AstAttributeMechanism &SageIRInterface::getAttribute(SgNode *n)
{
  ROSE_ASSERT(n);
#ifdef UNRELEASED_ROSE
  if ( n->get_attribute() == NULL ) {
    AstAttributeMechanism *attr = new AstAttributeMechanism();
    ROSE_ASSERT(attr != NULL);
    n->set_attribute(attr);
  }
  return n->attribute();
#else
  return n->attribute;
#endif
}

bool SageIRInterface::returnsAddress(SgFunctionCallExp *functionCallExp)
{
  ROSE_ASSERT(functionCallExp != NULL);

  bool returnsAddr = false;

  SgType *type = functionCallExp->get_type();
  //  SgType *type = functionCallExp->get_return_type();
  ROSE_ASSERT(type != NULL);
  
  if ( isSgReferenceType(type) || isSgPointerType(type) ) {
    returnsAddr = true;
  }
  
  return returnsAddr;
	
}

bool 
SageIRInterface::createsBaseType(SgConstructorInitializer *ctorInitializer) const
{
  bool ret = false;

  SgMemberFunctionDeclaration *memberFunctionDeclaration =
    ctorInitializer->get_declaration();

  if ( memberFunctionDeclaration == NULL ) {
    ret = true;
  }

  return ret;
}
