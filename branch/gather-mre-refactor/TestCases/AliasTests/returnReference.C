
/**
 *  Purpose:  Test our ability to dereference the return value
 *            corresponding to a reference type.
 */

int global;

int &globalRef = global;

int &returnRef() { return global; }
int &returnRef2() { return globalRef; }
const int &returnConstRef() { return global; }

// This gives a warning as we are returning a reference to 
// a temporary.  But, it compiles, at least with g++ 3.3.3,
// so test it.
// We model as:
//    int *tmp = &global;
//    return tmp; 
// where ther return will be modeled as:
//    returnConstRef2:returnSlot = &tmp; 
int * const &returnConstRef2() { return &global; }

int main()
{
    int x = returnRef();
    int &xRef = returnRef();

    int x2 = returnRef2();
    int &xRef2 = returnRef2();

    int x3 = returnConstRef();
    const int &xRef3 = returnConstRef();

    int *const &refPtr = returnConstRef2();

    return 0;
}
