/* By: Andy Stone (aistone@gmail.com) */

/* NOTE: This code is based off of the Recognizing Loops tutorial in the ROSE
   tutorial pdf. */

/* The purpose of this code is to manually perform a loop detection analysis
   in ROSE.  This sort of analysis should ultimatly be handeled by
   OpenAnalysis, but for now it's much easier to just do it in ROSE directly.
   The analysis works by performing a top down AST traversal (with the
   LoopNestProcessor class), each time a for loop is encountered the loop is
   abstracted into a LoopAbstraction object. */


#include "Sage2OA.h"
#include <rose.h>
using namespace std;
using namespace OA;
using namespace Loop;

// various debugging flags can be set by the OA_DEBUG environmental variable.
// when one of the debugging flags is on it will output loops that fail
// the associated requirement.
bool loopDebug       = false;   // set with LoopStats. Turns on the flags below
bool debugBadInit    = false;   // set with LoopStats_BadInit
bool debugLowerBound = false;   // set with LoopStats_BadLowerBound
bool debugUpperBound = false;   // set with LoopStats_BadUpperBound
bool debugBadStep    = false;   // set with LoopStats_BadStep

class LoopAnalStatistics {
  public:
    enum Rejection {
        BAD_INIT = 0,
        BAD_LOWER_BOUND,
        BAD_UPPER_BOUND,
        BAD_STEP,
        REDEFINITION,
        INCLUDES_FUNCTION_CALL,
        INCLUDES_DEREF
    };

    static const char *rejectionStr[];

    LoopAnalStatistics() :
        mnAccepted(0),
        mnRejected_BadLowerBound(0),
        mnRejected_BadUpperBound(0),
        mnRejected_BadStep(0),
        mnForLoops(0),
        mnWhileLoops(0),
        mnDoWhileLoops(0)
    { }

    void incrementAcceptedCounter() { mnAccepted++; }
    void encounteredForLoop() { mnForLoops++; }
    void encounteredWhileLoop() { mnWhileLoops++; }
    void encounteredDoWhileLoop() { mnDoWhileLoops++; }

    void incrementRejectedCounter(Rejection reason) {
        switch(reason) {
            case BAD_INIT:        mnRejected_BadInit++;       break;
            case BAD_LOWER_BOUND: mnRejected_BadLowerBound++; break;
            case BAD_UPPER_BOUND: mnRejected_BadUpperBound++; break;
            case BAD_STEP:        mnRejected_BadStep++;       break;
            case REDEFINITION:    mnRejected_Redefinition++;  break;
            case INCLUDES_FUNCTION_CALL: mnRejected_FunctionCall++; break;
            case INCLUDES_DEREF:         mnRejected_Deref++; break;
        };

        if(loopDebug) {
            cerr << "Loop was rejected for: "
                 << rejectionStr[reason] << endl;
        }
    }

    int getNumForLoops() { return mnForLoops; }
    int getNumWhileLoops() { return mnWhileLoops; }
    int getNumDoWhileLoops() { return mnDoWhileLoops; }
    int getNumAccepted() { return mnAccepted; }
    int getNumBadInit() { return mnRejected_BadInit; }
    int getNumBadLowerBound() { return mnRejected_BadLowerBound; }
    int getNumBadUpperBound() { return mnRejected_BadUpperBound; }
    int getNumBadStep() { return mnRejected_BadStep; }
    int getNumRedfinition() { return mnRejected_Redefinition; }
    int getNumLoopsWithFunctionCall() { return mnRejected_FunctionCall; }
    int getNumLoopsWithDeref() { return mnRejected_Deref; }

    int getNumRejected() {
        return getNumBadInit() +
               getNumBadLowerBound() +
               getNumBadUpperBound() +
               getNumBadStep() +
               getNumRedfinition() +
               getNumLoopsWithFunctionCall() +
               getNumLoopsWithDeref();
    }

    void output() {
        cerr << "Number of for loops:             " << getNumForLoops()
           << "\nNumber of while loops:           " << getNumWhileLoops()
           << "\nNumber of do while loops:        " << getNumDoWhileLoops()
           << "\nNumber of loops accepted:        " << getNumAccepted()
           << "\nNumber of loops rejected:        " << getNumRejected()
           << "\n  Rejected for bad initi:        " << getNumBadInit()
           << "\n  Rejected for bad lower bound:  " << getNumBadLowerBound()
           << "\n  Rejected for bad upper bound:  " << getNumBadUpperBound()
           << "\n  Rejected for bad step:         " << getNumBadStep()
           << "\n  Rejected for ivar redfinition: " << getNumRedfinition()
           << "\n  Rejected for function call:    "
           << getNumLoopsWithFunctionCall()
           << "\n  Rejected for pointer deref:    " << getNumLoopsWithDeref()
           << endl;
    }

