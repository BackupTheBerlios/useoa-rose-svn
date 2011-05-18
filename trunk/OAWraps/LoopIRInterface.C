/* By: Andy Stone (aistone@gmail.com) */

/* NOTE: This code is based off of the Recognizing Loops tutorial in the ROSE
   tutorial pdf. */

/* The purpose of this code is to manually perform a loop detection analysis
   in ROSE.  This sort of analysis should ultimatly be handeled by
   OpenAnalysis, but for now it's much easier to just do it in ROSE directly.
   The analysis works by performing a top down AST traversal (with the
   LoopNestProcessor class), each time a for loop is encountered the loop is
   abstracted into a LoopAbstraction object. */


#include "LoopIRInterface.h"


// various debugging flags can be set by the OA_DEBUG environmental variable.
// when one of the debugging flags is on it will output loops that fail
// the associated requirement.
bool debugLoop       = false;   // set with LoopStats. Turns on the flags below
bool debugBadInit    = false;   // set with LoopStats_BadInit
bool debugLowerBound = false;   // set with LoopStats_BadLowerBound
bool debugUpperBound = false;   // set with LoopStats_BadUpperBound
bool debugBadStep    = false;   // set with LoopStats_BadStep


void LoopAnalStatistics::incrementRejectedCounter(Rejection reason) {
    switch(reason) {
        case BAD_INIT:        mnRejected_BadInit++;       break;
        case BAD_LOWER_BOUND: mnRejected_BadLowerBound++; break;
        case BAD_UPPER_BOUND: mnRejected_BadUpperBound++; break;
        case BAD_STEP:        mnRejected_BadStep++;       break;
        case REDEFINITION:    mnRejected_Redefinition++;  break;
        case INCLUDES_FUNCTION_CALL: mnRejected_FunctionCall++; break;
        case INCLUDES_DEREF:         mnRejected_Deref++; break;
    };

    if(debugLoop) {
        cerr << "Loop was rejected for: "
             << rejectionStr[reason] << endl;
    }
}

const char *LoopAnalStatistics::rejectionStr[] = {
    "bad initialization of index variable.",
    "invalid lower bound statement.",
    "invalid upper bound statement.",
    "invalid step statement.",
    "redefinition of index variable.",
    "includes function call.",
    "includes pointer dereference."
};


LoopNestAttribute::LoopNestAttribute() { }

// copy constructor
LoopNestAttribute::LoopNestAttribute(
    const LoopNestAttribute &orig)
{
    // copy all elements in the nested loops member of orig into the
    // nested loops member of this (being) constructed loop.
    mNestedLoops.insert(
        mNestedLoops.begin(),
        orig.mNestedLoops.begin(),
        orig.mNestedLoops.end());
}

void LoopNestAttribute::addNestedLoop(OA_ptr<LoopAbstraction> loop) {
    mNestedLoops.push_back(loop);
}



LoopNestAttribute LoopNestProcessor::evaluateInheritedAttribute(
    SgNode *astNode,
    LoopNestAttribute attrib)
{
    // whenever a while or do while statement is encountered increment
    // the appropriate counter
    if(astNode->variantT() == V_SgWhileStmt) {
        gStatistics.encounteredWhileLoop();
    }
    else if(astNode->variantT() == V_SgDoWhileStmt) {
        gStatistics.encounteredDoWhileLoop();
    }

    // the attribute only needs to be modified within for statements
    else if(astNode->variantT() == V_SgForStatement) {
    	std::cout<<"\n$$$Traversal found a forloop";
        gStatistics.encounteredForLoop();
        SgForStatement *stmt = isSgForStatement(astNode);
        assert(stmt != NULL);

        // generate a LoopAbstraction object from the sage node
        // if a loop object can not be created (wrong format) NULL
        // will be returned.
        OA_ptr<LoopAbstraction> loop =
            buildLoopAbstraction(stmt, attrib);

        // add this new loop to the results and to the nest
        if(!loop.ptrEqual(0)) {
        	std::cout<<"\n$$$Traversal found a forloop";

            mResults->push_back(loop);
            attrib.addNestedLoop(loop);
        }
    }

    return attrib;
}


