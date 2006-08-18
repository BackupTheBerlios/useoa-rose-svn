
#include "common.h"

/** \brief isMethodCall returns true if the invoked procedure is
 *         a method (i.e., a constructor, a destructor, or a non-static
 *         method).
 *  \param functionCall A Sage node representing a function call expression.
 *  \param isDotExp  On output, a boolean indicating whether the
 *                   receiver of a method invocation is an object 
 *                   (isDotExp = true) or a pointer (isDotExp = false).
 *  \returns  Boolean indicating whether the invoked procedure is a method.
 *
 *  Be Careful!  For the purposes of isMethodCall a method is anything
 *  which has or creates a receiver/"this."  That is a constructor
 *  (which creates a this and may modify/initialization it), a 
 *  destructor, or a non-static method.  Each of these is passed
 *  an implicit "this" parameter.  A static method may only
 *  access static member variables, therefore we need not pass an 
 *  implicit "this" (which is convenient, since we don't have one).
 */
bool
isMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp)
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
        {
            isMethod = false;
            break;
        }
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
	    std::cerr << "Was not expecting an " 
                      << expression->sage_class_name() 
                      << std::endl
                      << "in a function call." 
                      << std::endl;
            ROSE_ABORT();
        }
    }
    return isMethod;
}

/*!  
   Returns a SgPointerDerefExp if the function call is made
   through a pointer and NULL otherwise.
*/
SgPointerDerefExp *
isFunctionPointer(SgFunctionCallExp *functionCall) 
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

/** \brief  Return the SgFunctionDeclaration invoked by a function call.
 *  \param  functionCall  A SgFunctionCallExp representing a function
 *                        call site.
 *  \returns  The SgFunctionDeclaration representing the function or
 *            method invoked at the call site.
 *
 *  This function does not perform any analysis, therefore it can not
 *  determine which function or method is invoked through a pointer.
 *  If functionCall is a pointer dereference expression, this function
 *  will abort.
 */
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
    case V_SgArrowExp:
        {
            SgBinaryOp *binaryOp = isSgBinaryOp(expression);
            ROSE_ASSERT(binaryOp != NULL);

            SgExpression *rhs = binaryOp->get_rhs_operand();
            ROSE_ASSERT(rhs != NULL);

            SgMemberFunctionRefExp *memberFunctionRefExp =
	        isSgMemberFunctionRefExp(rhs);
            ROSE_ASSERT(memberFunctionRefExp != NULL);

            funcDec = memberFunctionRefExp->get_symbol_i()->get_declaration();
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

/*!
   Verify that a node intended to be used as a call handle
   is one of the expected node types for a call handle.
*/
void verifyCallHandleNodeType(SgNode *node)
{
    switch(node->variantT()) {
    case V_SgFunctionCallExp:
    case V_SgConstructorInitializer:
    case V_SgDeleteExp:
        {
            // These are the expected call handle node types.
            break;
        }
    default:
        {
            std::cerr << "verifyCallHandleNodeType:  was not expecting a "
                      << node->sage_class_name()
                      << std::endl;
            ROSE_ABORT();
        }
    }
}

/**
 *  \brief Return a list of the types of the formal parameters involved
 *         in a function, method, or constructor invocation.
 *  \param node  a SgNode representing a function/method (SgFunctionCallExp),
 *               a constructor (SgConstructorInitializer) invocation,
 *               or a destruction (SgDeleteExp) invocation.
 *               i.e., node should have a corresponding call handle.
 *  \returns  a list of the types of the formal parameters of the
 *            invoked function/method/constructor.
 *
 *  NB:  This does not return the type of the implicit formal
 *       parameter corresponding to the 'this' pointer.  Contrast this with 
 *       getCallsiteParams which folds the actual 
 *       corresponding to the 'this' pointer in as the first argument.
 */
SgTypePtrList &
getFormalTypes(SgNode *node)
{
    ROSE_ASSERT(node != NULL);

    SgFunctionType *functionType = NULL;

    std::list<SgType *> typeList;

    verifyCallHandleNodeType(node);

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

                SgPointerType *pointerType = isSgPointerType(type);
                ROSE_ASSERT(pointerType != NULL);

                functionType = isSgFunctionType(pointerType->get_base_type());
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
    case V_SgDeleteExp:
        {
            SgDeleteExp *deleteExp = isSgDeleteExp(node);
            ROSE_ASSERT(deleteExp != NULL);

            SgExpression *var = deleteExp->get_variable(); 
            ROSE_ASSERT(var != NULL); 

            SgType *varType = var->get_type(); 
            ROSE_ASSERT(varType != NULL); 

            SgPointerType *ptrType = isSgPointerType(varType); 
            SgClassType *classType = NULL; 
            if ( ptrType ) { 
                classType = isSgClassType(ptrType->get_base_type()); 
            } else { 
                classType = isSgClassType(varType); 
            } 
            ROSE_ASSERT(classType != NULL); 

            SgClassDeclaration *classDeclaration =  
                isSgClassDeclaration(classType->get_declaration()); 
            ROSE_ASSERT(classDeclaration != NULL); 

            SgClassDefinition *classDefn  =  
                classDeclaration->get_definition();  
            ROSE_ASSERT(classDefn != NULL); 

            // This assumes that the AST has been normalized. 
            // Without it, the destructor declaration may 
            // not exist if it is compiler generated. 
            SgMemberFunctionDeclaration *funcDecl = 
                lookupDestructorInClass(classDefn); 
            ROSE_ASSERT(funcDecl != NULL); 

            functionType = funcDecl->get_type();
            
            break;
        }
    default:
        {
            ROSE_ABORT();
 
            break;
        }
    }

    ROSE_ASSERT(functionType != NULL);
    return functionType->get_arguments();
}