  private:
    int mnAccepted;
    int mnRejected_BadInit;
    int mnRejected_BadLowerBound;
    int mnRejected_BadUpperBound;
    int mnRejected_BadStep;
    int mnRejected_Redefinition;
    int mnRejected_FunctionCall;
    int mnRejected_Deref;
    int mnForLoops;
    int mnWhileLoops;
    int mnDoWhileLoops;
};

const char *LoopAnalStatistics::rejectionStr[] = {
    "bad initialization of index variable.",
    "invalid lower bound statement.",
    "invalid upper bound statement.",
    "invalid step statement.",
    "redefinition of index variable.",
    "includes function call.",
    "includes pointer dereference."
};



static LoopAnalStatistics gStatistics;


/*! Inherited attribute for a ROSE AST traversal.  This attribute will
    keep track of what, and how many loops have been encountered, while
    performing a top-down traversal. */
class LoopNestAttribute {
  public:
    LoopNestAttribute();
    LoopNestAttribute(const LoopNestAttribute &orig);

    /*! Add a new loop to this attribute.  Every time the traversal visits
        a loop it should be added, this running list of loops will keeps track
        of what loops any given node in the AST is nested under. */
    void addNestedLoop(OA_ptr<LoopAbstraction> loop);

    /*! Returns the last loop added to the attribute.  This is usually the
        parent of a new loop being constructed.  If the list is empty
        NULL is returned */
    OA_ptr<LoopAbstraction> getLastLoop() const {
        OA_ptr<LoopAbstraction> ret;

        if(!mNestedLoops.empty()) { 
            ret = mNestedLoops.back();
        }

        return ret;
    }

  private:
    list<OA_ptr<LoopAbstraction> > mNestedLoops;
};

/*! Analysis to determine the loop nesting structure */
class LoopNestProcessor : public AstTopDownProcessing<LoopNestAttribute>
{
  public:
    LoopNestProcessor(
        OA_ptr<list<OA_ptr<LoopAbstraction> > > results,
        SageIRInterface &ir,
        ProcHandle proc)
        :
        mResults(results),
        mIR(ir),
        mProc(proc)
        { }

    /*! Called as the traversal visits nodes in the AST, the attribute
        calculated for the parent node is passed by the traversal as an
        argument. */
    LoopNestAttribute evaluateInheritedAttribute(
        SgNode *astNode,
        LoopNestAttribute attrib);

  private:
    /*! Given a node to a for statement in a SAGE AST construct a loop
        abstraction object if possible.  If no such object can be
        constructed return NULL. Abstractions are prevented from being
        built when they don't fall neatly into the format:
        'for(int idxVariable = lowerBound; idxVariable < upperBound;
             idxVariabl++);' */
    OA_ptr<LoopAbstraction> buildLoopAbstraction(
        SgNode *forStatementNode,
        LoopNestAttribute const &attrib);

    /* Given a SAGE for-statement initialization node, extract the named
       location where the loop's index variable resides. If an index
       variable can not be determined return NULL and set error to true. */
    OA_ptr<NamedLoc> extractIndexVariable(
        SgForInitStatement *forStatementNode,
        bool *error,
        bool *isCStyleLoop);

    /* Extract the index variable in a C-style loop. */
    OA_ptr<NamedLoc> extractIndexVariableInCStyleLoop(
        SgForInitStatement *forStatementNode,
        bool *error);

    /* Extract the index variable in a C++ style loop. */
    OA_ptr<NamedLoc> extractIndexVariableInCPPStyleLoop(
        SgForInitStatement *forStatementNode,
        bool *error);

    /* Determine the lower bound specified in a well-formatted for statement */
    int extractLowerBound(
        OA_ptr<NamedLoc> indexVariable,
        SgForInitStatement *init,
        bool *error,
        bool isCStyleLoop);

    /* Extract the lower bound a C-style loop. */
    int extractLowerBoundInCStyleLoop(
        OA_ptr<NamedLoc> indexVariable,
        SgForInitStatement *init,
        bool *error);

    /* Extract the lower bound in a C++ style loop. */
    int extractLowerBoundInCPPStyleLoop(
        OA_ptr<NamedLoc> indexVariable,
        SgForInitStatement *init,
        bool *error);

    /* Determine the upper bound specified in a well-formatted for statement */
    int extractUpperBound(
        OA_ptr<NamedLoc> indexVariable,
        SgStatement *condition,
        bool *error);

