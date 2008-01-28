#include "SageOACallGraph.h"
#include "common.h"

using namespace UseOA;

//SageIRCallsiteIterator implementation

SageIRCallsiteIterator::SageIRCallsiteIterator(SgStatement * sgstmt, 
                                               SageIRInterface &in)
    : ir(in)
{
 //given an sgstmt put all call expressions in calls_in_stmt
  // Do not look for call sites within a SgClassDefinition.  We
  // return this as a statement so we can create implicit
  // virtual method ptr assign pairs from it.
  // Its procedures will be passed explicitly as statements.
  // If we recurse through the class definition, we will
  // visit them twice.
  if ( !isSgClassDefinition(sgstmt) && !isSgClassDeclaration(sgstmt) ) {
      FindCallsitesInSgStmt(sgstmt, calls_in_stmt);
  }
  begin = calls_in_stmt.begin();
  st_iter = calls_in_stmt.begin();
  end = calls_in_stmt.end();
  valid=TRUE;
}

void SageIRCallsiteIterator::FindCallsitesInSgStmt(SgStatement *sgstmt, std::list<SgNode*> & lst)
{
  //we only want Callsites in non-scope stmts, for scopes only in non-body
  if(!isSgScopeStatement(sgstmt))
    FindCallsitesPass(ir, lst).traverse(sgstmt, preorder);
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
       FindCallsitesPass(ir, lst).traverse(co->get_condition(), preorder);
     //SgClassDefinition ->no call sites
     else if(dw=isSgDoWhileStmt(sgstmt))
       FindCallsitesPass(ir, lst).traverse(dw->get_condition(), preorder);
     else if(forst=isSgForStatement(sgstmt))
     {
       
       FindCallsitesPass(ir, lst).traverse(forst->get_test_expr(), preorder);
#ifdef ROSE_PRE_0_8_10A
       FindCallsitesPass(ir, lst).traverse(forst->get_increment_expr(), preorder);
#else
       FindCallsitesPass(ir, lst).traverse(forst->get_increment(), preorder);
#endif
       FindCallsitesPass(ir, lst).traverse(forst->get_for_init_stmt(), preorder);
     }
     //SgFunctionDefinition ->no call sites
     //SgGlobal ->no call sites
     else if(ifst=isSgIfStmt(sgstmt))
     {
       FindCallsitesPass(ir, lst).traverse(ifst->get_conditional(), preorder);
     }
     //SgNamespaceDefinition -> no call sites
     //SgSwitchStatement -> no call sites
     else if(whst=isSgWhileStmt(sgstmt))
     {
       FindCallsitesPass(ir, lst).traverse(whst->get_condition(), preorder);
     }
     
  }
  
}

