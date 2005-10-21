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

SageIRStmtIterator::SageIRStmtIterator(SgNode* node, SageIRInterface * in)
{
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
  SgConditionalExp *conditionalExp = NULL;
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

OA::OA_ptr<OA::IRStmtIterator> SageIRInterface::getStmtIterator(OA::ProcHandle h)
{
   OA::OA_ptr<OA::IRStmtIterator> iter;
    SgFunctionDefinition * fdef=NULL;
    fdef=isSgFunctionDefinition((SgNode *)(getNodePtr(h)));
    if(fdef)
    iter=new SageIRStmtIterator(fdef, this);

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
    abort();
    //if it is a varref rerurn an SgInitializedName
  }
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

  if( ( fdef=isSgFunctionDefinition(pnode) ) != NULL )
  {
    	//SgSymbol * fsymbol=(isSgFunctionRefExp(fcall->get_function()))->get_symbol();
      //sh=(OA::irhandle_t)fsymbol;
    //we will use function declaration, at least for now
    SgFunctionDeclaration * fundecl=fdef->get_declaration();
    //printf("returning func decl from getProcSymHandle\n");
    sh=(OA::irhandle_t)(getNodeNumber(fundecl));
  }
  else
  {
    printf("in SageIRInterface::getSymHandle, expression type not yet implemented\n");
    abort();
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
    SgFunctionDefinition* f = (SgFunctionDefinition*)(getNodePtr(h));
    if (f) {
        SgStatementPtrList & pl=f->get_body()->get_statements();
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
  abort();
	return 0;
}

// Given an unstructured multi-way branch, return the label of the target
// statement at 'targetIndex'. The n targets are indexed [0..n-1]. 
OA::StmtLabel SageIRInterface::getUMultiTargetLabel(OA::StmtHandle h, int)
{
	//for now
  abort();
	return 0;
}

// Given an unstructured multi-way branch, return the label of the optional
// default/catchall target. Return 0 if no default target.
OA::StmtLabel SageIRInterface::getUMultiCatchallLabel(OA::StmtHandle h)
{
	//for now
  abort();
	return 0;
}

OA::ExprHandle 
SageIRInterface::getUMultiCondition (OA::StmtHandle h, int targetIndex)
{
    // for now
  abort();
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



OA::Alias::IRStmtType SageIRInterface::getAliasStmtType(OA::StmtHandle h)
{
  SgStatement * sgstmt=(SgStatement*)getNodePtr(h);
  SgExprStatement *est=isSgExprStatement(sgstmt);
  SgVariableDeclaration *vardecl=isSgVariableDeclaration(sgstmt);
  if((!est) && (!vardecl))
    return OA::Alias::ANY_STMT;
  if(vardecl)
  {
    SgExpression * varinit=NULL; //to do ROSE changed vardecl->get_definition()->get_initializer_expr_root()->get_operand_i();
    if(varinit)
    {
      SgType * init_type=varinit->get_type();
      if(!isSgPointerType(init_type)) //this is not general enough, also need to consider typedef'ed types
                                    //will add later
         return OA::Alias::ANY_STMT;
      else
          return OA::Alias::PTR_ASSIGN_STMT;
    }
    else
      return OA::Alias::ANY_STMT;
  }
  else if(est)
  {
    SgExpression * the_exp=est->get_the_expr();
    SgAssignOp * assgn=isSgAssignOp(the_exp);
    if(!assgn)
      return OA::Alias::ANY_STMT;
    SgExpression * lhs=assgn->get_lhs_operand();
    SgType * lhs_type=lhs->get_type();
    if(!isSgPointerType(lhs_type)) //this is not general enough, also need to consider typedef'ed types
                                  //will add later
       return OA::Alias::ANY_STMT;
    else
        return OA::Alias::PTR_ASSIGN_STMT;
  }
}
OA::OA_ptr<OA::IRSymIterator> SageIRInterface::getVisibleSymIterator(OA::ProcHandle h)
{
  OA::OA_ptr<OA::IRSymIterator> retval;//=NULL;
  abort();
  return retval;
}

OA::OA_ptr<OA::IRStmtIterator> SageIRInterface::getUnnamedDynAllocStmtIterator(OA::ProcHandle h)
{
  OA::OA_ptr<OA::IRStmtIterator> retval;//=NULL;
  abort();
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
  SgFunctionDefinition* funcdef = (SgFunctionDefinition*)(getNodePtr(p));
  bool islocal=true;
  //SgNode * n;
  SgInitializedName * symb=isSgInitializedName((SgNode*)(getNodePtr(s)));
  SgDeclarationStatement * decl=NULL;
  SgNode * paren1=NULL;
  SgNode * paren2=NULL;
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
      else
        printf("paren1 is %s\n", paren1->sage_class_name());
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
  //printf("node is %s\n", astNode->sage_class_name());
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
  SgVariableDeclaration * vardecl=isSgVariableDeclaration(astNode);
  SgFunctionParameterList * fparam;
  /*
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
    //printf("created nodeArray of size %i\n", nodeArrayPtr->size());
  }
}

int SageIRInterface::getNodeNumber(SgNode * n) //can be zero
{
  if(n==NULL)
  {
    printf("NULL ptr in getNodeNumber\n");
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
      abort();
      break;
    }

  }

  return ret;
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
	// This seems to occur when using varargs.
	// Let's ensure it only occurs then.
	SgType *type = initName->get_type();
	ROSE_ASSERT(type != NULL);
	ROSE_ASSERT(isSgTypeEllipse(type) != NULL);
	ret = "NULL";
      } else {
	ret = nm.str();
      }

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

//! If this is a PTR_ASSIGN_STMT then return an iterator over MemRefHandle
//! pairs where there is a source and target such that target
//! FIXME: returning a bogus iterator with no pairs
OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator>
SageIRInterface::getPtrAssignStmtPairIterator(OA::StmtHandle stmt)
{   
  abort();
  OA::OA_ptr<OA::Alias::PtrAssignPairStmtIterator> retval;
  retval = new SgPtrAssignPairStmtIterator;
  return retval;
}

// returns true if given symbol is a pass by reference parameter 
// NB:  I'm assuming (conservatively) that "pass by reference"
//      refers both to reference parameters (i.e., foo(int &a))
//      and (pass by value) pointer parameters (i.e., foo(int *a)).
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
  // SymHandle parameter may also be a SgClassDefinition,
  // in which case the function invoked is actually a method
  // and the argument is intended to represent the object
  // upon which it was invoked.

#if 1
  switch(node->variantT()) {

  case V_SgClassDefinition:
    {
      // Unfortunately, without more context we can not determine
      // if 'this' is modified by this method invocation.
      // It would be nice to know, e.g., whether the method is const.
      // To determine that we would need to be passed a procHandle
      // to the invoked method.
      // For now, simply be conservative.

      retVal = true;

      break;
    }

  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL); 

      const SgType *type = initName->get_type(); 
      ROSE_ASSERT(type != NULL); 

      retVal = ( isSgPointerType(type) || isSgReferenceType(type) );

      break;
    }

  default:
    {

      cerr << "SageIRInterface::isRefParam(OA::SymHandle h) didn't know" << endl;
      cerr << "how to handle a SymHandle of type: " << node->sage_class_name() << endl;
      abort();

      break;
    }

  }
#else
  switch(node->variantT()) {

  case V_SgVarRefExp:
    {
      SgVarRefExp *varRefExp = isSgVarRefExp(node); 
      ROSE_ASSERT(varRefExp != NULL);

      SgVariableSymbol *symbol = varRefExp->get_symbol(); 
      ROSE_ASSERT(symbol != NULL); 

      SgInitializedName *initName = symbol->get_declaration(); 
      ROSE_ASSERT(initName != NULL); 

      const SgType *type = initName->get_type(); 
      ROSE_ASSERT(type != NULL); 

      retVal = ( isSgPointerType(type) || isSgReferenceType(type) );

      break;
    }

  case V_SgVariableSymbol:
    {
      SgVariableSymbol *symbol = isSgVariableSymbol(node);
      ROSE_ASSERT(symbol != NULL); 

      SgInitializedName *initName = symbol->get_declaration(); 
      ROSE_ASSERT(initName != NULL); 

      const SgType *type = initName->get_type(); 
      ROSE_ASSERT(type != NULL); 

      retVal = ( isSgPointerType(type) || isSgReferenceType(type) );

      break;
    }

  case V_SgInitializedName:
    {
      SgInitializedName *initName = isSgInitializedName(node);
      ROSE_ASSERT(initName != NULL); 

      const SgType *type = initName->get_type(); 
      ROSE_ASSERT(type != NULL); 

      retVal = ( isSgPointerType(type) || isSgReferenceType(type) );

      break;
    }

  }
#endif
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
      abort();
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
    abort();

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

      lastArgIsVarArg = ( isSgTypeEllipse(type) != NULL );
    }

    if (!lastArgIsVarArg) {

      cerr << "Failed to find parameter #" << n;
      cerr << " in a function with only " << m - 1 << " parameters" << endl;
      abort();

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

    } else if ( isMemRefNode(astNode) ) {

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
      abort();
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

    OA::ExprHandle exprHandle = getNodeNumber(function);
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

  OA::OA_ptr<OA::IRCallsiteIterator> retIter;
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

  // Get the list of formal parameters for the function.
  SgFunctionParameterList *parameterList = 
    functionDeclaration->get_parameterList(); 
  ROSE_ASSERT(parameterList != NULL); 

  // Create a list to hold the handles to the formal parameters.
  OA::OA_ptr<std::list<OA::SymHandle> > symHandleList;
  symHandleList = new std::list<OA::SymHandle>;

  // If this is a method, fold its class (i.e., this) into 
  // the argument list as the first argument.
  // OpenAnalysis doesn't know about methods and wouldn't be otherwise
  // able to account, e.g., for MODs and USEs of the 'this' object.
  // NB:  This is truly awful.  I'm stuffing a SgClassDefinition*
  //      in where a SymHandle is expected.  Other code needs to
  //      be wary that when it a SymHandle is converted to a
  //      SgClassDefinition, it should be interpreted as a this pointer.
  //      I would love to put a SgThisExp here instead.  Unfortunately,
  //      we aren't guaranteed to be able to find one within the
  //      method (e.g., if it does invoke any other methods or
  //      access any member variables).
  SgMemberFunctionDeclaration *memberFunctionDeclaration =
    isSgMemberFunctionDeclaration(functionDeclaration);
  if ( memberFunctionDeclaration != NULL ) {

    SgClassDefinition *classDefinition = 
    isSgClassDefinition(
      memberFunctionDeclaration->get_scope());
    ROSE_ASSERT(classDefinition != NULL);

    OA::SymHandle symHandle = getNodeNumber(classDefinition);
    symHandleList->push_back(symHandle);

  }

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

#ifdef BWHITE_VERSION
  if ( memRefExp->isUseDef() ) refTypeStr = "USEDEF";
  else if ( memRefExp->isDefUse() ) refTypeStr = "DEFUSE";
#else
  if ( memRefExp->isUse() && memRefExp->isDef() ) refTypeStr = "USEDEF";
#endif
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