OA_ptr<MemRefExpr> LoopNestProcessor::extractIndexVariable(
    SgForInitStatement *init, bool *error, bool *isCStyleLoop) 
{
    OA_ptr<MemRefExpr> ret;

    // There are two different types of loop initializations handeled.
    // 'C-style' and 'C++ style'. C style loops do not declare the index
    // variable, but rather redefine it to some lower bound, C++ style loops
    // do both.  Examples:
    //
    //    C style loop:
    //
    //        int idx;
    //        for(idx = 0; idx < 10; idx++) {
    //           // do something.
    //        }
    //
    //
    //    C++ style loop:
    //
    //        for(int idx = 0; idx < 10; idx++) {
    //           // do something.
    //        }
    //


    // first try and extract a C style initialization, if there wasn't
    // an error return the results from there.  Otherwise reset error, so
    // we can itest for a CPP style loop.
    ret = extractIndexVariableInCStyleLoop(init, error);
    if(*error == false) {
        (*isCStyleLoop) = true;
        return ret;
    }
    (*error) = false;
    (*isCStyleLoop) = false;

    ret = extractIndexVariableInCPPStyleLoop(init, error);
    return ret;
}


OA_ptr<MemRefExpr> LoopNestProcessor::extractIndexVariableInCStyleLoop(
    SgForInitStatement *init, bool *error)
{
    OA_ptr<MemRefExpr> ret;

    // First look for C style for loops, that is loops where the index
    // variable is declared outside of the 'for' statement, but initialized
    // to the lower value in it.  Specifically look for this pattern:
    //   SgForInitStatement
    //     SgExprStatement      (first el. in list ret. by get_init_stmt)
    //       SgExpressionRoot   (get_expression_root)
    //         SgAssignOp       (get_statement)
    //           SgVarRefExp    (get_lhs_operand)
    //           SgIntVal       (get_rhs_operand)
    SgStatementPtrList lst = init->get_init_stmt();
    if(lst.size() != 1)  { *error = true; return ret; }
    
    SgExprStatement *exp =
        isSgExprStatement(lst.front());
    if(exp == NULL) { *error = true; return ret; }

    SgAssignOp *assign = 
       isSgAssignOp(exp->get_expression());
    if(assign == NULL) { *error = true; return ret; }

    SgVarRefExp *varRef =
        isSgVarRefExp(assign->get_lhs_operand());
    if(varRef == NULL) { *error = true; return ret; }

    SgIntVal *iVal =
        isSgIntVal(assign->get_rhs_operand());
    if(iVal == NULL) { *error = true; return ret; }


    // determine where the variable is declared, this declaration node
    // is what we use to derive the symbol handle for the index variable
    SgVariableSymbol  *varSym   = varRef->get_symbol();
    SgInitializedName *initName = varSym->get_declaration();

    // Convert the variable to a location
    SymHandle hSym = mIR.getVarSymHandle(initName);
    //ret = (mIR.getLocation(mProc, hSym)).convert<NamedLoc>();
    ret = mIR.createNamedRef(hSym);

    return ret;
}


OA_ptr<MemRefExpr> LoopNestProcessor::extractIndexVariableInCPPStyleLoop(
    SgForInitStatement *init, bool *error)
{
    OA_ptr<MemRefExpr> ret;

    // check to see if the init statement has a single variable
    // assignment, within the assignment get the variable reference
#ifdef PRE_ROSE_0_9_0B
    list<SgNode*> declarations = 
        NodeQuery::querySubTree(init, V_SgVariableDeclaration);
#else
    NodeQuerySynthesizedAttributeType declarations = 
        NodeQuery::querySubTree(init, V_SgVariableDeclaration);
#endif
    if(declarations.size() != 1) {
        (*error) = true;
        return ret;
    }
    SgVariableDeclaration *decl =
        isSgVariableDeclaration(declarations.front());
    
    // Get a list of the variables declared in the decl statement
    // Assure that there's only one
    SgInitializedNamePtrList ptrList = decl->get_variables();
    if(ptrList.size() != 1) {
        (*error) = true;
        return ret;
    }
    SgInitializedName *var = ptrList.front();
    
    // Convert the variable to a location
    SymHandle hSym = mIR.getVarSymHandle(var);
    //ret = (mIR.getLocation(mProc, hSym)).convert<NamedLoc>();
    ret = mIR.createNamedRef(hSym);

    return ret;
}

