/*! \file
  
  \authors Andy Stone

  Copyright (c) 2006, Contributors <br>
  All rights reserved. <br>
  See ../../../Copyright.txt for details. <br>
*/

#ifndef ArrayReferenceVisitor_H_
#define ArrayReferenceVisitor_H_

#include <OpenAnalysis/ExprTree/ExprTreeVisitor.hpp>
#include <OpenAnalysis/DataDep/Interface.hpp>

namespace OA {

class ArrayReferenceVisitor : public ExprTreeVisitor {
public:
    ArrayReferenceVisitor(
        DataDep::DataDepIRInterface *ir,
        OA_ptr<IdxExprAccess> memRefExp);
    ~ArrayReferenceVisitor() {}

    void visitExprTreeBefore(ExprTree& e);
    void visitExprTreeAfter(ExprTree& e);


    //---------------------------------------
    // method for each ExprTree::Node subclass
    //---------------------------------------
    // default base class so that visitors can handle unknown
    // node sub-classes in a generic fashion
    void visitNode(ExprTree::Node&);

    void visitOpNode(ExprTree::OpNode& n);
    void visitCallNode(ExprTree::CallNode& n);
    void visitMemRefNode(ExprTree::MemRefNode& n);
    void visitConstSymNode(ExprTree::ConstSymNode& n);
    void visitConstValNode(ExprTree::ConstValNode& n);

private:
    DataDep::DataDepIRInterface *mIR;
    OA_ptr<IdxExprAccess> mIdxExprAccess;
};


} // end of OA namespace

#endif
