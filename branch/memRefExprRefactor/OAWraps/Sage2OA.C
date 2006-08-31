#include "SageOACallGraph.h"
#include "MemSage2OA.h"
#include "common.h"

using namespace std;

static bool debug = false;

//########################################################
// Iterators
//########################################################

//std::vector<SgNode*> SageIRInterface::nodeArray;

//-----------------------------------------------------------------------------
// Constructor.
//-----------------------------------------------------------------------------
SageIRInterface::SageIRInterface(SgNode *root, 
                                 std::vector<SgNode*> *na, 
                                 bool use_persistent_handles,
                                 bool useVtableOpt)
  : nodeArrayPtr(na), wholeProject(root), 
    persistent_handles(use_persistent_handles),
    mUseVtableOpt(useVtableOpt)
{ 
  if ( !mUseVtableOpt ) {
    std::cout << "Using per-method virtual table model" << std::endl;
  }
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
  
    SgFunctionDefinition *functionDefinition = isSgFunctionDefinition(n);
    ROSE_ASSERT(functionDefinition != NULL);

    SgFunctionDeclaration *functionDeclaration =
      functionDefinition->get_declaration();
    ROSE_ASSERT(functionDeclaration != NULL);

    SgFunctionDeclaration *key = 
      isSgFunctionDeclaration(functionDeclaration->get_firstNondefiningDeclaration());
    if ( key == NULL ) key = functionDeclaration;
    mFunctions[key] = functionDeclaration;
  }

} 

