#include "ArrayReferenceVisitor.hpp"

static bool debug = false;

namespace OA {

ArrayReferenceVisitor::ArrayReferenceVisitor(
    DataDep::DataDepIRInterface *ir,
    OA_ptr<IdxExprAccess> memRefExp)
    :
    mIR(ir),
    mIdxExprAccess(memRefExp)
{
    // By default we set the memRefExp so that it has a coefficient of 1
    // and an offset of 0.  This is always the case if no OpNode is visited.
    // For example: A[i]
    mIdxExprAccess->setCoefficient(1);
    mIdxExprAccess->setOffset(0);
}

void ArrayReferenceVisitor::visitExprTreeBefore(ExprTree& e)
{
}

void ArrayReferenceVisitor::visitExprTreeAfter(ExprTree& e)
{
}

void ArrayReferenceVisitor::visitNode(ExprTree::Node&)
{
}

void ArrayReferenceVisitor::visitOpNode(ExprTree::OpNode& n)
{
    if(debug) { cout << "Visiting OpNode" << endl; }
    OA::OpHandle hOp = n.getHandle();
    OA::DataDep::OpType opType;
    opType = mIR->getOpType(hOp);

    // Iterate through all the children.  When we see an OpNode visit it,
    // when we see a constant value we set either the coefficient or
    // offset depending on whether this OpNode is for the multiply or
    // add operation
    OA::OA_ptr<OA::Tree::ChildNodesIterator> i = n.getChildNodesIterator();
    while(i->isValid()) {
        OA::OA_ptr<OA::ExprTree::Node> child =
            i->current().convert<ExprTree::Node>();

        if(child->isaOpNode()) {
            visitOpNode(*(child.convert<ExprTree::OpNode>()));
        }
        else if(child->isaConstValNode()) {
            // Get the value of this constant
            int val;
            OA::OA_ptr<OA::ExprTree::ConstValNode> valNode =
                i->current().convert<ExprTree::ConstValNode>();
            val = mIR->constValIntVal(valNode->getHandle());

            // Set the offset or coefficient depending on whether this
            // OpNode is for a multiply or add operation.  If it's for
            // a subtract operation negate the value.
            if(opType == DataDep::OP_MULTIPLY) {
                mIdxExprAccess->setCoefficient(val);
            }
            else if(opType == DataDep::OP_ADD) {
                mIdxExprAccess->setOffset(val);

            }
            else if(opType == DataDep::OP_SUBTRACT) {
                mIdxExprAccess->setOffset(-1 * val);
            }
        }
        (*i)++;
    }
}

void ArrayReferenceVisitor::visitCallNode(ExprTree::CallNode& n)
{
    if(debug) { cout << "Visiting Call" << endl; }
}

void ArrayReferenceVisitor::visitMemRefNode(ExprTree::MemRefNode& n)
{
    if(debug) { cout << "Visiting MemRef" << endl; }
}

void ArrayReferenceVisitor::visitConstSymNode(ExprTree::ConstSymNode& n)
{
    if(debug) { cout << "Visiting ConstSym" << endl; }
}

void ArrayReferenceVisitor::visitConstValNode(ExprTree::ConstValNode& n)
{
    // This node will be visited whenever the index expression has only a
    // constant value.  Otherwise the OpNode is visited instead.
    // For example: A[5]
    
    int val = mIR->constValIntVal(n.getHandle());
    mIdxExprAccess->setCoefficient(val);
}

} // end of OA namespace