int LoopNestProcessor::extractLowerBound(
    OA_ptr<MemRefExpr> indexVariable,
    SgForInitStatement *init,
    bool *error,
    bool isCStyleLoop)
{
    OA_ptr<MemRefExpr> ret;

    // determine how to extract the lower bound based on what type of loop
    // is being analyzed
    if(isCStyleLoop) {
        return extractLowerBoundInCStyleLoop(indexVariable, init, error);
    } else {
        return extractLowerBoundInCPPStyleLoop(indexVariable, init, error);
    }
}


int LoopNestProcessor::extractLowerBoundInCStyleLoop(
    OA_ptr<MemRefExpr> indexVariable,
    SgForInitStatement *init,
    bool *error)
{
    OA_ptr<MemRefExpr> ret;

    // The subtree for the initialization cluase in a C style loop looks
    // like this:
    //
    //   SgForInitStatement
    //     SgExprStatement      (first el. in list ret. by get_init_stmt)
    //       SgExpressionRoot   (get_expression_root)
    //         SgAssignOp       (get_statement)
    //           SgVarRefExp    (get_lhs_operand)
    //           SgIntVal       (get_rhs_operand)
    //
    // I need to extract that SgIntVal node and return the value it
    // represents

    SgStatementPtrList lst = init->get_init_stmt();
    if(lst.size() != 1)  { *error = true; return -1; }
    
    SgExprStatement *exp =
        isSgExprStatement(lst.front());
    if(exp == NULL) { *error = true; return -1; }

    SgAssignOp *assign = 
       isSgAssignOp(exp->get_expression());
    if(assign == NULL) { *error = true; return -1; }

    SgVarRefExp *varRef =
        isSgVarRefExp(assign->get_lhs_operand());
    if(varRef == NULL) { *error = true; return -1; }

    SgIntVal *iVal =
        isSgIntVal(assign->get_rhs_operand());
    if(iVal == NULL) { *error = true; return -1; }

    return iVal->get_value();
}


int LoopNestProcessor::extractLowerBoundInCPPStyleLoop(
    OA_ptr<MemRefExpr> indexVariablemre,
    SgForInitStatement *init,
    bool *error)
{
    OA_ptr<MemRefExpr> ret;

    ROSE_ASSERT(indexVariablemre->isaNamed());
    OA::OA_ptr<OA::NamedRef> indexVariable = indexVariablemre.convert<OA::NamedRef>();
    ROSE_ASSERT(!indexVariable.ptrEqual(0));
    ROSE_ASSERT(indexVariable->isaNamed());

    // get the node where the index variable is initialized, from
    // there get the node where it's assigned, from there get the
    // associated integer constant
    SgInitializedName *varNode =
        isSgInitializedName(mIR.getNodePtr(indexVariable->getSymHandle()));
    if(varNode == NULL) { *error = true;  return -1; }
    SgAssignInitializer *initNode =
        isSgAssignInitializer(varNode->get_initializer());
    if(initNode == NULL) { *error = true;  return -1; }
    SgIntVal *val =
        isSgIntVal(initNode->get_operand_i());
    if(val == NULL) { *error = true;  return -1; }

    return val->get_value();
}