/** \brief Returns true if the method is a destructor.
 *  \param memberFunctionDeclaration  A method declaration.
 *  \returns  Boolean indicating whether methodFunctionDeclaration is
 *            a destructor.
 */
static bool 
isDestructor(SgMemberFunctionDeclaration *memberFunctionDeclaration)
{
    ROSE_ASSERT(memberFunctionDeclaration != NULL);

    SgSpecialFunctionModifier &specialFunctionModifier =
        memberFunctionDeclaration->get_specialFunctionModifier();

    // Determine whether method is a destructor.
    return specialFunctionModifier.isDestructor();
}

/** \brief   Returns a destructor method declaration within a class 
 *           if it exists.
 *  \param   classDefinition  a SgNode representing a class definition.
 *  \returns A SgMemberFunctionDeclaration from classDefinition that
 *           representing a destructor, or NULL if it is not
 *           defined.
 */
SgMemberFunctionDeclaration *
lookupDestructorInClass(SgClassDefinition *classDefinition)
{
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
                ROSE_ASSERT(functionDeclaration != NULL);

                if ( isDestructor(functionDeclaration) ) {
                    return functionDeclaration;
                }

                break;
            }
        default:
            {
                break;
            }
        }
    }
    return NULL;
}

/**  \brief  Return the lhs of a constructor initializer.
 *   \param  ctorInitializer  A SgConstructorInitializer representing
 *                            the invocation of a constructor.
 *   \returns An expression representing the object created by
 *            the constructor.
 *
 *   In the following examples, we return the lhs:
 *       Foo f;
 *       Foo lhs(f);   // copy constructor.
 *       Foo lhs;      // default constructor.
 *       Foo lhs(1);   // some other constructor;
 *       Foo *lhs = new Foo;
 *
 *   In the following examples, the object constructed is
 *   anonymous, so we return that anonymous expression-- in these
 *   examples, new Foo.
 *       return (new Foo);
 *       bar(new Foo);
 *       (new Foo)->methodCall();
 *
 *   We don't really return an expression.  Notice in the first
 *   few examples, the only thing we can return is a SgInitializedName.
 *
 */
