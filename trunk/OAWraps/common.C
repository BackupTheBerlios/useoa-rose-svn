
#include "common.h"

namespace UseOA {

/** \brief Strip the "const" keyword from a string.
 *  \param name  A string (intending to represent a formal parameter
 *               type).
 *  \returns  A string holding name stripped of the "const" prefix.
 *
 *  This was copied from
 *  .../ROSE/src/midend/astUtil/astInterface/AstInterface.C
 *
 *  To do:  move this to a generally-accessible utilities file
 *          or make it accessible from AstInterface (by adding
 *          its declaration to AstInterface.h).
 */
static std::string stripParameterType( const std::string& name)
{
    char *const_start = (char *) strstr( name.c_str(), "const");
    std::string r = (const_start == 0)? name : std::string(const_start + 5);
    int end = r.size()-1;
    if (r[end] == '&') {
        r[end] = ' ';
    }
    std::string result = "";
    for (unsigned int i = 0; i < r.size(); ++i) {
        if (r[i] != ' ') {
            result.push_back(r[i]);
        }
    }
    return result;
}

/** \brief Return a type's name and size.
 *  \param t  A Sage type.
 *  \param tname  On output, holds the string name of the type.
 *  \param stripname  On output, holds the string name of the type,
 *                    stripped of any "const" prefix.
 *  \param size  On output, holds the size?  I don't know what
 *               this size corresponds to.  Certainly not the size
 *               of the type (which needn't be a word) or string.
 *
 *  This was copied from
 *  .../ROSE/src/midend/astUtil/astInterface/AstInterface.C
 *
 *  To do:  move this to a generally-accessible utilities file
 *          or make it accessible from AstInterface (by adding
 *          its declaration to AstInterface.h).
 */
void getTypeInfo(SgType *t, std::string *tname,
		 std::string* stripname, int* size)
{
    std::string r1 = get_type_name(t);
    std::string result = "";
    for (unsigned int i = 0; i < r1.size(); ++i) {
        if (r1[i] != ' ') {
            result.push_back(r1[i]);
        }
    }
    if (tname != 0) {
        *tname = result;
    }
    if (stripname != 0) {
        *stripname = stripParameterType(result);
    }
    if (size != 0) {
        *size = 4;
    }
}

/** \brief isNonStaticMethodCall returns true if the invoked procedure is
 *         a method (i.e., a constructor, a destructor, or a non-static
 *         method).
 *  \param functionCall A Sage node representing a function call expression.
 *  \param isDotExp  On output, a boolean indicating whether the
 *                   receiver of a method invocation is an object
 *                   (isDotExp = true) or a pointer (isDotExp = false).
 *  \returns  Boolean indicating whether the invoked procedure is a method.
 *
 *  Be Careful!  For the purposes of isNonStaticMethodCall a method is anything
 *  which has or creates a receiver/"this."  That is a constructor
 *  (which creates a this and may modify/initialization it), a
 *  destructor, or a non-static method.  Each of these is passed
 *  an implicit "this" parameter.  A static method may only
 *  access static member variables, therefore we need not pass an
 *  implicit "this" (which is convenient, since we don't have one).
 */
bool
isNonStaticMethodCall(SgFunctionCallExp *functionCall, bool &isDotExp)
{
    ROSE_ASSERT(functionCall != NULL);

    SgExpression *expression = functionCall->get_function();
    ROSE_ASSERT(expression != NULL);

    bool isMethod = false;
    isDotExp = false;

    switch(expression->variantT()) {
    case V_SgDotExp:
    case V_SgDotStarOp:
        {
            isMethod = true;

            SgBinaryOp *dotExp = isSgBinaryOp(expression);
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
    case V_SgArrowStarOp:
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
        {
            // These are the expected call handle node types.
            break;
        }
    case V_SgNewExp:
        {
            SgNewExp *newExp = isSgNewExp(node);
            ROSE_ASSERT(newExp != NULL);
            if ( !isPlacementNew(newExp) ) {
                std::cerr << "verifyCallHandleNodeType:  was not expecting a "
                          << "non-placement new expression" << std::endl;
                ROSE_ABORT();
            }
            break;
        }
    case V_SgDeleteExp:
        {
            // SgDeleteExp is used to represent the invocation of
            // a delete operator, not a destructor.
            SgDeleteExp *deleteExp = isSgDeleteExp(node);
            SgFunctionDeclaration *deleteDecl =
                isPlacementDelete(deleteExp);
            if ( !deleteDecl ) {
                std::cerr << "verifyCallHandleNodeType:  was not expecting a "
                          << "non-placement delete expression" << std::endl;
                ROSE_ABORT();
            }
            break;
        }
    case V_Sg_File_Info:
        {
            // (BW 5/3/07)  Needed to add this since we now represent
            // constructor invocations via the Sg_File_Info of the
            // SgConstructorInitializer, rather than the SgConstructorInitializer.

            // We now use the Sg_File_Info of a SgDeleteExp to
            // model destructor invocation.
            SgNode *parent = node->get_parent();
            if ( isSgConstructorInitializer(parent) || isSgDeleteExp(parent) ) {
              break;
            }
            // fall through
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

/*!
   Verify that a node intended to be used as a stmt handle
   is one of the expected node types for a stmt handle.
*/
void verifyStmtHandleNodeType(SgNode *node)
{
    if ( isSgStatement(node) ) {
        return;
    }
    switch(node->variantT()) {
    case V_SgNewExp:
        {
            // These are the expected stmt handle node types.
            break;
        }
    default:
        {
            std::cerr << "verifyStmtHandleNodeType:  was not expecting a "
                      << node->sage_class_name()
                      << std::endl;
            ROSE_ABORT();
        }
    }
}

/*!
   Verify that a node intended to be used as a sym handle
   is one of the expected node types for a sym handle.
*/
void verifySymHandleNodeType(SgNode *node)
{
    switch(node->variantT()) {
    case V_SgClassDefinition:
        {
            // We use a class definition within
            // createImplicitPtrAssignPairsForVirtualMethods as the
            // base of the rhs in the implicit ptr assign pair
            // for the virtual table model.
            // These are the expected stmt handle node types.
            break;
        }
    case V_SgInitializedName:
        {
            break;
        }
    default:
        {
            std::cerr << "verifySymHandleNodeType:  was not expecting a "
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
void getFormalTypes(SgNode *node, std::vector<SgType *> &formalTypes)
{
    ROSE_ASSERT(node != NULL);

    SgFunctionType *functionType = NULL;

    verifyCallHandleNodeType(node);

    switch(node->variantT()) {
    case V_SgFunctionCallExp:
        {
            SgFunctionCallExp *functionCallExp = isSgFunctionCallExp(node);
            ROSE_ASSERT(functionCallExp != NULL);

            SgExpression *expr = functionCallExp->get_function();
            ROSE_ASSERT(expr != NULL);

            SgBinaryOp *binaryOp = isSgBinaryOp(expr);

            SgPointerDerefExp *pointerDerefExp =
                isFunctionPointer(functionCallExp);

            if ( pointerDerefExp ) {
                SgType *type = pointerDerefExp->get_type();
                ROSE_ASSERT(type != NULL);

                functionType = isSgFunctionType(type);
                if ( functionType == NULL ) {
                    SgPointerType *pointerType = isSgPointerType(type);
                    ROSE_ASSERT(pointerType != NULL);

                    functionType =
                        isSgFunctionType(pointerType->get_base_type());
                }

                ROSE_ASSERT(functionType != NULL);
            } else if ( isSgArrowStarOp(binaryOp) ||
                        isSgDotStarOp(binaryOp) ) {

                SgExpression *rhs = binaryOp->get_rhs_operand();
                ROSE_ASSERT(rhs != NULL);

                SgPointerMemberType *rhsType =
                    isSgPointerMemberType(rhs->get_type());
                ROSE_ASSERT(rhsType != NULL);

                functionType = isSgMemberFunctionType(rhsType->get_base_type());
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
    case V_SgNewExp:
        {
            SgNewExp *newExp = isSgNewExp(node);
            ROSE_ASSERT(newExp != NULL);

            if ( !isPlacementNew(newExp) ) {
                std::cerr << "getFormalTypes was not expecting a "
                          << "non-placement new expression" << std::endl;
                ROSE_ABORT();
            }

            SgFunctionDeclaration *functionDeclaration =
                newExp->get_newOperatorDeclaration();
            ROSE_ASSERT(functionDeclaration != NULL);

            functionType = functionDeclaration->get_type();
            ROSE_ASSERT(functionType != NULL);

            SgTypePtrList &typePtrList = functionType->get_arguments();

            // The first formal to operator new is size_t.  However, the
            // corresponding actual is not passed explicitly.  Therefore,
            // and since the actual is not a pointer and does not induce
            // side effects, we will not include the formal in the list of
            // formals and their types.
	    SgTypePtrList::iterator it = typePtrList.begin();
            ++it;
            for(; it != typePtrList.end(); ++it) {
                SgType *type = *it;
                ROSE_ASSERT(type != NULL);
                formalTypes.push_back(type);
            }

            // return!  Not break;
            return;
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
    case V_Sg_File_Info:
        {
            Sg_File_Info *fileInfo = isSg_File_Info(node);
            ROSE_ASSERT(fileInfo != NULL);

            // Invocations of destructors are represented by
            // the Sg_File_Info of a SgDeleteExp.
            // Note that invocations of constructors are represented
            // by the Sg_File_Info of a SgConstructorInitializer,
            // with the latter being passed to this function.

            SgDeleteExp *deleteExp = isSgDeleteExp(node->get_parent());
            if ( deleteExp != NULL ) {

                SgExpression *var = deleteExp->get_variable();
                ROSE_ASSERT(var != NULL);

                SgType *varType = var->get_type();
                ROSE_ASSERT(varType != NULL);

                // Need getBaseType to look through typedefs.
                SgPointerType *ptrType = isSgPointerType(lookThruReferenceType(varType));
                SgClassType *classType = NULL;
                if ( ptrType ) {
                    classType = isSgClassType(lookThruReferenceType(ptrType->get_base_type()));
                } else {
                    classType = isSgClassType(lookThruReferenceType(varType));
                }
                if ( classType == NULL ) {
		    std::cout << "varType = " << varType->unparseToString() << std::endl;
		    std::cout << "class_name = " << varType->sage_class_name() << std::endl;

                }
                ROSE_ASSERT(classType != NULL);

                SgClassDeclaration *classDeclaration =
                    isSgClassDeclaration(classType->get_declaration());
                ROSE_ASSERT(classDeclaration != NULL);

                classDeclaration =
                    isSgClassDeclaration(getDefiningDeclaration(classDeclaration));
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
                ROSE_ASSERT(functionType != NULL);
            }
            break;
	}
    case V_SgDeleteExp:
        {
            // SgDeleteExp represents invocations of operator delete,
	    // not the destructor.
            // SgDeleteExp will eventually represent destructor invocations.
            SgDeleteExp *deleteExp = isSgDeleteExp(node);
            ROSE_ASSERT(deleteExp != NULL);

            SgFunctionDeclaration *deleteDecl =
                isPlacementDelete(deleteExp);
            if ( deleteDecl ) {
                functionType = deleteDecl->get_type();
                ROSE_ASSERT(functionType != NULL);
            } else {
		std::cerr << "getFormalTypes was not expecting a "
                          << "non-placement delete" << std::endl;
                ROSE_ABORT();
            }

            break;
        }
    default:
        {
            ROSE_ABORT();

            break;
        }
    }

    ROSE_ASSERT(functionType != NULL);
    SgTypePtrList &typePtrList = functionType->get_arguments();


    for(SgTypePtrList::iterator it = typePtrList.begin(); it != typePtrList.end(); ++it) {
        SgType *type = *it;
        ROSE_ASSERT(type != NULL);
        formalTypes.push_back(type);
    }
}

/** \brief Returns true if the method is a destructor.
 *  \param memberFunctionDeclaration  A method declaration.
 *  \returns  Boolean indicating whether methodFunctionDeclaration is
 *            a destructor.
 */
bool isDestructor(SgMemberFunctionDeclaration *memberFunctionDeclaration)
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


/**  DEPRECATED MMS 9/7/06
 *   \brief  Return the lhs of a constructor initializer.
 *   \param  ctorInitializer  A SgConstructorInitializer representing
 *                            the invocation of a constructor.
 *   \returns An expression used to represent the this (implicit or
 *            explicit) of a constructor invocation.
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
 *   For cases involving the invocation of a baseclass constructor:
 *
 *       class Foo : public Bar { Foo(Foo &f) : Bar(f) { } };
 *
 *   we represent the actual implicit 'this' via a SgExprListExp.
 *
 *   We don't really return an expression.  Notice in the first
 *   few examples, the only thing we can return is a SgInitializedName.
 *
 */
SgNode *
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

    SgInitializedName *initName = isSgInitializedName(parent);
    if ( initName != NULL ) {
      // If the parent is a SgInitializedName whose name is the
      // same as the constructor being invoked, then that SgInitializedName
      // is not a variable, but a base class.
      if ( isObjectInitialization(ctorInitializer) ) {
          return initName;
      } else {
        // In this case, the implicit actual to the constructor
        // invocation is the 'this' param of the caller (constructor).
        // First, get the caller.  However, we want to return
        // something that may be used as the actual/MemRefHandle.
        // We use the SgExprListExp of the invoked constructor
        // for that purpose.
        SgExprListExp *args = ctorInitializer->get_args();
        ROSE_ASSERT(args != NULL);

        return args;
      }

    }

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
    case V_SgNewExp:
        {
            SgNewExp *newExp = isSgNewExp(node);
            ROSE_ASSERT(newExp != NULL);
            if ( !isPlacementNew(newExp) ) {
                std::cerr << "getActuals was not expecting a "
                          << "non-placement new expression" << std::endl;
                ROSE_ABORT();
            }

            SgExprListExp* exprListExp = newExp->get_placement_args();
            if ( exprListExp != NULL ) {

                SgExpressionPtrList & actualArgs =
                    exprListExp->get_expressions();

                for(SgExpressionPtrList::iterator actualIt = actualArgs.begin();
                    actualIt != actualArgs.end(); ++actualIt) {

                    SgExpression *actual = *actualIt;
                    ROSE_ASSERT(actual != NULL);

                    actuals.push_back(actual);
   	        }
            }
            break;
        }
    case V_Sg_File_Info:
        {
            // (BW 5/3/07)  Needed to add this since we now represent
            // constructor invocations via the Sg_File_Info of the
            // SgConstructorInitializer, rather than the SgConstructorInitializer.

            // We now also use the Sg_File_Info of a SgDeleteExp to
            // model a destructor invocation.
            SgNode *parent = node->get_parent();
            if ( !isSgConstructorInitializer(parent) && !isSgDeleteExp(parent)) {
                ROSE_ABORT();
            }
            if ( isSgDeleteExp(parent) ) {
                SgDeleteExp *deleteExp = isSgDeleteExp(parent);
                ROSE_ASSERT(deleteExp != NULL);

                // The only actual passed to a destructor is
                // the receiver.
                SgExpression *var = deleteExp->get_variable();
                ROSE_ASSERT(var != NULL);

                actuals.push_back(var);
                break;
            }
            node = parent;
            // fall through
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
            // The implicit "this" actual is stored at the SgExprListExp node
            SgExprListExp* exprListExp = ctorInitializer->get_args();
            ROSE_ASSERT (exprListExp != NULL);

            actuals.push_back(exprListExp);

            // Now, get the non-implicit receiver actuals and
            // push them on the result list.

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
            // SgDeleteExp models an invocation of operator delete,
            // not the destructor.
            SgDeleteExp *deleteExp = isSgDeleteExp(node);
            ROSE_ASSERT(deleteExp != NULL);

            SgFunctionDeclaration *deleteDecl =
                isPlacementDelete(deleteExp);
            if ( deleteDecl == NULL ) {
		std::cerr << "getActuals was not expecting a "
                          << "non-placement delete" << std::endl;
                ROSE_ABORT();
            }

            // The only actual passed to operator delete is
            // the receiver.  Is this correct?  I don't know
            // where else to get actuals.  However, I thought
            // that the type signature of operator delete needed
            // to match that of operator new, which can have
            // more than one argument.
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

//! Return the function in which node occurs.
SgFunctionDefinition *getEnclosingFunction(SgNode *node)
{
    if ( node == NULL ) {
        return NULL;
    }

    if ( isSgGlobal(node) ) {
        return NULL;
    }

    SgFunctionDefinition *functionDefinition =
        isSgFunctionDefinition(node);

    if ( functionDefinition != NULL ) {
        return functionDefinition;
    }

    SgFunctionDeclaration *functionDeclaration =
        isSgFunctionDeclaration(node);

    if ( functionDeclaration != NULL ) {
        functionDeclaration =
            isSgFunctionDeclaration(functionDeclaration->get_definingDeclaration());
        ROSE_ASSERT(functionDeclaration != NULL);
        return getEnclosingFunction(functionDeclaration->get_definition());
    }

    return getEnclosingFunction(node->get_parent());
}


/** \brief  Return boolean indicating whether a function is declared
 *          virtual in its defining class.
 *  \param  functionDeclaration  A function declaration within the AST.
 *  \returns  Boolean indicating whether functionDeclaration is virtual.
 */
static bool
isVirtualWithinDefiningClass(SgFunctionDeclaration *functionDeclaration)
{

    if ( functionDeclaration == NULL ) {
        return false;
    }

    if ( functionDeclaration->get_functionModifier().isVirtual() ) {
        return true;
    }

    SgDeclarationStatement *firstNondefiningDeclaration =
        functionDeclaration->get_firstNondefiningDeclaration();

    if ( firstNondefiningDeclaration == NULL ) {
        return false;
    }

    SgFunctionDeclaration *firstNondefiningFuncDeclaration =
        isSgFunctionDeclaration(firstNondefiningDeclaration);
    ROSE_ASSERT(firstNondefiningFuncDeclaration != NULL);

    if ( firstNondefiningFuncDeclaration->get_functionModifier().isVirtual() ) {
        return true;
    }
    return false;
}

/** \brief  Return boolean indicating whether a function is declared
 *          virtual in some parent class of SgClassDefinition.
 *  \param  functionDeclaration  A function declaration within the AST.
 *  \returns  Boolean indicating whether functionDeclaration is virtual.
 */
bool
isDeclaredVirtualWithinClassAncestry(SgFunctionDeclaration *functionDeclaration,
                                     SgClassDefinition *classDefinition)
{
    SgType *functionType =
        functionDeclaration->get_type();
    ROSE_ASSERT(functionType != NULL);

    SgName funcName = functionDeclaration->get_name();

    // Look in each of the class' parent classes.
    SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances();
    for (SgBaseClassPtrList::iterator i = baseClassList.begin();
         i != baseClassList.end(); ++i) {

        SgBaseClass *baseClass = *i;
        ROSE_ASSERT(baseClass != NULL);

        SgClassDeclaration *classDeclaration = baseClass->get_base_class();
        ROSE_ASSERT(classDeclaration != NULL);

        classDeclaration = isSgClassDeclaration(getDefiningDeclaration(classDeclaration));
        ROSE_ASSERT(classDeclaration != NULL);

        SgClassDefinition  *parentClassDefinition  =
            classDeclaration->get_definition();
        ROSE_ASSERT(parentClassDefinition != NULL);

        // Visit all methods in the parent class.
        SgDeclarationStatementPtrList &members =
            parentClassDefinition->get_members();

        bool isDeclaredVirtual = false;

        for (SgDeclarationStatementPtrList::iterator it = members.begin();
             it != members.end(); ++it) {

            SgDeclarationStatement *declarationStatement = *it;
            ROSE_ASSERT(declarationStatement != NULL);

            switch(declarationStatement->variantT()) {
            case V_SgMemberFunctionDeclaration:
                {
                    SgMemberFunctionDeclaration *memberFunctionDeclaration =
                        isSgMemberFunctionDeclaration(declarationStatement);
                    ROSE_ASSERT(memberFunctionDeclaration);

                    SgName memberFuncName =
                        memberFunctionDeclaration->get_name();

                    if ( ( ( ( *funcName.str() == '~' ) &&
			     ( *memberFuncName.str() == '~') )
                           || ( memberFuncName == funcName ) ) &&
                         isVirtual(memberFunctionDeclaration) ) {
                        SgType *parentMemberFunctionType =
                            memberFunctionDeclaration->get_type();
                        ROSE_ASSERT(parentMemberFunctionType != NULL);

                        eqTypes eq;
                        if ( eq(parentMemberFunctionType, functionType) ) {
                            return true;
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

        if ( isDeclaredVirtualWithinClassAncestry(functionDeclaration,
                                                  parentClassDefinition) ) {
            return true;
        }
    }
    return false;
}

/** \brief  Return boolean indicating whether a function is declared
 *          virtual in some parent class, regardless of whether it is
 *          declared virtual in its defining class.
 *  \param  functionDeclaration  A function declaration within the AST.
 *  \returns  Boolean indicating whether functionDeclaration is virtual.
 */
static bool
isDeclaredVirtualWithinAncestor(SgFunctionDeclaration *functionDeclaration)
{
    SgMemberFunctionDeclaration *memberFunctionDeclaration =
        isSgMemberFunctionDeclaration(functionDeclaration);
    if ( memberFunctionDeclaration == NULL ) {
        return false;
    }

    SgClassDefinition *classDefinition =
        isSgClassDefinition(memberFunctionDeclaration->get_scope());
    ROSE_ASSERT(classDefinition != NULL);

    return isDeclaredVirtualWithinClassAncestry(functionDeclaration,
                                                classDefinition);
}

/** \brief Returns true if the method is a constructor.
 *  \param memberFunctionDeclaration  A method declaration.
 *  \returns  Boolean indicating whether methodFunctionDeclaration is
 *            a constructor.
 */
bool isConstructor(SgMemberFunctionDeclaration *memberFunctionDeclaration)
{
  ROSE_ASSERT(memberFunctionDeclaration != NULL);

  SgSpecialFunctionModifier &specialFunctionModifier =
    memberFunctionDeclaration->get_specialFunctionModifier();

  // Determine whether method is a constructor.
  return ( specialFunctionModifier.isConstructor() );
}

/** \brief  Return boolean indicating whether a function is
 *          virtual-- i.e., whether it is declared virtual
 *          in its defining class or any of that class'
 *          base classes.
 */
bool
isVirtual(SgFunctionDeclaration *functionDeclaration)
{
    if ( functionDeclaration == NULL ) {
        return false;
    }

    // Sometimes constructors are being marked as virtual.
    // Clearly they are not.  Detect this case and return false.
    SgMemberFunctionDeclaration *method =
        isSgMemberFunctionDeclaration(functionDeclaration);
    if ( method == NULL ) {
        return false;
    }

    if ( isConstructor(method) ) {
      return false;
    }

    if ( isVirtualWithinDefiningClass(functionDeclaration) ) {
        return true;
    }

    return isDeclaredVirtualWithinAncestor(functionDeclaration);
}

/** \brief  Returns true if method is a pure virtual method.
 *  \param  functionDeclaration  A method declaration.
 *  \returns  Boolean indicating whether method is a pure virtual
 *            method.
 */
bool isPureVirtual(SgFunctionDeclaration *functionDeclaration)
{
  if ( functionDeclaration == NULL ) return false;

  if ( functionDeclaration->get_functionModifier().isPureVirtual() )
    return true;

  SgDeclarationStatement *firstNondefiningDeclaration =
    functionDeclaration->get_firstNondefiningDeclaration();

  if ( firstNondefiningDeclaration == NULL )
    return false;

  SgFunctionDeclaration *firstNondefiningFuncDeclaration =
    isSgFunctionDeclaration(firstNondefiningDeclaration);
  ROSE_ASSERT(firstNondefiningFuncDeclaration != NULL);

  return firstNondefiningFuncDeclaration->get_functionModifier().isPureVirtual();
}

bool hasDefinition(SgFunctionDeclaration *functionDeclaration)
{
  if ( functionDeclaration == NULL ) return false;

  if ( functionDeclaration->get_definition() != NULL ) {
      return true;
  }

  SgFunctionDeclaration *definingDecl =
      getDefiningDeclaration(functionDeclaration);
  if ( definingDecl == NULL ) {
      return false;
  }

  if ( definingDecl->get_definition() != NULL ) {
      return true;
  }

  return false;
}

/** \brief Return true if a class or any of its base classes
 *         has a virtual method.
 *  \param classDefinition  a SgNode representing a class definition.
 *  \return boolean indicating whether the class represented by
 *                  classDefinition, or any of its base classes,
 *                  define a virtual method.
 */
bool classHasVirtualMethods(SgClassDefinition *classDefinition)
{
    bool hasMethods = false;

    if ( classDefinition == NULL ) return hasMethods;

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
                if ( isVirtual(functionDeclaration) ) {
                    return true;
                }
                break;
            }
        default:
            {
                break;
            }
        }
    }

    if ( hasMethods ) {
        return true;
    }

    // The class did not directly define any virtual methods, look in
    // its base classes.
    SgBaseClassPtrList & baseClassList = classDefinition->get_inheritances();
    for (SgBaseClassPtrList::iterator i = baseClassList.begin();
         i != baseClassList.end(); ++i) {

        SgBaseClass *baseClass = *i;
        ROSE_ASSERT(baseClass != NULL);

        SgClassDeclaration *classDeclaration = baseClass->get_base_class();
        ROSE_ASSERT(classDeclaration != NULL);

        SgClassDefinition  *parentClassDefinition  =
            classDeclaration->get_definition();

        if ( parentClassDefinition != NULL ) {
            if ( classHasVirtualMethods(parentClassDefinition) ) {
                return true;
            }
        }
    }
    return hasMethods;
}

/** \brief  Return boolean indicating whether two functions have the
 *          same type signature.
 */
bool consistentFunctionTypes(SgFunctionType *funcType1,
                             SgFunctionType *funcType2)
{

    ROSE_ASSERT(funcType1 != NULL);
    ROSE_ASSERT(funcType2 != NULL);

    // Compare the return types of the two funcs.
    SgType *func1Type = getBaseType(funcType1->get_return_type());
    SgType *func2Type = getBaseType(funcType2->get_return_type());

    std::string ret1Type, ret2Type;
    getTypeInfo(func1Type, 0, &ret1Type);
    getTypeInfo(func2Type, 0, &ret2Type);
    if ( ret1Type != ret2Type ) {
        return false;
    }


    SgTypePtrList &formal1Types = funcType1->get_arguments();
    SgTypePtrList &formal2Types = funcType2->get_arguments();

    if ( formal1Types.size() != formal2Types.size() ) {
        return false;
    }

    for(SgTypePtrList::iterator it1 = formal1Types.begin(), it2 = formal2Types.begin(); it1 != formal1Types.end(); ++it1, ++it2) {
        SgType *type1 = getBaseType(*it1);
        ROSE_ASSERT(type1 != NULL);

        SgType *type2 = getBaseType(*it2);
        ROSE_ASSERT(type2 != NULL);

        std::string param1Type, param2Type;
        getTypeInfo(type1, 0, &param1Type);
        getTypeInfo(type2, 0, &param2Type);
        if (param1Type != param2Type) {
            return false;
        }

    }

    return true;
}

bool matchingFunctions(SgFunctionDeclaration *decl1,
                       SgFunctionDeclaration *decl2)
{
  //  std::cout << "Comparing decl1: " << decl1->unparseToString() << " and decl2: " << decl2->unparseToString() << std::endl;
  // Compare the names of the two methods.
  SgName name1 = decl1->get_name();
  SgName name2 = decl2->get_name();
  if ( name1 != name2 )
    return false;

  SgFunctionType *funcType1 = decl1->get_type();
  SgFunctionType *funcType2 = decl2->get_type();

  return consistentFunctionTypes(funcType1, funcType2);
}

/** \brief Return a function's mangled name.
 *
 *  We do not rely on ROSE's funcDecl->get_mangled_name().str()
 *  because it will include the class scope information
 *  for a member function.  This will undermine its use
 *  in a virtual method field because we want a single
 *  field to be able to point to any of a number of
 *  virtual methods defined/declared within a class
 *  hierarchy.
 *
 *  For now, just include the function name and the
 *  return and formal types, each separated by an underscore.
 *  This may not be sufficient to capture, e.g., template
 *  info.
 */
std::string mangleFunctionName(SgFunctionDeclaration *functionDeclaration)
{
    ROSE_ASSERT(functionDeclaration != NULL);
    std::string mangled;

    mangled = functionDeclaration->get_name().str();

    SgType *funcType = getBaseType(functionDeclaration->get_orig_return_type());

    // Include the function's return type.
    std::string retType;
    getTypeInfo(funcType, 0, &retType);
    mangled += "_" + retType;

    // Include the formal types.
    SgInitializedNamePtrList &funcParams = functionDeclaration->get_args();
    SgInitializedNamePtrList::iterator p = funcParams.begin();
    for ( ; p != funcParams.end(); ++p) {
        SgType* t = getBaseType((*p)->get_type());
        ROSE_ASSERT(t != NULL);
        std::string paramType;
        getTypeInfo(t, 0, &paramType);
        mangled += "_" + paramType;
    }
    return mangled;
}

/** \brief Return the declaration of the class with a given type.
 *  \param type  a SgNode representing a type.
 *  \return a SgClassDeclaration that is the class declaration
 *          for the given type, or NULL if type does not
 *          correspond to a class.
 */
SgClassDeclaration *
getClassDeclaration(SgType *type)
{
    SgClassDeclaration *classDeclaration = NULL;
    if ( type == NULL ) {
        return NULL;
    }

    switch(type->variantT()) {
    case V_SgPointerType:
        {
            SgPointerType *pointerType = isSgPointerType(type);
            ROSE_ASSERT(pointerType != NULL);

            classDeclaration = getClassDeclaration(pointerType->get_base_type());
            break;
        }
    case V_SgReferenceType:
        {
            SgReferenceType *referenceType = isSgReferenceType(type);
            ROSE_ASSERT(referenceType != NULL);

            classDeclaration = getClassDeclaration(referenceType->get_base_type());
            break;
        }
    case V_SgTypedefType:
        {
            SgTypedefType *typedefType = isSgTypedefType(type);
            ROSE_ASSERT(typedefType != NULL);

            SgDeclarationStatement *declStmt = typedefType->get_declaration();
            ROSE_ASSERT(declStmt != NULL);

            SgTypedefDeclaration *typedefDeclaration =
                isSgTypedefDeclaration(declStmt);
            ROSE_ASSERT(typedefDeclaration != NULL);

            SgType *baseType = typedefDeclaration->get_base_type();
            ROSE_ASSERT(baseType != NULL);

            if ( isSgTypedefType(baseType) ) {
                // Recursive case:  base type of typedef is also a
                // typedef type.
                classDeclaration = getClassDeclaration(baseType);
            } else if ( isSgNamedType(baseType) ) {
                SgNamedType *namedType = isSgNamedType(type);

                SgDeclarationStatement *innerDecl =
                    namedType->get_declaration();
                ROSE_ASSERT(innerDecl != NULL);

                classDeclaration = isSgClassDeclaration(innerDecl);
            }
            break;
        }

    case V_SgClassType:
        {
            SgClassType *classType = isSgClassType(type);
            ROSE_ASSERT(classType != NULL);

            SgDeclarationStatement *declStmt = classType->get_declaration();
            ROSE_ASSERT(declStmt != NULL);

            classDeclaration = isSgClassDeclaration(declStmt);
            ROSE_ASSERT(classDeclaration != NULL);

            break;
        }

   default:
        {
            ROSE_ABORT();
            break;
        }
   }

   classDeclaration = isSgClassDeclaration(getDefiningDeclaration(classDeclaration));
   ROSE_ASSERT(classDeclaration != NULL);

   return classDeclaration;
}

/** \brief Return the definition of the class with a given type.
 *  \param type  a SgNode representing a type.
 *  \return a SgClassDefinition that is the class definition
 *          for the given type, or NULL if type does not
 *          correspond to a class.
 */
SgClassDefinition *
getClassDefinition(SgType *type)
{
    SgClassDeclaration *classDeclaration = getClassDeclaration(type);
    ROSE_ASSERT(classDeclaration != NULL);

    SgClassDefinition *classDefinition =
        classDeclaration->get_definition();

    return classDefinition;
}

SgFunctionDeclaration *getDefiningDeclaration(SgFunctionDeclaration *funcDecl)
{
  return isSgFunctionDeclaration(funcDecl->get_definingDeclaration());
}

SgClassDeclaration *getDefiningDeclaration(SgClassDeclaration *classDecl)
{
  return isSgClassDeclaration(classDecl->get_definingDeclaration());
}

/** \brief Returns boolean indicating whether the invoked
 *         function has name funcName.
 *  \param  functionCallExp A SgFunctionCallExp representing
 *          an invoked function.
 *  \param  funcName  A string representing the name of a function.
 *  \returns  Boolean indicating whether the name of the invoked
 *            function is funcName.
 */
bool isFunc(SgFunctionCallExp *functionCallExp,
            ::std::string funcName)
{
  if( functionCallExp == NULL) return false;

  SgExpression *expression = functionCallExp->get_function();
  ROSE_ASSERT(expression != NULL);

  SgFunctionRefExp *functionRefExp = isSgFunctionRefExp(expression);

  if (functionRefExp == NULL) return false;

  // Found a standard function reference.
  SgFunctionDeclaration *functionDeclaration =
    functionRefExp->get_symbol_i()->get_declaration();
  ROSE_ASSERT(functionDeclaration != NULL);

  SgName name = functionDeclaration->get_name();
  return ( !strcmp(funcName.c_str(), name.str() ) );
}

bool returnsReference(SgFunctionCallExp *functionCallExp)
{
  ROSE_ASSERT(functionCallExp != NULL);

  bool returnsReference = false;

  SgType *type = functionCallExp->get_type();
  //  SgType *type = functionCallExp->get_return_type();
  ROSE_ASSERT(type != NULL);

  if ( isSgReferenceType(type) ) {
    returnsReference = true;
  }

  // I believe in earlier versions of ROSE, that the type
  // of a function call expression was the type of the expression
  // (i.e., the return type).  Evidently, this has changed.
  // Leave the above code for backward compatibility?
  SgFunctionType *funcType = isSgFunctionType(type);

  // NB:  type could be SgTypeVoid and need not be a SgFunctionType.
  if ( funcType != NULL ) {
    SgType *retType = funcType->get_return_type();
    if ( isSgReferenceType(retType) ) {
      returnsReference = true;
    }
  }

  return returnsReference;
}

bool returnsAddress(SgFunctionCallExp *functionCallExp)
{
  ROSE_ASSERT(functionCallExp != NULL);

  bool returnsAddr = false;
  SgType *type = functionCallExp->get_type();
  //  SgType *type = functionCallExp->get_return_type();
  ROSE_ASSERT(type != NULL);

  type = getBaseType(type);
  ROSE_ASSERT(type != NULL);

  if ( isSgReferenceType(type) || isSgPointerType(type) ) {
    returnsAddr = true;
  }

  // I believe in earlier versions of ROSE, that the type
  // of a function call expression was the type of the expression
  // (i.e., the return type).  Evidently, this has changed.
  // Leave the above code for backward compatibility?
  SgFunctionType *funcType = isSgFunctionType(type);

  // NB:  type could be SgTypeVoid and need not be a SgFunctionType.
  if ( funcType != NULL ) {
    SgType *retType = funcType->get_return_type();
    retType = getBaseType(retType);
    ROSE_ASSERT(retType != NULL);
    if ( isSgReferenceType(retType) || isSgPointerType(retType) ) {
      returnsAddr = true;
    }
  }

  return returnsAddr;
}

/** \brief  Return a SgNode representing a 'this' expression.
 *  \param  node  A SgNode from which we would like to derive
 *                a SgNode for a 'this' expression.
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
SgNode *getThisExpNode(SgNode *node)
{
    // Proper handling of 'this' is discussed in the thread
    // Subject: Member function bug and others
    // between Michelle and Brian around Aug 29, 2006.

    SgFunctionDeclaration *functionDeclaration = NULL;

    switch(node->variantT()) {
    case V_SgMemberFunctionDeclaration:
        {
            functionDeclaration = isSgFunctionDeclaration(node);
            break;
        }
    case V_SgTemplateInstantiationMemberFunctionDecl:
        {
            functionDeclaration = isSgFunctionDeclaration(node);
            break;
        }
    case V_SgThisExp:
        {
            SgThisExp *thisExp = isSgThisExp(node);
            ROSE_ASSERT(thisExp != NULL);

            SgFunctionDefinition *functionDefinition =
                getEnclosingFunction(thisExp);
            ROSE_ASSERT(functionDefinition != NULL);

            functionDeclaration = functionDefinition->get_declaration();
            break;
        }

    default:
        {
            std::cerr << "'This' should be represented by its enclosing "
                      << "methods' SgFunctionParameterList."
                      << std::endl
                      << "Don't know how to extract a SgFunctionParameterList "
                      << "from a " << node->sage_class_name()
                      << std::endl;
            ROSE_ABORT();
            break;
        }
    }

    ROSE_ASSERT(functionDeclaration != NULL);

    SgFunctionParameterList *paramList =
        functionDeclaration->get_parameterList();
    ROSE_ASSERT(paramList != NULL);

    return paramList;
}

/** \brief  Return boolean indicating whether the SgConstructorInitializer
 *          initializes an object.
 *  \param  ctorInitializer  A SgConstructorInitializer.
 *  \returns  Boolean indicating whether ctorInitializer initializes
 *            an object.
 *
 *  The following are object initializations:
 *  Foo f;
 *  Foo(Bar &b) : mBar(b) { }
 *
 *  The following are not:
 *  Foo *f = new Foo;
 *  class Foo : public Bar { Foo(Foo &f) : Bar(f) { } }
 *
 *  Notice that the Bar in Bar(f) in the initializer is represented
 *  by a SgInitializedName, whose name is 'Bar'.  Thus if the name
 *  of the SgInitializedName and of the invoked constructor are the
 *  same, this is an invocation of a base class constructor.
 */
bool isObjectInitialization(SgConstructorInitializer *ctorInitializer)
{
    ROSE_ASSERT(ctorInitializer != NULL);

    SgNode *parent = ctorInitializer->get_parent();

    SgInitializedName *initName = isSgInitializedName(parent);
    if ( initName != NULL ) {
        if ( isBaseClassInvocation(ctorInitializer) ) {
            return false;
        }
        return true;
    }
    return false;
}

bool isBaseClassInvocation(SgConstructorInitializer *ctorInitializer)
{
    ROSE_ASSERT(ctorInitializer != NULL);

    SgNode *parent = ctorInitializer->get_parent();

    SgInitializedName *initName = isSgInitializedName(parent);
    if ( initName != NULL ) {
      // If the parent is a SgInitializedName whose name is the
      // same as the constructor being invoked, then that SgInitializedName
      // is not a variable, but a base class.
      SgMemberFunctionDeclaration *invokedCtor =
        ctorInitializer->get_declaration();
      if ( invokedCtor == NULL ) {
          // The constructor initializer is NULL, so we can not determine
          // the name of the invoked constructor.  [This may happen
          // when the constructor is implicitly defined.]  Instead,
          // compare the type of the SgInitializedName, which should
          // have the same name as the constructor, to the name
          // of the SgInitializedName.
          SgType *initType = initName->get_type();
          ROSE_ASSERT(initType != NULL);

          initType = getBaseType(initType);

          SgNamedType *namedType = isSgNamedType(initType);

	  //	  std::string typeName = initType->unparseToString();
	  //	  std::cout << "initName: " << initName->get_name().str() << " type = " << typeName << std::endl;
          if ( ( namedType != NULL ) &&
               ( namedType->get_name() == initName->get_name() ) ) {
            return true;
          }
          return false;
      }
      ROSE_ASSERT(invokedCtor != NULL);

      if ( invokedCtor->get_name() == initName->get_name() ) {
        // In this case, the implicit actual to the constructor
        // invocation is the 'this' param of the caller (constructor).
        // First, get the caller.  However, we want to return
        // something that may be used as the actual/MemRefHandle.
        // We use the SgExprListExp of the invoked constructor
        // for that purpose.
        return true;
      }
    }
    return false;
}

SgMemberFunctionDeclaration *
getInvokedMethod(SgMemberFunctionRefExp *memberFunctionRefExp)
{
    ROSE_ASSERT(memberFunctionRefExp != NULL);

    // Get the declaration of the function.
    SgFunctionSymbol *functionSymbol = memberFunctionRefExp->get_symbol();
    ROSE_ASSERT(functionSymbol != NULL);

    SgFunctionDeclaration *functionDeclaration =
        functionSymbol->get_declaration();
    ROSE_ASSERT(functionDeclaration != NULL);

    SgMemberFunctionDeclaration *methodDecl =
        isSgMemberFunctionDeclaration(functionDeclaration);
    ROSE_ASSERT(methodDecl != NULL);

    return methodDecl;
}

/** \brief isNonStaticMethod returns boolen indicating whether the
 *         method is a non-static method.
 */
bool
isNonStaticMethod(SgMemberFunctionDeclaration *memberFunctionDecl)
{
    ROSE_ASSERT(memberFunctionDecl != NULL);
    return ( !memberFunctionDecl->get_declarationModifier().get_storageModifier().isStatic() );
}

/** \brief  Return boolean indicating whether the invoked function is
 *          va_start.
 */
bool
isVaStart(SgFunctionCallExp *functionCallExp)
{
    ROSE_ASSERT(functionCallExp != NULL);
    // careful, va_start seems to be showing up as __builtin_va_start
    // in ROSE 0.8.9a.
    return ( isFunc(functionCallExp, "va_start") ||
             isFunc(functionCallExp, "__builtin_va_start") );
}

// Utility function to look through typedefs to return a type.
SgType* getBaseType(SgType *type)
{
  if ( type == NULL ) return NULL;

  SgTypedefType *typedefType = isSgTypedefType(type);
  if (typedefType != NULL) {

    SgType *baseType = typedefType->get_base_type();
    ROSE_ASSERT(baseType != NULL);
    return getBaseType(baseType);

  }

  SgModifierType *modifierType = isSgModifierType(type);
  if (modifierType) {
    return getBaseType(modifierType->get_base_type());
  }

  return type;
}

SgType* lookThruReferenceType(SgType *type)
{
  if ( type == NULL ) return NULL;

  SgTypedefType *typedefType = isSgTypedefType(type);
  if (typedefType != NULL) {

    SgType *baseType = typedefType->get_base_type();
    ROSE_ASSERT(baseType != NULL);
    return lookThruReferenceType(baseType);

  }

  SgModifierType *modifierType = isSgModifierType(type);
  if (modifierType) {
    return lookThruReferenceType(modifierType->get_base_type());
  }

  SgReferenceType *refType = isSgReferenceType(type);
  if (refType) {
    return lookThruReferenceType(refType->get_base_type());
  }

  return type;
}

#if 0
// evidently defined in src/midend/astInlining/typeTraits.C
/**
 *  \brief isConstType returns boolean indicating whether type has a const modifier.
 */
bool isConstType(SgType *type)
{

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
                return true;
            }
        }
    }

    return false;
}
#endif

#if 0
// evidently defined in src/midend/astInlining/typeTraits.C
/**
 *  \brief isNonconstReference returns boolean indicating whether type is a
 *         non-const reference type.
 */
bool isNonconstReference(SgType *type)
{
  // This probably isn't what you want.  This looks for a const reference,
  // I'm not sure if such a beast exists.  You probably want a reference
  // to a const type.
  ROSE_ABORT();

    ROSE_ASSERT(type != NULL);
    return ( isSgReferenceType(type) && !isConstType(type) );
}
#endif

//! Returns true if function is a SgArrowExp or a SgDotExp
//! whose operand is a SgPointerDerefExp or a SgDotExp
//! whose lhs is a reference.  In this latter case
//! we will convert the lhs to appear as a pointer.
//! i.e., returns true if function represents a->method() or (*a).method().
bool isArrowExp(SgExpression *function)
{
  if ( isSgArrowExp(function) )
    return true;

  if ( isSgDotExp(function) ) {

    SgBinaryOp *dotOrArrow = isSgBinaryOp(function);
    ROSE_ASSERT(dotOrArrow != NULL);

    SgExpression *lhs = dotOrArrow->get_lhs_operand();
    ROSE_ASSERT(lhs != NULL);

    SgDotExp *dotExp = isSgDotExp(function);
    if ( dotExp != NULL ) {

      SgPointerDerefExp *pointerDerefExp =
	isSgPointerDerefExp(lhs);

      if ( pointerDerefExp != NULL ) {
	return true;
      }
#if 0
      if ( isSgReferenceType(lhs->get_type()) ) {
	return true;
      }
#endif
    }

  }

  return false;
}

bool findNodeInSubtree(SgNode *needle, SgNode *haystack)
{
    if ( ( haystack == NULL ) || ( needle == NULL ) ) {
        return false;
    }

    NodeQuerySynthesizedAttributeType nodeLst = NodeQuery::querySubTree(haystack, V_SgNode);

    for (NodeQuerySynthesizedAttributeType::iterator nodeIt = nodeLst.begin();
         nodeIt != nodeLst.end(); ++nodeIt ) {

        SgNode *n = *nodeIt;
        ROSE_ASSERT(n != NULL);

        if ( n == needle ) {
            return true;
        }
    }

    return false;
}

bool isPlacementNew(SgNewExp *newExp)
{
    ROSE_ASSERT(newExp != NULL);
    SgFunctionDeclaration *newDecl = newExp->get_newOperatorDeclaration();
    SgExprListExp *placement_args = newExp->get_placement_args();

    // If the new expression uses placement args, it should
    // also have a handle to the invoked new operator.
    // However, we may have the handle without having placement
    // args (i.e., the defined operator new may simply use
    // the default size_t arg).
    ROSE_ASSERT( ( newDecl != NULL ) || ( placement_args == NULL ) );

    return ( newDecl != NULL );
}

SgFunctionDeclaration *isPlacementDelete(SgDeleteExp *deleteExp)
{
    ROSE_ASSERT(deleteExp != NULL);
    SgFunctionDeclaration *deleteDecl =
        deleteExp->get_deleteOperatorDeclaration();
    return deleteDecl;
}

bool isOperatorNew(SgFunctionDeclaration *functionDeclaration)
{
    ROSE_ASSERT(functionDeclaration != NULL);
    SgSpecialFunctionModifier &specialFunctionModifier =
        functionDeclaration->get_specialFunctionModifier();

    SgName functionName = functionDeclaration->get_name();

    //   std::cout << "functionName = " << functionName.str() << " operator = " << specialFunctionModifier.isOperator() << std::endl;

    return ( specialFunctionModifier.isOperator() &&
             functionName == SgName("operator new") );
}

/**
 *  Return any function declarations defined in cls or any of
 *  its derived classes, which have a function signature consistent
 *  with funcType.
 */
std::vector<SgFunctionDeclaration *>
getFunctionsInDerivedClasses(SgClassDefinition *cls,
                             SgFunctionType *funcType,
                             ClassHierarchyWrapper *classHierarchy)
{
    std::vector<SgFunctionDeclaration *> funcs;

    SgDeclarationStatementPtrList &members = cls->get_members();
    for (SgDeclarationStatementPtrList::iterator it = members.begin();
         it != members.end(); ++it) {

        SgMemberFunctionDeclaration *functionDeclaration =
            isSgMemberFunctionDeclaration(*it);
        if ( functionDeclaration != NULL ) {
            SgFunctionType *methodType = functionDeclaration->get_type();
            ROSE_ASSERT(methodType != NULL);
            if ( consistentFunctionTypes(methodType, funcType) ) {
                funcs.push_back(functionDeclaration);
            }
        }
    }

    ClassHierarchyWrapper::ClassDefSet subclasses =
        classHierarchy->getSubclasses(cls);

    // Iterate over all subclasses.
    for (ClassHierarchyWrapper::ClassDefSet::iterator subclassIt = subclasses.begin();
          subclassIt != subclasses.end(); ++subclassIt) {

        SgClassDefinition *subclass = *subclassIt;
        ROSE_ASSERT(subclass != NULL);

        SgDeclarationStatementPtrList &members = subclass->get_members();
        for (SgDeclarationStatementPtrList::iterator it = members.begin();
              it != members.end(); ++it) {

            SgMemberFunctionDeclaration *functionDeclaration =
                isSgMemberFunctionDeclaration(*it);
            if ( functionDeclaration != NULL ) {
                SgFunctionType *methodType = functionDeclaration->get_type();
                ROSE_ASSERT(methodType != NULL);
                if ( consistentFunctionTypes(methodType, funcType) ) {
                    funcs.push_back(functionDeclaration);
                }
            }
        }
    }
    return funcs;
}

/**
 *  Return the list of methods that may be called at a method pointer
 *  invocation.  If the pointer is invoked on a receiver of type T,
 *  the conservative set of methods that may be invoked include any
 *  non-static, virtual or non-virtual method m with type signature
 *  consistent with the method pointer that is defined in a class C.
 *  C is any class derived from any of the ancestors of T.
 */
std::set<SgFunctionDeclaration *>
getFunctionsConsistentWithMethodPointerInvocation(SgBinaryOp *dotStarOrArrowStar,
                                                  ClassHierarchyWrapper *classHierarchy)
{
    SgDotStarOp *dotStar = isSgDotStarOp(dotStarOrArrowStar);
    SgArrowStarOp *arrowStar = isSgArrowStarOp(dotStarOrArrowStar);
    ROSE_ASSERT( ( dotStar != NULL ) || ( arrowStar != NULL ) );

    SgType *lhsType = NULL;
    SgExpression *receiver = dotStarOrArrowStar->get_lhs_operand();
    ROSE_ASSERT(receiver != NULL);

    SgType *receiverType = receiver->get_type();
    ROSE_ASSERT(receiverType != NULL);

    SgClassDeclaration *classDecl = getClassDeclaration(receiverType);
    ROSE_ASSERT(classDecl != NULL);

    classDecl = isSgClassDeclaration(getDefiningDeclaration(classDecl));
    ROSE_ASSERT(classDecl != NULL);

    SgClassDefinition *classDefn = classDecl->get_definition();
    ROSE_ASSERT(classDefn != NULL);

    SgExpression *rhs = dotStarOrArrowStar->get_rhs_operand();
    ROSE_ASSERT(rhs != NULL);

    SgPointerMemberType *rhsType = isSgPointerMemberType(rhs->get_type());
    ROSE_ASSERT(rhsType != NULL);

    SgMemberFunctionType *funcType =
        isSgMemberFunctionType(rhsType->get_base_type());
    ROSE_ASSERT(funcType != NULL);

    std::set<SgFunctionDeclaration *> funcs;

    // Get all of the ancestors of the receiver type.
    ClassHierarchyWrapper::ClassDefSet ancestors =
        classHierarchy->getAncestorClasses(classDefn);
    std::vector<SgClassDefinition *> classes;
    classes.push_back(classDefn);

    // Iterate over all ancestors.
    for (ClassHierarchyWrapper::ClassDefSet::iterator ancestorIt = ancestors.begin();
          ancestorIt != ancestors.end(); ++ancestorIt) {

        SgClassDefinition *ancestor = *ancestorIt;
        ROSE_ASSERT(ancestor != NULL);

        classes.push_back(ancestor);

    }

    for (int i = 0; i < classes.size(); ++i) {

        SgClassDefinition *cls = classes[i];
        ROSE_ASSERT(cls != NULL);

        std::vector<SgFunctionDeclaration *> funcsWithCompatibleTypes;
        funcsWithCompatibleTypes =
            getFunctionsInDerivedClasses(cls,
                                         funcType,
                                         classHierarchy);


        for (int i = 0; i < funcsWithCompatibleTypes.size(); ++i) {
            SgMemberFunctionDeclaration *funcDecl =
              isSgMemberFunctionDeclaration(funcsWithCompatibleTypes[i]);
            ROSE_ASSERT(funcDecl != NULL);

            if ( isNonStaticMethod(funcDecl) &&
                 ( funcs.find(funcDecl) == funcs.end() ) ) {
                funcs.insert(funcDecl);
            }
        }
    }

    return funcs;
}

/*
 * SHK: Wrapper to return globalScope of an SgFile
 */
SgGlobal * globalScope(SgFile * file){
  //SHK - sgFile->root is not longer supported.
  if(SgSourceFile * source = isSgSourceFile(file)){
    return source->get_globalScope();
  }else if(SgSourceFile * unknown =isSgSourceFile(file)){
    return unknown->get_globalScope();
  }else{
    //classify_SgNode(file);
    //fatal(" An SgFile node is neither a sourcefile nor an Unkownfile. It is classified above :");
    ROSE_ASSERT(0);
  }
}


} // end of namespace UseOA