int LoopNestProcessor::extractUpperBound(
    OA_ptr<MemRefExpr> indexVariable,
    SgStatement *condition,
    bool *error)
{
    // We're looking for a subtree in *exactly* the following format:
    //   SgExprStatement
    //       SgExpressionRoot   (from SgForStatement::get_text_expr_root)
    //       SgLessThanOp       (get_operand_i)
    //         SgVarRefExp      (get_lhs_operand)
    //         SgIntVal         (get_rhs_operand)

    SgExprStatement *exp =
        isSgExprStatement(condition);
    if(exp == NULL) { *error = true;  return -1; }

    SgLessThanOp *op =
        isSgLessThanOp(exp->get_expression());
    if(op == NULL) { *error = true;  return -1; }

    SgVarRefExp *var =
        isSgVarRefExp(op->get_lhs_operand());
    if(var == NULL) { *error = true;  return -1; }

    SgIntVal *val =
        isSgIntVal(op->get_rhs_operand());
    if(val == NULL) { *error = true;  return -1; }

    return val->get_value();
}


int LoopNestProcessor::extractLoopStep(
    OA_ptr<MemRefExpr> indexVariable,
    SgExpression *stepExpr,
    bool *error)
{
    // Loop steps of an index variable (i) can come in the following format:
    // i++ or ++i       (type 1)
    // i = i + b        (type 2)
    // i += b           (type 3)

    // Check for a type 1 increment
    // SgExpressionRoot
    //   SgPlusPlusOp
    //   SgVarRefExp
  {   // each type is placed in its own scope to assure against
      // name clashes
    SgPlusPlusOp *plusPlusOp;
    SgVarRefExp *varRefExp;

    if(((plusPlusOp = isSgPlusPlusOp(stepExpr)) != NULL) &&
       ((varRefExp = isSgVarRefExp(plusPlusOp->get_operand())) != NULL))
    {
        // Assure that the variable being incremented in the type 1 expression
        // is actually the proper index variable.
        if(!isIndexVariable(varRefExp, indexVariable)) {
            *error = true;
            return -1;
        }

        return 1;
    }
  }

    // Check for a type 2 increment
    // SgExpressionRoot
    //   SgAssignOp
    //     SgVarRefExp
    //     SgAddOp
    //       SgVarRefExp    <- The order of these two is arbitray
    //       SgIntVal       <- 
  {
    SgAssignOp *assignOp;
    SgVarRefExp *varRefExp;
    SgAddOp *addOp;
    SgVarRefExp *varRefExp2;
    SgIntVal *intVal;

    if(((assignOp = isSgAssignOp(stepExpr)) != NULL) &&
       ((varRefExp  = isSgVarRefExp(assignOp->get_lhs_operand())) != NULL) &&
       ((addOp = isSgAddOp(assignOp->get_rhs_operand())) != NULL) &&
        ((((varRefExp2 = isSgVarRefExp(addOp->get_lhs_operand())) != NULL) &&
          ((intVal = isSgIntVal(addOp->get_rhs_operand())) != NULL)) ||
        (((intVal = isSgIntVal(addOp->get_lhs_operand())) != NULL) &&
         ((varRefExp2 = isSgVarRefExp(addOp->get_rhs_operand())) != NULL))))
    {

        // Assure that the variable being incremented in the type 2 expression
        // is actually the proper index variable.
        if(!isIndexVariable(varRefExp, indexVariable) ||
           !isIndexVariable(varRefExp2, indexVariable))
        {
            *error = true;
            return -1;
        }

        return 1;
    }
  }

    // Check for a type 3 increment
    // SgExpressionRoot
    //   SgPlusAssignOp
    //     SgVarRefExp
    //     SgIntVal
  {
    SgPlusAssignOp *plusAssignOp;
    SgVarRefExp *varRefExp;
    SgIntVal *intVal;

    if(((plusAssignOp = isSgPlusAssignOp(stepExpr)) != NULL) &&
       ((varRefExp = isSgVarRefExp(plusAssignOp->get_lhs_operand())) != NULL) &&
       ((intVal = isSgIntVal(plusAssignOp->get_rhs_operand())) != NULL))
    {
        
        // Assure that the variable being incremented in the type 3 expression
        // is actually the proper index variable.
        if(!isIndexVariable(varRefExp, indexVariable)) {
            *error = true;
            return -1;
        }

        return 1;
    }
  }

    *error = true;
    return -1;
}