    /* Determine the loop step */
    int extractLoopStep(
        OA_ptr<NamedLoc> indexVariable,
        SgExpression *stepExpr,
        bool *error);

    /* Given a node to a variable reference and an OA location abstraction
       to an index variable determine if they refer to the same thing */
    bool isIndexVariable(SgVarRefExp *varRef, OA_ptr<NamedLoc> idxVar);

    /* Given a statement handle for a loop extract the last statement
       the loop contains.  If the last statement is a loop recurse. */
    StmtHandle extractLastLoopStmt(StmtHandle stmt);

    // Member vars:
    OA_ptr<list<OA_ptr<LoopAbstraction> > > mResults;
    SageIRInterface &mIR;
    ProcHandle mProc;
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
            mResults->push_back(loop);
            attrib.addNestedLoop(loop);
        }
    }

    return attrib;
}


OA_ptr<NamedLoc> LoopNestProcessor::extractIndexVariable(
    SgForInitStatement *init, bool *error, bool *isCStyleLoop) 
{
    OA_ptr<NamedLoc> ret;

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


OA_ptr<NamedLoc> LoopNestProcessor::extractIndexVariableInCStyleLoop(
    SgForInitStatement *init, bool *error)
{
    OA_ptr<NamedLoc> ret;

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
    ret = (mIR.getLocation(mProc, hSym)).convert<NamedLoc>();

    return ret;
}


OA_ptr<NamedLoc> LoopNestProcessor::extractIndexVariableInCPPStyleLoop(
    SgForInitStatement *init, bool *error)
{
    OA_ptr<NamedLoc> ret;

    // check to see if the init statement has a single variable
    // assignment, within the assignment get the variable reference
    list<SgNode*> declarations = 
        NodeQuery::querySubTree(init, V_SgVariableDeclaration);
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
    ret = (mIR.getLocation(mProc, hSym)).convert<NamedLoc>();

    return ret;
}

int LoopNestProcessor::extractLowerBound(
    OA_ptr<NamedLoc> indexVariable,
    SgForInitStatement *init,
    bool *error,
    bool isCStyleLoop)
{
    OA_ptr<NamedLoc> ret;

    // determine how to extract the lower bound based on what type of loop
    // is being analyzed
    if(isCStyleLoop) {
        return extractLowerBoundInCStyleLoop(indexVariable, init, error);
    } else {
        return extractLowerBoundInCPPStyleLoop(indexVariable, init, error);
    }
}


int LoopNestProcessor::extractLowerBoundInCStyleLoop(
    OA_ptr<NamedLoc> indexVariable,
    SgForInitStatement *init,
    bool *error)
{
    OA_ptr<NamedLoc> ret;

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
    OA_ptr<NamedLoc> indexVariable,
    SgForInitStatement *init,
    bool *error)
{
    OA_ptr<NamedLoc> ret;

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
    OA_ptr<NamedLoc> indexVariable,
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
    OA_ptr<NamedLoc> indexVariable,
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
    OA_ptr<NamedLoc> idxVar)
{
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
    OA_ptr<NamedLoc> indexVariable;
    OA_ptr<LoopAbstraction> loopAbstraction;
    bool rejected = false;
    bool error    = false;
    bool isCStyleLoop = false;
    int lowerBound, upperBound, step;


    if(loopDebug) {
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
    list<SgNode*> assignments = 
        NodeQuery::querySubTree(
            forStatement->get_loop_body(), &queryAssignments);
    if(!assignments.empty()) {
        // check to see if the LHS of any of the assignments is the
        // index variable
        for(list<SgNode*>::iterator i = assignments.begin();
            i != assignments.end(); i++)
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
    list<SgNode*> functions =
        NodeQuery::querySubTree(
            forStatement->get_loop_body(), V_SgFunctionCallExp);
    if(!functions.empty()) {
        rejected = true;
        gStatistics.incrementRejectedCounter(
            LoopAnalStatistics::INCLUDES_FUNCTION_CALL);
        return loopAbstraction;
    }

    // check to assure that the loop contains no dereferences
    list<SgNode*> pointers =
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
    OA_DEBUG_CTRL_MACRO("LoopStats:ALL", loopDebug);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadInit:ALL", debugBadInit);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadLowerBound:ALL",
                        debugLowerBound);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadUpperBound:ALL",
                        debugUpperBound);
    OA_DEBUG_CTRL_MACRO("LoopStats:LoopStats_BadStep:ALL", debugBadStep);

    loopDebug = true;
    if(loopDebug) {
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
    if(loopDebug) {
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

