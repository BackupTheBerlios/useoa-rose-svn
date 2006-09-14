
/** 
 *  Purpose:  Test reference conversion rules 3 and 4 on initialization:
 *  If the rhs of the initialization has an lval, we take its 
 *  address (rule 4), if it does not, we deref the lhs (rule 3).  
 */

int main()
{
   int x;

   // no lval for the rhs.
   const int &constRef = 5;    // should get NamedRef(constRef);

   // no lval for the rhs.
   // should model as:
   // int *tmp = &x;
   // int **addrRef = &tmp;
   const int *&addrRef = &x;   // reference to a const pointer.

   // lval on the lhs.
   int &ref = x;               // should get <ref, &x> ptr assign pair.

   // both lval and non-lval.
   bool cond = true;

   // should get, NamedRef(ref2) for ref2 = 5 and
   //             <ref2, &x> for ref2 = x;
   const int &ref2 = ( cond ? x : 5 );  

   // should get <ref3, ref>
   int &ref3 = ref;

   return 0;
}
