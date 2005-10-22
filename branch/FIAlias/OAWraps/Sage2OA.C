#include "Sage2OA.h"
#include "SageOACallGraph.h"
#include "MemSage2OA.h"
 
//########################################################
// Iterators
//########################################################

//std::vector<SgNode*> SageIRInterface::nodeArray;
 
 
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
    isSgMemberFunctionDeclaration(parent);
  if ( memberFunctionDeclaration != NULL ) {
    SgCtorInitializerList *initializerList =
      memberFunctionDeclaration->get_CtorInitializerList();
    if ( initializerList != NULL )
      all_stmts.push_back(initializerList);
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
    lst.push_back(stmt);
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
  iter=new SageIRCallsiteIterator(stmt, this);
  
  return iter;
}

SgFunctionDeclaration * 
getFunctionDeclaration(SgFunctionCallExp *functionCall) 
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
    //printf("returning func decl from getProcSymHandle\n");
    sh=(OA::irhandle_t)(getNodeNumber(fundecl));
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
    st->attribute.add("OANumber", new SageNodeNumAttr(currentNumber));
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

  switch(node->variantT()) {
 
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
      
      if ( varDeclHasPtrAssign(varDecl, collectPtrAssigns, NULL) )
	stmtType = OA::Alias::PTR_ASSIGN_STMT;

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

//need to now if a location is "local" with respect to a procedure.  
//local here means visible only in that procedure.  
//Member variables are not local to any one method in a class.

OA::OA_ptr<OA::Location::Location> 
SageIRInterface::getLocation(OA::ProcHandle p, OA::SymHandle s)
{
  if((((int)s)==0) || (s.hval()==0))
  {
    //not sure what to do
      OA::OA_ptr<OA::Location> loc;
      loc=new OA::UnknownLoc();
      return loc;
  }
  bool islocal=true;
  //SgNode * n;
  SgInitializedName * symb=isSgInitializedName((SgNode*)(getNodePtr(s)));
  SgDeclarationStatement * decl=NULL;
  SgNode * paren1=NULL;

  if(symb)
  {
    decl=symb->get_declaration();
    if(decl)
    {
      paren1=decl->get_parent();
      if(isSgGlobal(paren1))
      {
        islocal=false;
      }
      /* this is all wrong wiil use attributes to store function declaration for each init name (unless it is global)
      else if(isSgBasicBlock(paren1))
      {
        paren2=paren1->get_parent();
        if(paren2!=funcdef)
        {
          printf("TO DO: paren1 is BasicBlock but paren2 is not the right function def\n");
          islocal=false;
        }
      }
      */
      else {
	//        printf("paren1 is %s\n", paren1->sage_class_name());
      }
    }
  }
  //if(symb->get_declaration() && (n=symb->get_declaration()->get_parent()) && isSgGlobal(n))
  //{
  //  islocal=false;
  //}
  //proc handle is SgFunctionDefinition
  //are parameters local? I hope so
  //local vars are definitely local
  
  OA::OA_ptr<OA::Location> loc;
  loc=new OA::NamedLoc(s, islocal);
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
  if(exp || st || in)
  {
    if(in)
    { 
      //this does not work for math.h 
      //apparently some SgInitializedNames have no p_char in SgName
      //printf("assigning node number %i to SgInitializedName %s\n", currentNumber+1,
      //                                          in->get_name().getString().c_str());
    }
    astNode->attribute.add("OANumber", new SageNodeNumAttr(currentNumber));
    ir->nodeArrayPtr->push_back(astNode);
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

void SageIRInterface::createNodeArray(SgNode * root)
{
  if(nodeArrayPtr->size()==0)
  {
    NumberTraversal t(this);
    t.traverse(root, preorder);
    //    printf("created nodeArray of size %i\n", nodeArrayPtr->size());
  }
}

int SageIRInterface::getNodeNumber(SgNode * n) //can be zero
{
  if(n==NULL)
  {
    //    cerr << "NULL ptr in getNodeNumber" << endl;
    return 0;
  }
  else if(persistent_handles)
  {
    if(n->attribute.exists("OANumber"))
    {
      SageNodeNumAttr * attr=dynamic_cast<SageNodeNumAttr * >(n->attribute["OANumber"]);
      return (attr->number)+1;
    }
    else
      return 0;
  }
  else
    return (int)n;
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
  char val[20];
  if(h.hval()==0)
    strdump="NULL mem ref ";
  else
    {
      sprintf(val, " %i", (int)h.hval());
      SgNode *node = getNodePtr(h);
      ROSE_ASSERT(node != NULL);
      if(isSgExpression(node))
        strdump=isSgExpression(node)->unparseToString();
      else if(isSgInitializedName(node))
        strdump=isSgInitializedName(node)->get_name().getString();
    }
  // Let's not print the memory address.  This creates problems
  // comparing results of regression tests.
  //    strdump+=val;
  return strdump;
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

      nm = fd->get_name();
      //      nm = fd->get_qualified_name();
      ret = nm.str();

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
	SgFunctionDefinition *functionDefinition = 
	  getEnclosingMethod(initName);

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

  case V_SgFunctionDefinition:
    {
      // If we see a SgFunctionDefinition where we expected a symbol,
      // it means that the symbol represents a 'this' pointer.
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

      ret = string("this: ") + nm.str();

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

// Create lhs/rhs pairs corresponding to pointer assignments
// within the tree rooted at an assign.
SgExpression *
SgPtrAssignPairStmtIterator::createPtrAssignPairsFromAssignment(SgNode *node)
{
  if ( node == NULL ) return NULL;

  SgExpression *lhs = NULL;
  SgExpression *rhs = NULL;

  switch(node->variantT()) {
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
	  rhs = createPtrAssignPairsFromAssignment(rhs);
	  ROSE_ASSERT(rhs != NULL);

	  break;
	}
      default:
	{
	  // The lhs is a pointer type and the rhs is not an assign.
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
	      
	      mMemRefList.push_back(pair<OA::OA_ptr<OA::MemRefExpr>, OA::OA_ptr<OA::MemRefExpr> >(lhsMre, rhsMre));
	      
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

  return lhs;
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

// Create iterator consisting of pairs of procedure formal SymHandles
// and procedure call actual MemRefHandles.  A pair is created
// only for pointer or reference formals.
void SgParamBindPtrAssignIterator::create(OA::ExprHandle call)
{
  SgNode *node = mIR->getNodePtr(call);
  ROSE_ASSERT(node != NULL);
  
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

  OA::OA_ptr<OA::IRFormalParamIterator> formalsIter;
  formalsIter = NULL;
  SgFunctionDeclaration *functionDeclaration = 
    getFunctionDeclaration(functionCallExp); 
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

  OA::OA_ptr<OA::IRCallsiteParamIterator> actualsIter = 
    mIR->getCallsiteParams(call);

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

    }

    if ( treatAsPointerParam ) {

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

	function = functionCallExp->get_function();
	ROSE_ASSERT(function != NULL);
	
	if ( isSgDotExp(function) || isSgArrowExp(function) ) 
	  handleThisExp = true;

      }

      OA::MemRefHandle actualMemRefHandle = (OA::MemRefHandle)0;

      bool isArrowExp = false;
      if ( handleThisExp == true ) {

	SgNode *lhs = NULL;

	SgArrowExp *arrowExp = isSgArrowExp(function);

	if ( arrowExp != NULL ) {

	  lhs = arrowExp->get_lhs_operand();
	  ROSE_ASSERT(lhs != NULL);

	  lhs = mIR->lookThroughCastExpAndAssignInitializer(lhs);
	  //	  ROSE_ASSERT(mIR->isMemRefNode(lhs));

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
	  //	  ROSE_ASSERT(mIR->isMemRefNode(lhs));

	}

	if ( mIR->isMemRefNode(lhs) )
	  actualMemRefHandle = mIR->getNodeNumber(lhs);

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
	  
	  
	  mPairList.push_back(pair<int, OA::OA_ptr<OA::MemRefExpr> >(paramNum, actualMre));
	  
	}

      }

    }

    if ( !formalsIter.ptrEqual(0) )
      (*formalsIter)++;
    paramNum++;

  }

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
      
    }
    
  }
  
  return true;
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

// Create iterator consisting of lhs/rhs pairs from pointer
// assignments in stmt.
void SgPtrAssignPairStmtIterator::create(OA::StmtHandle stmt)
{
  if ( mIR->getAliasStmtType(stmt) != OA::Alias::PTR_ASSIGN_STMT )
    return;

  SgNode *node = mIR->getNodePtr(stmt);
  ROSE_ASSERT(node != NULL);

  // We expect this to be either a SgVariableDeclaration of an assign.
  switch(node->variantT()) {

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
//! whose operand is a SgPointerDerefExp.  i.e., returns true
//! if function represents a->method() or (*a).method().
bool SageIRInterface::isArrowExp(SgExpression *function)
{
  if ( isSgArrowExp(function) )
    return true;

  if ( isSgDotExp(function) ) {

    SgBinaryOp *dotOrArrow = isSgBinaryOp(function);
    ROSE_ASSERT(dotOrArrow != NULL);

    SgNode *lhs = dotOrArrow->get_lhs_operand();
    ROSE_ASSERT(lhs != NULL);

    SgDotExp *dotExp = isSgDotExp(function);
    if ( dotExp != NULL ) {
      
      SgPointerDerefExp *pointerDerefExp =
	isSgPointerDerefExp(lhs);

      if ( pointerDerefExp != NULL ) {
	return true;
      }

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

  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
  ROSE_ASSERT(functionCallExp != NULL);

  SgExpression *function = functionCallExp->get_function();
  ROSE_ASSERT(function != NULL);

  if ( isArrowExp(function) ) {

    // This is a method invocation via an arrow exp.
    // We should return a Deref here.  Call the general
    // MemRefExpr routines to create it.

    SgStatement *stmt = getEnclosingStatement(functionCallExp);
    ROSE_ASSERT(stmt != NULL);
    
    OA::StmtHandle stmtHandle = getNodeNumber(stmt);
    ROSE_ASSERT(stmtHandle != (OA::StmtHandle)0);
    
    // Call getMemRefIterator to initialize the sMemref2mreSetMap
    // data structure-- i.e., to ensure that we have a mapping between
    // the MemRefHandle corresponding to this method invocation
    // and its MemRefExpr.
    getMemRefIterator(stmtHandle);

    // We expect the MemRefHandle to be associated with the
    // arrow or dot expression.
    ROSE_ASSERT( isMemRefNode(function) );
    OA::MemRefHandle memRefHandle = getNodeNumber(function);

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

    // ... and it had better be a Derf.
    ROSE_ASSERT(mre->isaRefOp());

    OA::OA_ptr<OA::RefOp> refOp = mre.convert<OA::RefOp>();
    ROSE_ASSERT(!refOp.ptrEqual(0));

    ROSE_ASSERT(refOp->isaDeref());

  } else {

    // This is either a function call or a dot expression.
    // In either case, we know exactly which function is envoked.
    // There is no possibility of a virtual method invocation:
    // we handle pointers above, reference (which would otherwise
    // be manifest as dot expressions) are converted to pointers
    // by SageIRInterface, and non-pointer, non-reference method
    // invocations on objects do not involve virtual dispatch.

    // Create a NamedRef mre.  We expect the symbol embedded
    // in that mre will be a ProcHandle-- i.e., a
    // SgFunctionDeclaration.

    SgFunctionDeclaration *functionDeclaration = 
      getFunctionDeclaration(functionCallExp); 
    ROSE_ASSERT(functionDeclaration != NULL);

    OA::MemRefExpr::MemRefType memRefType = OA::MemRefExpr::DEF;

#if 0
    SgFunctionDefinition *functionDefinition =
      functionDeclaration->get_definition();
    ROSE_ASSERT(functionDefinition != NULL);
    OA::SymHandle symHandle = getNodeNumber(functionDefinition);
#else
    OA::SymHandle symHandle = getNodeNumber(functionDeclaration);
#endif

    bool addressTaken = false;
    bool fullAccuracy = true;

    mre = new OA::NamedRef(addressTaken, fullAccuracy,
			   memRefType, symHandle);
    ROSE_ASSERT(!mre.ptrEqual(0));

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
SageIRInterface::getParamBindPtrAssignIterator(OA::ExprHandle call)
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

  // A callsite is represented in Sage as a SgFunctionCallExp.
  SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
  ROSE_ASSERT(functionCallExp != NULL);

  // Get the list of actual arguments from the function call.
  SgExprListExp* exprListExp = functionCallExp->get_args();  
  ROSE_ASSERT (exprListExp != NULL);  
   
  SgExpressionPtrList & actualArgs =  
    exprListExp->get_expressions();  

  // Create a list to hold the handles to the actual arguments.
  OA::OA_ptr<std::list<OA::ExprHandle> > exprHandleList;
  exprHandleList = new std::list<OA::ExprHandle>;

  // If this is a method call, fold the object upon which the
  // method is invoked into the argument list as the first argument.
  // OpenAnalysis doesn't know about methods and wouldn't be otherwise
  // able to account, e.g., for MODs and USEs of the 'this' object.
  SgExpression *function = functionCallExp->get_function();
  ROSE_ASSERT(function != NULL);

  if ( isSgDotExp(function) || isSgArrowExp(function) ) {
#if 0
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

    OA::ExprHandle exprHandle = getNodeNumber(lhs);
    lhs = lookThroughCastExpAndAssignInitializer(lhs);
    ROSE_ASSERT( isMemRefNode(lhs) );
#else
    OA::ExprHandle exprHandle = getNodeNumber(function);
#endif
    exprHandleList->push_back(exprHandle);

  }

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
  
//! get iterator over formal parameters in a function declaration 
OA::OA_ptr<OA::IRFormalParamIterator> 
SageIRInterface::getFormalParamIterator(OA::SymHandle h)
{
  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);

  // SymHandle must be a function declaration in this context.
  // A function declaration is represented in Sage as a SgFunctionDeclaration.
  SgFunctionDeclaration *functionDeclaration = isSgFunctionDeclaration(node);
  ROSE_ASSERT(functionDeclaration != NULL);

  // Create a list to hold the handles to the formal parameters.
  OA::OA_ptr<std::list<OA::SymHandle> > symHandleList;
  symHandleList = new std::list<OA::SymHandle>;

  // If this is a method, fold its class (i.e., this) into 
  // the argument list as the first argument.
  // OpenAnalysis doesn't know about methods and wouldn't be otherwise
  // able to account, e.g., for MODs and USEs of the 'this' object.
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
#endif
  SgMemberFunctionDeclaration *memberFunctionDeclaration =
    isSgMemberFunctionDeclaration(functionDeclaration);
  if ( memberFunctionDeclaration != NULL ) {
    
#if 0
    SgClassDefinition *classDefinition = 
      memberFunctionDeclaration->get_scope();
    ROSE_ASSERT(classDefinition != NULL);
    
    OA::SymHandle symHandle = getNodeNumber(classDefinition);
#else
    OA::SymHandle symHandle = getThisExpSymHandle(memberFunctionDeclaration);
#endif
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

// Given a SgNode return a symbol handle to represent the
// corresponding SgThisExp.  This symbol handle references
// a SgClassSymbol.
OA::SymHandle SageIRInterface::getThisExpSymHandle(SgNode *node)
{
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

      ret = lookThroughCastExpAndAssignInitializer(castExp->get_operand_i());

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