SageIRInterface::~SageIRInterface()
{
#if 0
  // Delete any non-AST nodes that SageIRInterface has created.
  for (int i = 0; i < SageIRInterface::sAllocatedNodes.size(); ++i) {
    SgNode *node = SageIRInterface::sAllocatedNodes[i];
    delete node;
  }

  for (int i = mNonAstNodes.size() - 1; i >= 0; --i) {
    SgNode *node = mNonAstNodes[i];
    mNonAstNodes.pop_back();
    delete node;
  } 
#endif
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


/** \brief Add any non-AST Sage nodes/statement that have been
 *         appended after node.
 *  \param  node  A Sage node representing a statement in the AST.
 *  \param  lst   A list of pointers to Sage statemenets.  On output,
 *                contains any non-AST nodes appended to node.
 *  
 *  Non-AST nodes are appened to AST nodes to model object-oriented
 *  semantics in an imperative manner.  e.g., implicit invocations of
 *  default constructors is made explicitly by creating non-AST nodes
 *  to model the calls.
 */
static void appendNonASTStmts(SgNode *node, SgStatementPtrList &lst)
{
#if 0
  if ( SageIRInterface::sNonASTStmts.find(node) != 
       SageIRInterface::sNonASTStmts.end() ) {
    std::vector<SgNode *> &appendedStmts =
      SageIRInterface::sNonASTStmts[node];
    for (int i = 0; i < appendedStmts.size(); ++i) {
      SgNode *appendedStmt = appendedStmts[i];
      ROSE_ASSERT(appendedStmt != NULL);

      SgStatement *stmt = isSgStatement(appendedStmt);
      ROSE_ASSERT(stmt != NULL);
      lst.push_back(stmt);
    }
  }
#endif
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
    //    appendNonASTStmts(node, lst);
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
      //      appendNonASTStmts(stmt, lst);
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
OA::OA_ptr<OA::IRCallsiteIterator> 
SageIRInterface::getCallsites(OA::StmtHandle h)
{
  OA::OA_ptr<OA::IRCallsiteIterator> iter;
  SgStatement * stmt=(SgStatement *)getNodePtr(h);
  OA::OA_ptr<SageIRInterface> ir; ir = new SageIRInterface(*this);
  iter=new SageIRCallsiteIterator(stmt, ir);
  
  return iter;
}

OA::SymHandle SageIRInterface::getSymHandle(OA::ExprHandle eh)
{
  OA::SymHandle sh;
  SgExpression * sgexp=(SgExpression*)getNodePtr(eh);
  printf("in SageIRInterface::getSymHandle, expression type not yet implemented\n");
  sh=0;
  ROSE_ABORT();
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
  OA::OA_ptr<SageIRMemRefIterator> mIter;
  mIter = new SageIRMemRefIterator(stmt, *this);
  for ( ; mIter->isValid(); ++(*mIter) ) {
    OA::MemRefHandle memref = mIter->current();

    // loop over memory reference expressions for this memref handle
    set<OA::OA_ptr<OA::MemRefExpr> >::iterator mreIter;
    for (mreIter = mMemref2mreSetMap[memref].begin();
         mreIter != mMemref2mreSetMap[memref].end(); mreIter++ )
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
  OA::OA_ptr<SageIRMemRefIterator> mIter;
  mIter = new SageIRMemRefIterator(stmt, *this);
  for ( ; mIter->isValid(); ++(*mIter) ) {
    OA::MemRefHandle memref = mIter->current();

    // loop over memory reference expressions for this memref handle
    set<OA::OA_ptr<OA::MemRefExpr> >::iterator mreIter;
    for (mreIter = mMemref2mreSetMap[memref].begin();
         mreIter != mMemref2mreSetMap[memref].end(); mreIter++ )
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
OA::OA_ptr<OA::MemRefHandleIterator> 
SageIRInterface::getAllMemRefs(OA::StmtHandle stmt)
{
  OA::OA_ptr<SageIRMemRefIterator> mIter;
  mIter = new SageIRMemRefIterator(stmt,*this);
  return mIter;
}

OA::Alias::IRStmtType SageIRInterface::getAliasStmtType(OA::StmtHandle h)
{ 
  // if haven't already determined the set of ptr assigns for the program
  // then call the initialization procedure
  if (mStmtToPtrPairs.empty() ) {
      initMemRefAndPtrAssignMaps();
  }

  // if there are no pointer pairs for this statement then 
  // it is an ANY_STMT, otherwise it is a PTR_ASSIGN_STMT
  if (mStmtToPtrPairs[h].empty()) {
    return OA::Alias::ANY_STMT;
  } else {
    return OA::Alias::PTR_ASSIGN_STMT;
  }

}


OA::OA_ptr<OA::MemRefExprIterator> 
SageIRInterface::getMemRefExprIterator(OA::MemRefHandle h)
{
  OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr> > > retList;
  retList = new std::list<OA::OA_ptr<OA::MemRefExpr> >;

  // iterate over set of MemRefExpr's associated with
  // the given MemRefHandle and put them in our list
  set<OA::OA_ptr<OA::MemRefExpr> >::iterator mreIter;
  for (mreIter = mMemref2mreSetMap[h].begin();
       mreIter != mMemref2mreSetMap[h].end(); mreIter++ )
    {
      retList->push_back(*mreIter);
    }

  OA::OA_ptr<SageMemRefExprIterator> retval;
  retval = new SageMemRefExprIterator(retList, this);
  return retval;
}

/*! 
   Create a named location based on SymHandle.

   Need to know if a location is "local" with respect to a procedure.  
   local here means visible only in that procedure.  
   Member variables are not local to any one method in a class.
*/
OA::OA_ptr<OA::Location::Location> 
SageIRInterface::getLocation(OA::ProcHandle p, OA::SymHandle s)
{
  OA::OA_ptr<OA::Location> loc;
  loc = NULL;

  /*
      From Michelle:
      (8/24/06 Subject: Re: missing Location for static class variable)

      All member variables are "not local" in OA terminology.  The definition
      of local in OA means that a variable is ONLY visible within that one
      procedure.  That is not the case for member variables since even if they
      are private they are visible in all member methods.
  */


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

      if (debug) 
        cout << "getLocation initName: " << initName->get_name().str() << endl;

      SgDeclarationStatement *declarationStmt = initName->get_declaration();
      ROSE_ASSERT(declarationStmt != NULL);
      
      SgFunctionDefinition *enclosingProc = 
	getEnclosingFunction(declarationStmt);
      
      // For some reason, the declaration stmt of the initialized
      // name for myParent is the SgCtorInitializerList and is
      // not a SgVariableDeclaration!?  BW  8/29/06
      // class SubClass : public Base {
      //  public:
      //   SubClass(Base & parSubClass) : myParent(parSubClass) {}
      //  Base &myParent;
      // };
      if ( ( enclosingProc == NULL ) || 
           isSgCtorInitializerList(declarationStmt) ) {
	// This variable was either declared in the global scope
	// or in a class/struct.  Consider it non-local.
	// Is this correct?  If it is declared within a 
	// class and it is private, should be consider it
	// local and visible only within methods of that class?
	isLocal = false;
      } else {

        SgFunctionDefinition *procDefn = 
          isSgFunctionDefinition(procNode);

	if ( enclosingProc == procDefn ) {
          // This symbol is local to this procedure.
#if 0
        cout << "getLocation initName: " << initName->get_name().str() << endl;
	cout << "enclosing: " << enclosingProc->unparseToString() << endl;
	cout << "procDefn: " << procDefn->unparseToString() << endl;
	cout << "decl: " << declarationStmt->unparseToString() << endl;
#endif
          isLocal = true;
        } else {
          // This symbol is not visible within this procedure, 
          // so return a NULL location.
	  return loc;
	}

      }
      
#if 0

      SgNode *declarationParent = declarationStmt->get_parent();
      ROSE_ASSERT(declarationParent != NULL);

      if ( isSgGlobal(declarationParent) ) {
        // This symbol is global.
        isLocal = false;
      } else {
        
        SgFunctionDefinition *enclosingProc = 
          getEnclosingFunction(declarationStmt);
        
        SgFunctionDefinition *procDefn = 
          isSgFunctionDefinition(procNode);
        ROSE_ASSERT(procDefn != NULL);
        
        if ( enclosingProc == procDefn ) {
          // This symbol is local to this procedure.
          isLocal = true;
        } else {
          // This symbol is not visible within this procedure, 
          // so return a NULL location.
	  if ( enclosingProc)
	    std::cout << "enclosing proc: " << enclosingProc->unparseToString() << std::endl;
	  if (procDefn)
	    std::cout << "procDefn: " << procDefn->unparseToString() << std::endl;
	  std::cout << "init: " << initName->get_name().str() << std::endl;
          return loc;
        }
      } 
#endif
    break;
    }
  case V_SgFunctionDeclaration:
  case V_SgMemberFunctionDeclaration:
    {
      // For now, the solution is simply to call all functions/methods
      // visible.  This isn't true given static functions, private methods,
      // etc.  But it is the conservative approximation that we must
      // make in OA since it only has two scopes.
      isLocal = false;

      SgFunctionDeclaration *functionDeclaration =
        isSgFunctionDeclaration(node);
      ROSE_ASSERT(functionDeclaration != NULL);

      if (debug) 
        cout << "getLocation func: " 
             << functionDeclaration->get_name().str() << endl;

      break;
    }
  case V_SgFunctionParameterList:
    {
      // If we see a SgFunctionParameterList where we expected a symbol,
      // it means that the symbol represents a 'this' pointer.
      SgFunctionParameterList *parameterList = 
        isSgFunctionParameterList(node);
      ROSE_ASSERT(parameterList != NULL);

      // See comment above.  Member variables are not local.
      isLocal = false;

      break;
    }
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
          //      cout << "Visiting for " << functionDefinition->unparseToCompleteString() << endl;
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
    // The pre-defined Sage traversals skip over nodes that
    // we need to be numbered.  Therefore, manually traverse the AST.
    numberASTNodes(root);
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
      case V_SgExpressionRoot:
        {
          SgExpressionRoot *exprRoot = isSgExpressionRoot(parent);
          ROSE_ASSERT(exprRoot != NULL);
          
          SgNode *parent = exprRoot->get_parent();
          ROSE_ASSERT(isSgReturnStmt(parent));

          retstr = parent->unparseToString();
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

std::string SageIRInterface::toStringWithoutScope(const OA::SymHandle h) 
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

      //      nm = fd->get_name();
      //      ret = nm.str();
      ret = mangleFunctionName(fd);

      break;
    }

  case V_SgMemberFunctionDeclaration:
    {
      SgMemberFunctionDeclaration *fd = isSgMemberFunctionDeclaration(node);
      ROSE_ASSERT(fd != NULL);

      //      nm = fd->get_name();
      //      nm = fd->get_qualified_name() + "__" + fd->get_mangled_name();
      //      ret = string("method:") + nm.str();
      ret = mangleFunctionName(fd);

      break;
    }
 
  case V_SgFunctionRefExp:
  case V_SgMemberFunctionRefExp:
    {
      SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(node);
      ROSE_ASSERT(functionRefExp != NULL);
      
      SgFunctionSymbol *functionSymbol = functionRefExp->get_symbol();
      ROSE_ASSERT(functionSymbol != NULL);

      SgFunctionDeclaration *functionDecl = 
        isSgFunctionDeclaration(functionSymbol);
      ROSE_ASSERT(functionDecl != NULL);

      ret = toStringWithoutScope(functionDecl);

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
          getEnclosingFunction(declarationStmt);

        if (functionDefinition == NULL) {

          //      ret = string("global: " ) + nm.str();
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
          
          //      ret = nm.str() + string(" defined in: ") + funcName.str();
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

std::string SageIRInterface::toStringWithoutScope(SgNode *node)
{
  SgName nm;
  std::string ret;

  switch(node->variantT()) {
    
  case V_SgFunctionDeclaration:
    {
      SgFunctionDeclaration *fd = isSgFunctionDeclaration(node);
      ROSE_ASSERT(fd != NULL);

      //      nm = fd->get_name();
      //      ret = nm.str();
      ret = mangleFunctionName(fd);

      break;
    }

  case V_SgMemberFunctionDeclaration:
    {
      SgMemberFunctionDeclaration *fd = isSgMemberFunctionDeclaration(node);
      ROSE_ASSERT(fd != NULL);

      //      nm = fd->get_name();
      //      nm = fd->get_qualified_name() + "__" + fd->get_mangled_name();
      //      ret = string("method:") + nm.str();
      ret = mangleFunctionName(fd);

      break;
    }

  case V_SgFunctionRefExp:
    {
      SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(node);
      ROSE_ASSERT(functionRefExp != NULL);
      
      SgFunctionSymbol *functionSymbol = functionRefExp->get_symbol();
      ROSE_ASSERT(functionSymbol != NULL);

      SgFunctionDeclaration *functionDecl = 
        functionSymbol->get_declaration();
      ROSE_ASSERT(functionDecl != NULL);

      ret = toStringWithoutScope(functionDecl);

      break;
    }

  case V_SgMemberFunctionRefExp:
    {
      SgMemberFunctionRefExp *functionRefExp = isSgMemberFunctionRefExp(node);
      ROSE_ASSERT(functionRefExp != NULL);
      
      SgFunctionSymbol *functionSymbol = functionRefExp->get_symbol();
      ROSE_ASSERT(functionSymbol != NULL);

      SgFunctionDeclaration *functionDecl = 
        functionSymbol->get_declaration();
      ROSE_ASSERT(functionDecl != NULL);

      ret = toStringWithoutScope(functionDecl);

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

      return toStringWithoutScope(initName);

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
          getEnclosingFunction(declarationStmt);

        if (functionDefinition == NULL) {

          //      ret = string("global: " ) + nm.str();
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
          
          //      ret = nm.str() + string(" defined in: ") + funcName.str();
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
          getEnclosingFunction(declarationStmt);

        if (functionDefinition == NULL) {

          //      ret = string("global: " ) + nm.str();
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
          
          //      ret = nm.str() + string(" defined in: ") + funcName.str();
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
// AliasIRInterface
//-------------------------------------------------------------------------

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
void SgParamBindPtrAssignIterator::create(OA::CallHandle call)
{
  
  // loop through the pairs we found in initMemRefsAndPtrAssigns
  std::set<std::pair<int, 
                     OA::OA_ptr<OA::MemRefExpr> > >::iterator pairIter;
  for (pairIter=mIR->mCallToParamPtrPairs[call].begin();
       pairIter!=mIR->mCallToParamPtrPairs[call].end();
       pairIter++)
  {
      mPairList.push_back(*pairIter);
  }
}

// Create iterator consisting of lhs/rhs pairs from pointer
// assignments in stmt.
void SgPtrAssignPairStmtIterator::create(OA::StmtHandle stmt)
{
  if ( mIR->getAliasStmtType(stmt) != OA::Alias::PTR_ASSIGN_STMT )
    return;

  // loop through the pairs we found in initMemRefsAndPtrAssigns
  std::set<std::pair<OA::OA_ptr<OA::MemRefExpr>,
                     OA::OA_ptr<OA::MemRefExpr> > >::iterator pairIter;
  for (pairIter=mIR->mStmtToPtrPairs[stmt].begin();
       pairIter!=mIR->mStmtToPtrPairs[stmt].end();
       pairIter++)
  {
      mMemRefList.push_back(*pairIter);
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


//! Given a procedure call create a memory reference expression 
//! to describe that call.  For example, a normal call is 
//! a NamedRef.  A call involving a function ptr is a Deref. 
OA::OA_ptr<OA::MemRefExpr> SageIRInterface::getCallMemRefExpr(OA::CallHandle h)
{
    return mCallToMRE[h];
}

//! Given the callee symbol returns the callee proc handle 
OA::ProcHandle SageIRInterface::getProcHandle(OA::SymHandle sym)
{
  // sym is the handle within the NamedRef representing a
  // call.  By design (above in getCallMemRefExpr), this
  // is a procHandle.
  SgNode *node = getNodePtr(sym);
  ROSE_ASSERT(node != NULL);

  SgFunctionDeclaration *functionDeclaration = 
    isSgFunctionDeclaration(node);
  ROSE_ASSERT(functionDeclaration != NULL);
  SgFunctionDefinition *functionDefinition =
    functionDeclaration->get_definition();
  return getProcHandle(functionDefinition);
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

 
//-------------------------------------------------------------------------
// SSAIRInterface
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

//! returns true if given symbol is a parameter 
bool SageIRInterface::isParam(OA::SymHandle h)
{   
  SgNode *node = getNodePtr(h);
  SgNode *parent = node->get_parent();
  return isSgFunctionParameterList(parent);
}

// return the formal parameter that an actual parameter is associated with 
OA::SymHandle SageIRInterface::getFormalForActual(OA::ProcHandle caller, 
                                                  OA::CallHandle call, 
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
                isSgVarArgStartOp(astNode)) {

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
 
//-------------------------------------------------------------------------
// ActivityIRInterface
//-------------------------------------------------------------------------
 
//! Given a statement return a list to the pairs of 
//! target MemRefHandle, ExprHandle where
//! target = expr
OA::OA_ptr<OA::ExprStmtPairIterator> 
  SageIRInterface::getExprStmtPairIterator(OA::StmtHandle h)
{
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // FIXME
  // BK: this routine is incomplete.
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  OA::OA_ptr<ExprStmtPairList> exprStmtPairList;
  exprStmtPairList = new ExprStmtPairList;
  
  if (getActivityStmtType(h)==OA::Activity::EXPR_STMT) {
    SgNode *node = getNodePtr(h);
    ROSE_ASSERT(node != NULL);
    
    SgExpression *rhs;
    SgNode *lhs;
    
    bool collectPtrAssigns = false;
    
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
            // A subset of this case is a new expression.
            SgBinaryOp *assignOp = isSgBinaryOp(expression);
            ROSE_ASSERT(assignOp != NULL);
            
            SgExpression *lhs = assignOp->get_lhs_operand();
            ROSE_ASSERT(lhs != NULL);
            
            SgExpression *rhs = assignOp->get_rhs_operand();
            ROSE_ASSERT(rhs != NULL);
            
            OA::ExprHandle rhsHandle = getNodeNumber(rhs);
            OA::MemRefHandle lhsHandle = getNodeNumber(lhs);
            exprStmtPairList->push_back(ExprStmtPair(lhsHandle, rhsHandle));
            
            
            break;
          } // end of case V_SgAssignOp:
          
          // BK:  I am sure that I am missing some stmts that should be
          // flagged as an OA::Activity::EXPR_STMT, but this will get us
          // started
          
        default: 
          {
            break;
          }  
        }
        break;
      } // end of case V_SgExprStatement:
      
    default:
      {
        break;
      }
    }
  } // end of if (OA::Activity::EXPR_STMT)
  OA::OA_ptr<OA::ExprStmtPairIterator> espIter;
  espIter = new SageIRExprStmtPairIterator(exprStmtPairList);
  return espIter;
}

//! Return an iterator over all independent MemRefExpr for given proc
OA::OA_ptr<OA::MemRefExprIterator> 
SageIRInterface::getIndepMemRefExprIter(OA::ProcHandle h)
{ 
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // FIXME  // BK: this routine is incomplete.
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  // Get independent variables
  OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr> > > indepList;
  indepList = new std::list<OA::OA_ptr<OA::MemRefExpr> >;  
  assert(0);
  // not implemented yet
  
  OA::OA_ptr<OA::MemRefExprIterator> indepIter;
  indepIter = new SageMemRefExprIterator(indepList,this);
  return indepIter;
}

//! Return an iterator over all dependent MemRefExpr for given proc
OA::OA_ptr<OA::MemRefExprIterator>
SageIRInterface::getDepMemRefExprIter(OA::ProcHandle h)
{
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // FIXME
  // BK: this routine is incomplete.
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  // Get dependent variables
  OA::OA_ptr<std::list<OA::OA_ptr<OA::MemRefExpr> > > depList;
  depList = new std::list<OA::OA_ptr<OA::MemRefExpr> >;

  assert(0);
  // not implemented yet

  OA::OA_ptr<OA::MemRefExprIterator> depIter;
  depIter = new SageMemRefExprIterator(depList,this);
  return depIter;

}

//! Given a statement, return its Activity::IRStmtType
OA::Activity::IRStmtType 
SageIRInterface::getActivityStmtType(OA::StmtHandle h)
{
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // FIXME
  // BK: this routine is incomplete.
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  SgNode *node = getNodePtr(h);
  ROSE_ASSERT(node != NULL);
  
  OA::Activity::IRStmtType stmtType = OA::Activity::ANY_STMT;
  
  bool collectPtrAssigns = false;
  
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
          // A subset of this case is a new expression.
          SgBinaryOp *assignOp = isSgBinaryOp(expression);
          ROSE_ASSERT(assignOp != NULL);
          
          SgExpression *lhs = assignOp->get_lhs_operand();
          ROSE_ASSERT(lhs != NULL);
          
          SgExpression *rhs = assignOp->get_rhs_operand();
          ROSE_ASSERT(rhs != NULL);

          lhsType = lhs->get_type();
          ROSE_ASSERT(lhsType != NULL);
          
          if (!isSgFunctionCallExp(rhs)) {
            // somehow, cannot handle y = foo() as an EXPR_STMT
            // FIXME ??
            stmtType = OA::Activity::EXPR_STMT;
          }
          /* don't care about this for Activity::StmtType
             if ( lhsType != NULL ) {
             SgType *baseType = getBaseType(lhsType);
             ROSE_ASSERT(baseType != NULL);
             if ( isSgPointerType(baseType) || isSgReferenceType(baseType) ) 
             {
             stmtType = OA::Alias::PTR_ASSIGN_STMT;
             }
             }
          */
          break;
        } // end of case V_SgAssignOp:

        // BK:  I am sure that I am missing some stmts that should be
        // flagged as an OA::Activity::EXPR_STMT, but this will get us
        // started

      default: 
        {
          break;
        }  
      }
      break;
    } // end of case V_SgExprStatement:

  default:
    {
      break;
    }
  }

  return stmtType;
}

//! given a symbol return the size in bytes of that symbol
int 
SageIRInterface::getSizeInBytes(OA::SymHandle h)
{
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  // FIXME
  // BK: this routine is incomplete.
  //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

  int result = 0;

  assert(0);
  // not implemented yet

  return result;
}

//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------  

/** \brief  Create an IRCallsiteParamIterator for the callsite.
 *  \param  h  A CallHandle representing a function or method,
 *             including destructor or constructor, invocation.
 *  \returns  An IRCallsiteParamIterator that iterates
 *            over the actuals in called order.
 *
 *  NB:  The first actual is the receiver for non-static
 *       method invocations, including constructors and
 *       destructors.  
 *
 */
OA::OA_ptr<OA::IRCallsiteParamIterator> 
SageIRInterface::getCallsiteParams(OA::CallHandle h)
{
    SgNode *node = getNodePtr(h);
    ROSE_ASSERT(node != NULL);

    verifyCallHandleType(h);

    // Get the expressions passed as actual arguments 
    // to the function or method invocation.
    std::list<SgNode *> actuals;
    getActuals(node, actuals); 

    // Create a list to hold the handles to the actual arguments.
    OA::OA_ptr<std::list<OA::ExprHandle> > exprHandleList;
    exprHandleList = new std::list<OA::ExprHandle>;

    // Iterate over the actual arguments as represented by
    // SgExpressions.  Convert them to OA::ExprHandle and put
    // them in the list of handles.
    for(std::list<SgNode *>::iterator actualIt = actuals.begin(); 
        actualIt != actuals.end(); ++actualIt) { 

        SgNode *actual = *actualIt;
        ROSE_ASSERT(actual != NULL);

        OA::MemRefHandle actual_memref 
            = findTopMemRefHandle(actual);
        exprHandleList->push_back(actual_memref);
    }

    OA::OA_ptr<OA::IRCallsiteParamIterator> retIter;
    retIter = new SageIRCallsiteParamIterator(exprHandleList);
    return retIter;
}

/** \brief  Return a SymHandle representing a 'this' expression.
 *  \param  node  A SgNode from which we would like to derive
 *                a SymHandle for a 'this' expression.
 *  \returns  A SymHandle representing a 'this' expression
 *            corresponding to the method or SgThisExp
 *            represented by node.
 *
 *  node should be either a SgMemberFunctionDeclaration or
 *  a SgThisExp.
 *
 *  We use the SgFunctionParameterList from the enclosing
 *  method to represent the method-specific 'this' expression.
 *  NB:  Using a class-wide 'this' expression (e.g., a SgClassSymbol)
 *  would lead to loss of precision.
 */
OA::SymHandle SageIRInterface::getThisExpSymHandle(SgNode *node)
{
    SgNode *thisNode = getThisExpNode(node);

    OA::SymHandle symHandle = getNodeNumber(thisNode);      
    return symHandle;
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

// getAttribute is a wrapper around the ROSE attribute mechanism
// which returns a node's attribute.  This was defined for 
// compatibility between older versions of ROSE in which
// attributes were an instance variable of a SgNode, and
// newer versions in which attributes are accessible via a method call.
AstAttributeMechanism &SageIRInterface::getAttribute(SgNode *n)
{
/* AIS - this code works on an earlier version of ROSE.  The code to
         handle the newest version is written below:

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
*/

    ROSE_ASSERT(n);
    if ( n->get_attributeMechanism() == NULL ) {
        AstAttributeMechanism *attr = new AstAttributeMechanism();
        ROSE_ASSERT(attr != NULL);
        n->set_attributeMechanism(attr);
    }
    return *n->get_attributeMechanism();
}

/*
void SageIRInterface::initPointerAssignMaps()
{
  // if haven't already determined the set of memrefs for the program
  // then call initMemRefMaps
  if (mStmt2allMemRefsMap.empty() ) {
      initMemRefMaps();
  }

  // iterate over all procedures
  bool excludeInputFiles = false;  // FIXME: should this be true?
  OA::OA_ptr<OA::IRProcIterator> procIter;
  procIter = new SageIRProcIterator(wholeProject, *this, excludeInputFiles);
  for (procIter->reset() ; procIter->isValid(); ++(*procIter)) {
      OA::ProcHandle currProc = procIter->current();

      // Iterate over the statements of this procedure
      OA::OA_ptr<OA::IRStmtIterator> stmtIterPtr = getStmtIterator(currProc);
      for ( ; stmtIterPtr->isValid(); ++(*stmtIterPtr)) {
          OA::StmtHandle stmt = stmtIterPtr->current();

          // find all of the ptr assign pairs in the statement
          // including those due to parameter bindings involving ptrs
          findAllPtrAssignAndParamBindPairs( getSgNode(stmt), stmt);
      }
  }
}
*/

/*!
 * DECIDED to move all of this to findAllMemRefsAndPtrAssigns.
   Should originally be called on a statement so astNode and stmt will
   be the same.  This is a recursive procedure that traverses down
   the AST and does work in a preorder.
   As pointer assignments are found, mStmtToPtrPairs and
   mCallToParamPtrPairs are updated.
*/
/*
void SageIRInterface::findAllPtrAssignAndParamBindPairs(SgNode *astNode, 
                                                        OA::StmtHandle stmt)
{
    return; // FIXME

    ROSE_ASSERT(astNode != NULL);  bool retVal = false;
    switch(astNode->variantT()) {

    // ---------------------------------------- Expression cases
    case V_SgExprListExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgClassNameRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgMemberFunctionRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionCallExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgSizeOfOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgConditionalExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgNewExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgDeleteExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgThisExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarArgStartOp:
    case V_SgVarArgCopyOp:
    case V_SgVarArgStartOneOperandOp:
        {
            // don't think we are seeing these now, 8/9/06 Dan email
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarArgOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVarArgEndOp:
        {
            ROSE_ASSERT(0);
            break;
        }


    // ---------------------------------------- Initializer cases
    case V_SgInitializedName:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAggregateInitializer:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgConstructorInitializer:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAssignInitializer:
        {
            ROSE_ASSERT(0);
            break;
        }

    // ---------------------------------------- Binary Op cases
    case V_SgArrowExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgDotExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgDotStarOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgArrowStarOp:
        {
            ROSE_ASSERT(0);
            break;
        }

    case V_SgEqualityOp:
    case V_SgLessThanOp:
    case V_SgGreaterThanOp:
    case V_SgNotEqualOp: 
    case V_SgLessOrEqualOp:
    case V_SgGreaterOrEqualOp:
    case V_SgAddOp:
    case V_SgSubtractOp:
    case V_SgMultiplyOp:
    case V_SgDivideOp:
    case V_SgIntegerDivideOp:
    case V_SgModOp: 
    case V_SgAndOp:
    case V_SgOrOp:
    case V_SgBitXorOp: 
    case V_SgBitAndOp:
    case V_SgBitOrOp:
    case V_SgCommaOpExp:
    case V_SgLshiftOp:
    case V_SgRshiftOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgPntrArrRefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAssignOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgPlusAssignOp:
    case V_SgMinusAssignOp:
    case V_SgAndAssignOp:
    case V_SgIorAssignOp:
    case V_SgMultAssignOp:
    case V_SgDivAssignOp:
    case V_SgModAssignOp:
    case V_SgXorAssignOp:
    case V_SgLshiftAssignOp:
    case V_SgRshiftAssignOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    // ---------------------------------------- Unary Op cases
    case V_SgExpressionRoot:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgMinusOp:
    case V_SgUnaryAddOp:
    case V_SgNotOp:
    case V_SgBitComplementOp:
    case V_SgCastExp:
    case V_SgThrowOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgPointerDerefExp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAddressOfOp:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgMinusMinusOp:
    case V_SgPlusPlusOp:
        {
            ROSE_ASSERT(0);
            break;
        }

    // ---------------------------------------- Statement cases
    case V_SgExprStatement:
        {
            ROSE_ASSERT(0);
            break;
        }
    //case V_SgCaseOptionStatement: // NOT in enum? spelled differently?
    //    {
    //        ROSE_ASSERT(0);
    //        break;
    //    }
    //case V_SgTryStatement:
    //    {
    //        ROSE_ASSERT(0);
    //        break;
    //    }
    case V_SgReturnStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgSpawnStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVariableDeclaration:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgVariableDefinition:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgAsmStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgFunctionParameterList:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgCtorInitializerList:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgIfStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgForStatement:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgWhileStmt:
    case V_SgDoWhileStmt:
        {
            ROSE_ASSERT(0);
            break;
        }
    case V_SgCatchOptionStmt:
        {
            ROSE_ASSERT(0);
            break;
        }

    // The statement cases that we should not see
    case V_SgFunctionDefinition:
    case V_SgNamespaceDefinitionStatement:
    case V_SgClassDefinition:
        ROSE_ASSERT(0);
        break;

    // The "do nothing" statement cases
    case V_SgLabelStatement:
    case V_SgDefaultOptionStmt:
    case V_SgBreakStmt:
    case V_SgContinueStmt:
    case V_SgGotoStatement:
    case V_SgForInitStatement: //FIXME: not sure about this one ???
    //case V_SgCatcheStatementSeq: // NOT in enum??  Spelled wrong?
    case V_SgClinkageStartStatement:
    case V_SgEnumDeclaration:
    case V_SgTemplateDeclaration:
    case V_SgNamespaceDeclarationStatement:
    case V_SgNamespaceAliasDeclarationStatement:
    case V_SgUsingDirectiveStatement:
    case V_SgUsingDeclarationStatement:
    case V_SgPragmaDeclaration:
    case V_SgGlobal:
    case V_SgBasicBlock:
        break;

    default:
        ROSE_ASSERT(0);  // should have a case for all possible nodes

    } // end of switch
}
*/

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

// Utility function to look through typedefs to return a type.
SgType* SageIRInterface::getBaseType(SgType *type) 
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

/** \brief Strip off any leading SgCastExps or SgAssignInitializers
 *         in the tree rooted at node.
 *  \param node  A Sage node.
 *  \returns  node stripped of any leading SgCastExps or
 *                 SgAssignInitializers.
 */
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

      SgNode *operand = castExp->get_operand();
      ret = lookThroughCastExpAndAssignInitializer(operand);
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

bool SageIRInterface::isMemRefNode(SgNode *astNode)
{ 
    // initialize all of the mre information if it hasn't been done
    if (mMemref2mreSetMap.empty()) {
        initMemRefAndPtrAssignMaps();
    }

    // If this node does not have any MREs associated with it then it
    // is not a MemRefHandle
    if (mMemref2mreSetMap[OA::MemRefHandle((OA::irhandle_t)astNode)].empty()) {
        return false;
    } else {
        return true;
    }
}

OA::ProcHandle SageIRInterface::getProcHandle(SgFunctionDefinition *node)
{
      OA::ProcHandle procHandle = getNodeNumber(node);
        return procHandle;
}

/*!
   Verify that a call handle is actually a node
   of the expected type.
*/
void SageIRInterface::verifyCallHandleType(OA::CallHandle call)
{
    SgNode *node = getNodePtr(call);
    verifyCallHandleNodeType(node);
}

/*!
   Verify that a stmt handle is actually a node
   of the expected type.
*/
void SageIRInterface::verifyStmtHandleType(OA::StmtHandle stmt)
{
    SgNode *node = getNodePtr(stmt);
    verifyStmtHandleNodeType(node);
}

OA::SymHandle SageIRInterface::getSymHandle(SgNode *astNode)
{
    verifySymHandleNodeType(astNode);
    OA::SymHandle symHandle = getNodeNumber(astNode);
    return symHandle;
}

OA::SymHandle SageIRInterface::getVTableBaseSymHandle(SgClassDefinition *classDefn)
{
    return getSymHandle(classDefn);
}
