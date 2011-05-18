/*
 * LoopIRInterface.h
 *
 *  Created on: May 3, 2011
 *      Author: snarayan
 */

#ifndef _LOOPINTERFACE_H
#define _LOOPINTERFACE_H

#include "Sage2OA.h"
#include <rose.h>
using namespace std;
using namespace OA;
using namespace Loop;


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
    void incrementRejectedCounter(Rejection reason);
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
    /*! Return the a list of LoopAbstraction elements for valid loops */

    OA_ptr<list<OA_ptr<LoopAbstraction> > > getResults()
		{ return mResults;}
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
    OA_ptr<MemRefExpr> extractIndexVariable(
        SgForInitStatement *forStatementNode,
        bool *error,
        bool *isCStyleLoop);

    /* Extract the index variable in a C-style loop. */
    OA_ptr<MemRefExpr> extractIndexVariableInCStyleLoop(
        SgForInitStatement *forStatementNode,
        bool *error);

    /* Extract the index variable in a C++ style loop. */
    OA_ptr<MemRefExpr> extractIndexVariableInCPPStyleLoop(
        SgForInitStatement *forStatementNode,
        bool *error);

    /* Determine the lower bound specified in a well-formatted for statement */
    int extractLowerBound(
        OA_ptr<MemRefExpr> indexVariable,
        SgForInitStatement *init,
        bool *error,
        bool isCStyleLoop);

    /* Extract the lower bound a C-style loop. */
    int extractLowerBoundInCStyleLoop(
        OA_ptr<MemRefExpr> indexVariable,
        SgForInitStatement *init,
        bool *error);

    /* Extract the lower bound in a C++ style loop. */
    int extractLowerBoundInCPPStyleLoop(
        OA_ptr<MemRefExpr> indexVariable,
        SgForInitStatement *init,
        bool *error);

    /* Determine the upper bound specified in a well-formatted for statement */
    int extractUpperBound(
        OA_ptr<MemRefExpr> indexVariable,
        SgStatement *condition,
        bool *error);

    /* Determine the loop step */
    int extractLoopStep(
        OA_ptr<MemRefExpr> indexVariable,
        SgExpression *stepExpr,
        bool *error);

    /* Given a node to a variable reference and an OA location abstraction
       to an index variable determine if they refer to the same thing */
    bool isIndexVariable(SgVarRefExp *varRef, OA_ptr<MemRefExpr> idxVar);

    /* Given a statement handle for a loop extract the last statement
       the loop contains.  If the last statement is a loop recurse. */
    StmtHandle extractLastLoopStmt(StmtHandle stmt);

    // Member vars:
    OA_ptr<list<OA_ptr<LoopAbstraction> > > mResults;
    SageIRInterface &mIR;
    ProcHandle mProc;
};
#endif