bool LoopNestProcessor::isIndexVariable(
    SgVarRefExp *varRef,
    OA_ptr<MemRefExpr> idxVarmre)
{
    ROSE_ASSERT(idxVarmre->isaNamed());
    OA::OA_ptr<OA::NamedRef> idxVar = idxVarmre.convert<OA::NamedRef>();
    ROSE_ASSERT(!idxVar.ptrEqual(0));
    ROSE_ASSERT(idxVar->isaNamed());

    // convert the OA abstraction to a SAGE node
    SgInitializedName *indexVarNode = 
        isSgInitializedName(mIR.getNodePtr(idxVar->getSymHandle()));
    assert(indexVarNode != NULL);

    // convert the variable refernce to what should be the same SAGE node
    // if this is the same variable.
    SgVariableSymbol *varSym = varRef->get_symbol();
    SgInitializedName *indexVarNode2 = varSym->get_declaration();

    return (indexVarNode == indexVarNode2);
}

StmtHandle LoopNestProcessor::extractLastLoopStmt(StmtHandle stmt) {
    OA_ptr<IRRegionStmtIterator> bodyIter = mIR.loopBody(stmt);
    StmtHandle last = 0;
    for(; bodyIter->isValid(); (*bodyIter)++) {
        last = bodyIter->current();
    }

    // if the last statement was another block of some sort, rescurse
    if(isSgScopeStatement(mIR.getNodePtr(last)) != NULL) {
        StmtHandle recursedLast;
        recursedLast = extractLastLoopStmt(last);

        if(recursedLast) { last = recursedLast; }
    }

    return last;
}

// Query for assignment operations like '=', '+=', '*=', '/=', or '%='
static NodeQuerySynthesizedAttributeType queryAssignments(SgNode *astNode) {
    assert(astNode != 0);
    NodeQuerySynthesizedAttributeType returnNodeList;

    if((isSgAssignOp(astNode) != NULL) ||
       (isSgDivAssignOp(astNode) != NULL) ||
       (isSgIorAssignOp(astNode) != NULL) ||
       (isSgLshiftAssignOp(astNode) != NULL) ||
       (isSgMinusAssignOp(astNode) != NULL) ||
       (isSgModAssignOp(astNode) != NULL) ||
       (isSgMultAssignOp(astNode) != NULL) ||
       (isSgPlusAssignOp(astNode) != NULL) ||
       (isSgRshiftAssignOp(astNode) != NULL) ||
       (isSgXorAssignOp(astNode) != NULL) ||
       (isSgAndAssignOp(astNode) != NULL))
    {
        returnNodeList.push_back(astNode);
    }

    return returnNodeList;
}