static SgNode *
getConstructorInitializerLhs(SgConstructorInitializer *ctorInitializer)
{
    ROSE_ASSERT( ctorInitializer != NULL );
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
  
    while ( ( parent != NULL ) && ( !isSgGlobal(parent) ) ) {
    
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

    if ( lhs == NULL ) {
        // We were unable to find a lhs.  Hopefully this is
        // one of the anonymous memory object cases named above.
        // In that case, our parent had better be a SgNewExp.
        lhs = isSgNewExp(ctorInitializer->get_parent());
    }
    ROSE_ASSERT(lhs != NULL);

    return lhs;
}

/**  \brief  Return the actual arguments from a method, which 
 *           may include a constructor or destructor, or function
 *           invocation.
 *   \param  node  A Sage node representing a method or function
 *                 invocation-- i.e., a SgFunctionCallExp,
 *                 a SgConstructorInitializer, or a SgDeleteExp.
 *   \param  actualArgs  On output, holds the expressions passed
 *                       as actual arguments to the invocation.
 *
 *   NB:  If this is a non-static method invocation, including
 *        a constructor or destructor call, then the first
 *        actual is the receiver.  Yes, the receiver, i.e., the
 *        object that is being allocated, is returned for a
 *        constructor.  
 *
 *   Notice that the receiver may not be a named variable.
 *   e.g., 
 *      (new D)->foo();
 *      bar(new D);
 *      return (new D);
 *   In each of these cases we return the expression 'new D'
 *   as the actual for the implicit receiver.
 */  
void getActuals(SgNode *node, std::list<SgNode *> &actuals) 
{ 
    ROSE_ASSERT(node != NULL);

    switch(node->variantT()) {

    case V_SgFunctionCallExp:
        {
            SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
            ROSE_ASSERT(functionCallExp != NULL);

            // If this is a method call, fold the object upon which the
            // method is invoked into the argument list as the first argument.
            SgExpression *function = functionCallExp->get_function();
            ROSE_ASSERT(function != NULL);

            if ( isSgDotExp(function) || isSgArrowExp(function) ) {
                SgBinaryOp *dotOrArrow = isSgBinaryOp(function);
                ROSE_ASSERT(dotOrArrow != NULL);

                SgExpression *lhs = dotOrArrow->get_lhs_operand();
                ROSE_ASSERT(lhs != NULL);

                actuals.push_back(lhs);
            }

            // Now, get the non-implicit receiver actuals and
            // push them on the result list.
            SgExprListExp* exprListExp = functionCallExp->get_args();
            ROSE_ASSERT (exprListExp != NULL);  

            SgExpressionPtrList & actualArgs =  
                exprListExp->get_expressions();  

            for(SgExpressionPtrList::iterator actualIt = actualArgs.begin(); 
                actualIt != actualArgs.end(); ++actualIt) { 
 
                SgExpression *actual = *actualIt;
                ROSE_ASSERT(actual != NULL);

                actuals.push_back(actual);
	    }

            break;
        }        
    case V_SgConstructorInitializer:
        {
            SgConstructorInitializer *ctorInitializer =
                isSgConstructorInitializer(node);
            ROSE_ASSERT(ctorInitializer != NULL);

            // As above, this is a method, so fold the object upon which the
            // method is invoked into the argument list as the first argument.
            // This may seem a little strange since we may not consider
            // a constructor to be invoked on an object.
            SgNode *lhs = getConstructorInitializerLhs(ctorInitializer);
            ROSE_ASSERT(lhs != NULL);
  
            actuals.push_back(lhs);

            // Now, get the non-implicit receiver actuals and
            // push them on the result list.
            SgExprListExp* exprListExp = ctorInitializer->get_args();
            ROSE_ASSERT (exprListExp != NULL);  

            SgExpressionPtrList & actualArgs =  
                exprListExp->get_expressions();  

            for(SgExpressionPtrList::iterator actualIt = actualArgs.begin(); 
                actualIt != actualArgs.end(); ++actualIt) { 
 
                SgExpression *actual = *actualIt;
                ROSE_ASSERT(actual != NULL);

                actuals.push_back(actual);
	    }

            break;
        }
    case V_SgDeleteExp:
        {
            SgDeleteExp *deleteExp = isSgDeleteExp(node);
            ROSE_ASSERT(deleteExp != NULL);

            // The only actual passed to a destructor is
            // the receiver.
            SgExpression *var = deleteExp->get_variable(); 
            ROSE_ASSERT(var != NULL); 

            actuals.push_back(var);

            break;
        }
    default:
        {
            std::cerr << "Expected a method or function invocation to be"
                      << std::endl
                      << "represented by a SgFunctionCallExp, a SgDeleteExp,"
                      << std::endl
                      << "or a SgConstructorInitializer."
                      << std::endl
                      << "Instead got a: " << node->sage_class_name()
                      << std::endl;
            ROSE_ABORT();
            break;
        }
    }
}