// Returns the current item.
OA::CallHandle SageIRCallsiteIterator::current() const
{
  SgNode* cur = *st_iter;
  OA::CallHandle h = 0;

  if (isValid()) {
    //cerr << "cur expr: " << cur->unparseToString() << endl;
    h = (OA::irhandle_t)(ir.getNodeNumber(cur));    
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

void SageIRProcIterator::FindProcsInSgTree(SgNode *node, std::set<SgStatement *>& lst)
{
  if ( mExcludeInputFiles ) {
    //    std::cout << "Exclude = true" << std::endl;
    // Only visit those AST nodes defined within the same file as node.
    // i.e., ignore AST nodes pulled in from input files.
    SgProject *project = isSgProject(node);
    if ( project != NULL ) {
      FindProcsPass(lst).traverseInputFiles(project, preorder);
    } else {
      FindProcsPass(lst).traverseWithinFile(node, preorder);
    }
  } else {
    //    std::cout << "Exclude = false" << std::endl;
    FindProcsPass(lst).traverse(node, preorder);
  }
}

SageIRProcIterator::SageIRProcIterator(SgNode *node, 
                                       SageIRInterface& in)
  : ir(in), mExcludeInputFiles(in.excludeInputFiles())
{
 //given an sgstmt put all call expressions in calls_in_stmt
   FindProcsInSgTree(node, procs_in_proj);
  begin = procs_in_proj.begin();
  st_iter = procs_in_proj.begin();
  end = procs_in_proj.end();
  valid=TRUE;
}


// Returns the current item.
OA::ProcHandle SageIRProcIterator::current() const
{
  SgStatement* cur = *st_iter;
  OA::ProcHandle h = 0;

  if (isValid()) {
    //cerr << "cur stmt: " << cur->unparseToString() << endl;
    h = (OA::irhandle_t)(ir.getNodeNumber(cur));    
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

void SageIRProcIterator::unionIterators(SageIRProcIterator &otherIter)
{
    // It only makes sense to union ProcHandles from the same IR interface.
    ROSE_ASSERT(&ir == &otherIter.ir);

    std::set<SgStatement *> tmp;
    std::set_union(procs_in_proj.begin(), procs_in_proj.end(),
                   otherIter.procs_in_proj.begin(), otherIter.procs_in_proj.end(),
                   std::inserter(tmp, tmp.end()));
    procs_in_proj = tmp;
    
    // Recent the iterator pointers.  We may get unexpected results
    // if we are in the middle of iterating over this.
    begin = procs_in_proj.begin();
    st_iter = procs_in_proj.begin();
    end = procs_in_proj.end();

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
  // 2/1/06 BW:  Was only capturing constructor invocations thru
  // new, also need to get those involved in a stack allocation.
  // Both can be captured by looking at SgConstructorInitializers instead.

  if( isSgFunctionCallExp(exp) ) {
      if (!isVaStart(isSgFunctionCallExp(exp))) {
	//	std::cout << "Found call site: " << node->unparseToString() << std::endl;
        call_lst.push_back(exp);
      }

  } else if ( isSgNewExp(exp) ) {

      SgNewExp *newExp = isSgNewExp(exp);
      ROSE_ASSERT(newExp != NULL);

      if ( isPlacementNew(newExp) ) {
          call_lst.push_back(newExp);
      }

  } else if ( isSgConstructorInitializer(exp) ) {
    //    ROSE_ASSERT(0); // MMS, not sure how we handle these yet

    // I don't like this ... I suspect this problem will go
    // away after the AST is properly normalized.  Until then,
    // this ugly (and probably incorrect) approach will suffice
    // to effectively ignore constructor invocations on base
    // types (as well as any other type which does not define
    // constructors explicitly).  BW

    // 2/1/06 BW:  Only consider this a function call if it creates
    // a named type.  Otherwise we get problems (e.g., in getFormalTypes)
    // when we expect that a basic type has a constructor with a 
    // function declaration, formal parameters, etc.
    SgConstructorInitializer *ctorInitializer =
      isSgConstructorInitializer(exp);
    ROSE_ASSERT(ctorInitializer != NULL);

    if ( !mIR.createsBaseType(ctorInitializer) ) {
      //      call_lst.push_back(exp);
      // (5/3/07 BW) I changed constructor invocations to be
      // represented by the Sg_File_Info at the SgConstructorInitializer.
      Sg_File_Info *fileInfo = ctorInitializer->get_file_info();
      ROSE_ASSERT(fileInfo != NULL);
      call_lst.push_back(fileInfo);
    } 

  } else if ( isSgDeleteExp(exp) ) {
    // Do not mark a destructor as a call site
    // if it destroys a basic type.
    SgDeleteExp *deleteExp = isSgDeleteExp(exp);
    ROSE_ASSERT(deleteExp != NULL);

    SgExpression *receiver = deleteExp->get_variable();
    ROSE_ASSERT(receiver != NULL);

    SgType *type = receiver->get_type(); 
    ROSE_ASSERT(type != NULL);

    // Need getBaseType to look through typedefs.
    SgPointerType *ptrType = isSgPointerType(lookThruReferenceType(type));
    ROSE_ASSERT(ptrType != NULL);
    type = getBaseType(ptrType->get_base_type());

    ROSE_ASSERT(type != NULL);

    // If this is a base type, then do nothing more--
    // i.e., not even a call handle.
    if ( isSgClassType(type) ) {

      // AST NORMALIZATION
      // Also, do not create a call handle if this is an
      // invocation of a destructor without a definition,
      // i.e., a default destructor.  Eventually, once
      // the AST is normalized (i.e., all special method
      // definitions and invocations are made explicit),
      // then all destructors will have definitions.
      SgClassDefinition *classDefn = getClassDefinition(type);
      ROSE_ASSERT(classDefn != NULL);
    
      SgMemberFunctionDeclaration *methodDecl = 
          lookupDestructorInClass(classDefn); 
      if ( methodDecl != NULL ) {
          Sg_File_Info *fileInfo = exp->get_file_info();
          ROSE_ASSERT(fileInfo != NULL);
          // Use the Sg_File_Info of a SgDeleteExp to
          // represent the invocation of a destructor.
          call_lst.push_back(fileInfo);
      }
    }
  }

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
  {
    //    std::cout << "Found proc: " << isSgFunctionDefinition(node)->get_declaration()->get_name().str() << std::endl;
    proc_lst.insert(isSgStatement(node));
  }
  return;
}