OA_ptr<LoopAbstraction> LoopNestProcessor::buildLoopAbstraction(
    SgNode *forStatementNode, const LoopNestAttribute &attrib)
{
    OA_ptr<MemRefExpr> indexVariable;
    OA_ptr<LoopAbstraction> loopAbstraction;
    bool rejected = false;
    bool error    = false;
    bool isCStyleLoop = false;
    int lowerBound, upperBound, step;


    if(debugLoop) {
        cerr << "Trying to build an abstraction from the statement: "
             << forStatementNode->unparseToString() << endl;
    }

    // note: many of the functions used here will set the variable error to
    // true if they can't perform their duty.  Whenever this happens a
    // NULL pointer should be returned.

    SgForStatement *forStatement;
    forStatement = isSgForStatement(forStatementNode);

    // get the loop index variable from the initialization node
    SgForInitStatement *init = forStatement->get_for_init_stmt();
    indexVariable = extractIndexVariable(init, &error, &isCStyleLoop);
    if(error) {
        rejected = true;
        gStatistics.incrementRejectedCounter(LoopAnalStatistics::BAD_INIT);
       
        // output the loop if the debugging flag to do so is on
        if(debugBadInit) {
            cerr << "BadInit: " << forStatement->unparseToString() << endl;
        }

        return loopAbstraction;
    }

    // get the loop's lower bound
    lowerBound = extractLowerBound(indexVariable, init, &error, isCStyleLoop);
    if(error) {
        rejected = true;
        gStatistics.incrementRejectedCounter(
            LoopAnalStatistics::BAD_LOWER_BOUND);

        // output the loop if the debugging flag to do so is on
        if(debugLowerBound) { cerr << forStatement->unparseToString() << endl; }

        return loopAbstraction;
    }

    // get the loop's upper bound
    SgStatement *condition = forStatement->get_test();
    upperBound = extractUpperBound(indexVariable, condition, &error);
    if(error) {
        rejected = true;
        gStatistics.incrementRejectedCounter(
            LoopAnalStatistics::BAD_UPPER_BOUND);

        // output the loop if the debugging flag to do so is on
        if(debugUpperBound) { cerr << forStatement->unparseToString() << endl; }

        return loopAbstraction;
    }

    // get the loop's step
    SgExpression *stepExpr = forStatement->get_increment();
    step = extractLoopStep(indexVariable, stepExpr, &error);
    if(error) {
        cout << "Rejected loop: " << forStatementNode->unparseToString()
             << endl << endl << endl;
        rejected = true;
        gStatistics.incrementRejectedCounter(LoopAnalStatistics::BAD_STEP);

        // output the loop if the debugging flag to do so is on
        if(debugBadStep) { cerr << forStatement->unparseToString() << endl; }

        return loopAbstraction;
    }

    // check to assure that the index variable is not redefined anywhere
    // in the loop body.  Search for assignment expressions with the
    // index variable as the LHS
#ifdef PRE_ROSE_0_9_0B
    list<SgNode*> assignments = 
        NodeQuery::querySubTree(
            forStatement->get_loop_body(), &queryAssignments);
#else
    NodeQuerySynthesizedAttributeType assignments = 
        NodeQuery::querySubTree(
            forStatement->get_loop_body(), &queryAssignments);
#endif    
    if(!assignments.empty()) {
        // check to see if the LHS of any of the assignments is the
        // index variable
#ifdef PRE_ROSE_0_9_0B
        for(list<SgNode*>::iterator i = assignments.begin();
            i != assignments.end(); i++)
#else
        for(NodeQuerySynthesizedAttributeType::iterator i = assignments.begin();
            i != assignments.end(); i++)
#endif
        {
            SgBinaryOp *binOp = isSgBinaryOp(*i);

            assert(binOp != NULL);
            SgVarRefExp *varRef = isSgVarRefExp(binOp->get_lhs_operand());
            if(varRef == NULL) { continue; }

            if(isIndexVariable(varRef, indexVariable)) {
                rejected = true;
                gStatistics.incrementRejectedCounter(
                    LoopAnalStatistics::REDEFINITION);
                return loopAbstraction;
            }
        }
    }

    // check to assure that the loop contains no function calls
#ifdef PRE_ROSE_0_9_0B
    list<SgNode*> functions =
#else
    NodeQuerySynthesizedAttributeType functions = 
#endif
        NodeQuery::querySubTree(
            forStatement->get_loop_body(), V_SgFunctionCallExp);
    if(!functions.empty()) {
        rejected = true;
        gStatistics.incrementRejectedCounter(
            LoopAnalStatistics::INCLUDES_FUNCTION_CALL);
        return loopAbstraction;
    }

    // check to assure that the loop contains no dereferences
#ifdef PRE_ROSE_0_9_0B
    list<SgNode*> pointers =
#else
    NodeQuerySynthesizedAttributeType pointers =
#endif
        NodeQuery::querySubTree(
            forStatement->get_loop_body(), V_SgPointerDerefExp);
    if(!pointers.empty()) {
        gStatistics.incrementRejectedCounter(
            LoopAnalStatistics::INCLUDES_DEREF);
        return loopAbstraction;
    }

    // now determine the statement that is the end of this loop
    StmtHandle stmt = (OA::irhandle_t)(mIR.getNodeNumber(forStatement));    
    StmtHandle last = extractLastLoopStmt(stmt);

    
    // otherwise use the information calculated above to construct a new
    // LoopAbstraction object.
    OA_ptr<LoopIndex> loopIdx;
    OA_ptr<LoopAbstraction> parent;
    parent = attrib.getLastLoop();
    loopIdx = new LoopIndex(indexVariable, lowerBound, upperBound, step);
    loopAbstraction = new LoopAbstraction(loopIdx, stmt, last, parent);

    gStatistics.incrementAcceptedCounter();
    return loopAbstraction;
}

// debugging function to output the contents of a list of loops
void outputListOfLoops(
    OA_ptr<list<OA_ptr<LoopAbstraction> > > loops,
    IRHandlesIRInterface &ir)
{
    // iterate through the list, output each object
    for(list<OA_ptr<LoopAbstraction> >::iterator i = loops->begin();
        i != loops->end();
        i++)
    {
        (*i)->output(ir);
    }
}

/*! Temporary function to explicitly gather loops from ROSE.  This is a
    compiler dependent loop detection analysis. OA should really implement
    it's own compiler-independent version. */
OA_ptr<list<OA_ptr<LoopAbstraction> > >
SageIRInterface::gatherLoops(const ProcHandle &proc)
{
    // check the OA_DEBUG environmental variable to see what sorts of debugging
    // information should be output
    OA_DEBUG_CTRL_MACRO("LoopStats:ALL", debugLoop);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadInit:ALL", debugBadInit);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadLowerBound:ALL",
                        debugLowerBound);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadUpperBound:ALL",
                        debugUpperBound);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadStep:ALL", debugBadStep);

    debugLoop = true;
    if(debugLoop) {
        cerr << endl;
        cerr << "Gather loops for procedure: "
             << toString(proc) << endl;
        cerr << "------------------------------------------------------"
             << endl;
    }

    // construct a list to hold the results (the found lists) in
    OA_ptr<list<OA_ptr<LoopAbstraction> > > loops;
    loops = new list<OA_ptr<LoopAbstraction> >();

    // run the loop detection analysis, this analysis will store the results
    // in the passed list (loops).
    LoopNestProcessor processor(loops, *this, proc);
    LoopNestAttribute attrib;
    processor.traverse(getNodePtr(proc), attrib);

    // if debug mode is on output results:
    if(debugLoop) {
//        outputListOfLoops(loops, *this);
//        cerr << endl;
        gStatistics.output();
        cerr << endl;
    }

    return loops;
}

StmtHandle SageIRInterface::findEnclosingLoop(const StmtHandle &stmt)
{
    // traverse from the statement up to the top of the tree until we
    // find a for loop
    SgNode *current = (getNodePtr(stmt))->get_parent();
    while(isSgForStatement(current) == NULL && current != NULL) {
        current = current->get_parent();
    }

    // if we didn't reach the top convert the SgNode to a statement handle
    // and return it, otherwise return the NULL StmtHandle
    if(current != NULL) {
        return (irhandle_t)(getNodeNumber(current));
    } else {
        return 0;
    }
}

StmtHandle SageIRInterface::findEnclosingLoop(const SgNode *node)
{
    // traverse from the node up to the top of the tree until we
    // find a for loop
    SgNode *current = node->get_parent();
    while(isSgForStatement(current) == NULL && current != NULL) {
        current = current->get_parent();
    }

    // if we didn't reach the top convert the SgNode to a statement handle
    // and return it, otherwise return the NULL StmtHandle
    if(current != NULL) {
        return (irhandle_t)(getNodeNumber(current));
    } else {
        return 0;
    }
}

