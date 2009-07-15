// Written for undergraduate C++ course project at Dept of
// Computer Science, Rutgers University.  
#include <iostream>
#include <string.h>
#include "student3.h"
using namespace std;
// evaluation of a constant is the value of the constant.

double const_expr::eval(double numb)
{
  return (this) -> value;
}

// evaluation of a variable is the input value.

double var_expr::eval(double numb)
{
  return numb;
}

// evaluation of a sum expression is the sum of the values of its
// subexpressions.

double sum_expr::eval(double numb)
{
  return ( *((this) -> first)).eval(numb) + ( *((this) -> second)).eval(numb);
}

// evaluation of a product expression is the product of the values of 
// its subexpressions.

double prod_expr::eval(double numb)
{
  return ( *((this) -> first)).eval(numb) * ( *((this) -> second)).eval(numb);
}

// evaluation of a quotient expression of the quotient
// of the values of its subexpressions.

double quotient_expr::eval(double numb)
{
  return ( *((this) -> first)).eval(numb) / ( *((this) -> second)).eval(numb);
}

// An expression is printed recursively such that its left subexpression
// is printed, then the operator, then its right subexpression.

void bin_op_expr::print_me()
{
   *((&std::cout))<<"(";
  ( *((this) -> first)).print_me();
   *((&std::cout))<<" "<<((this) -> op_name)<<" ";
  ( *((this) -> second)).print_me();
   *((&std::cout))<<")";
}

// The derivative of a constant is 0.

expr *const_expr::deriv(strng var)
{
  return (new const_expr ((0)));
}

// The derivative of a variable is 1 if the derivative is in
// respect to the variable, and 0 otherwise.

expr *var_expr::deriv(strng var)
{
/*strcmp(var, name)*/
  if ((0)) {
    return (new const_expr ((0)));
  }
  else {
    return (new const_expr ((1)));
  }
}

// The derivative of a sum expression is equal to the sum of the 
// derivatives of the subexpressions.

expr *sum_expr::deriv(strng var)
{
  class expr *dvfirst;
  class expr *dvsecond;
  dvfirst = ( *((this) -> first)).deriv(var);
  dvsecond = ( *((this) -> second)).deriv(var);
  if (((dvfirst -> isconst())) && (dvfirst -> eval((0)) == (0))) {
    return dvsecond;
  }
  else {
    if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (0))) {
      return dvfirst;
    }
    else {
      return ((new sum_expr (dvfirst,dvsecond)));
    }
  }
}

// The derivative of a product expression is equal the the sum
// of the products of the first subexpression and the derivative of the
// second subexpression, and the second subexpression and the 
// derivative of the first.

expr *prod_expr::deriv(strng var)
{
  class expr *dvfirst;
  class expr *dvsecond;
  dvfirst = ( *((this) -> first)).deriv(var);
  dvsecond = ( *((this) -> second)).deriv(var);
  if (((dvfirst -> isconst())) && (dvfirst -> eval((0)) == (0))) {
    if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (0))) {
      return dvfirst;
    }
    else {
      if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (1))) {
        return (this) -> first;
      }
      else {
        return ((new prod_expr (((this) -> first),dvsecond)));
      }
    }
  }
  else {
    if (((dvfirst -> isconst())) && (dvfirst -> eval((0)) == (1))) {
      if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (0))) {
        return (this) -> second;
      }
      else {
        if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (1))) {
          return ((new sum_expr (((this) -> first),((this) -> second))));
        }
        else {
          return ((new sum_expr ((((new prod_expr (((this) -> first),dvsecond)))),((this) -> second))));
        }
      }
    }
    else {
      if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (0))) {
        return ((new prod_expr (dvfirst,((this) -> second))));
      }
      else {
        if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (1))) {
          return ((new sum_expr (((this) -> first),(((new prod_expr (dvfirst,((this) -> second))))))));
        }
        else {
          return ((new sum_expr ((((new prod_expr (((this) -> first),dvsecond)))),(((new prod_expr (dvfirst,((this) -> second))))))));
        }
      }
    }
  }
}

// The derivative of a quotient expression if equal to:
// (second subexpresssion * derivative of the first) - (the derivative
// of the second * the first subexpression) all over the
// second subexpression squared.

expr *quotient_expr::deriv(strng var)
{
  class expr *dvfirst;
  class expr *dvsecond;
  dvfirst = ( *((this) -> first)).deriv(var);
  dvsecond = ( *((this) -> second)).deriv(var);
  if (((dvfirst -> isconst())) && (dvfirst -> eval((0)) == (0))) {
    if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (0))) {
      return dvfirst;
    }
    else {
      if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (1))) {
        if (( *((this) -> first)).eval((0)) == (1)) {
          return ((new quotient_expr (((new const_expr (((-1))))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
        else {
          return ((new quotient_expr ((((new prod_expr (((new const_expr (((-1))))),((this) -> first))))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
      }
      else {
        if (( *((this) -> first)).eval((0)) == (1)) {
          return ((new quotient_expr ((((new prod_expr (((new const_expr (((-1))))),dvsecond)))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
        else {
          return ((new quotient_expr ((((new prod_expr (((new const_expr (((-1))))),(((new prod_expr (dvsecond,((this) -> first))))))))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
      }
    }
  }
  else {
    if (((dvfirst -> isconst())) && (dvfirst -> eval((0)) == (1))) {
      if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (0))) {
        return ((new quotient_expr (dvfirst,((this) -> second))));
      }
      else {
        if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (1))) {
          return ((new quotient_expr ((((new sum_expr (((this) -> second),(((new prod_expr (((new const_expr (((-1))))),((this) -> first))))))))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
        else {
          return ((new quotient_expr ((((new sum_expr (((this) -> second),(((new prod_expr (((new const_expr (((-1))))),(((new prod_expr (dvsecond,((this) -> first))))))))))))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
      }
    }
    else {
      if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (0))) {
        return ((new quotient_expr (dvfirst,((this) -> second))));
      }
      else {
        if (((dvsecond -> isconst())) && (dvsecond -> eval((0)) == (1))) {
          return ((new quotient_expr ((((new sum_expr ((((new prod_expr (((this) -> second),dvfirst)))),(((new prod_expr (((new const_expr (((-1))))),((this) -> first))))))))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
        else {
          return ((new quotient_expr ((((new sum_expr ((((new prod_expr (((this) -> second),dvfirst)))),(((new prod_expr (((new const_expr (((-1))))),(((new prod_expr (dvsecond,((this) -> first))))))))))))),(((new prod_expr (((this) -> second),((this) -> second))))))));
        }
      }
    }
  }
}


int main()
{
  class const_expr c((8));
  class var_expr x(("x"));
  class prod_expr simple(((new const_expr (123.450000000000002842171))),((new var_expr (("y")))));
   *((&std::cout))<<"c is ";
  c.const_expr::print_me();
  ( *((&std::cout))<<"\n      and its value at 3 is: ")<<c.const_expr::eval((3));
   *((&std::cout))<<"\n      and its derivative with respect to x is: ";
  ( *c.const_expr::deriv(("x"))).print_me();
   *((&std::cout))<<"\nx is ";
  x.var_expr::print_me();
  ( *((&std::cout))<<"\n      and its value at 3 is: ")<<x.var_expr::eval((3));
   *((&std::cout))<<"\n      and its derivative with respect to x is: ";
  ( *x.var_expr::deriv(("x"))).print_me();
   *((&std::cout))<<"\nsimple is ";
  simple.print_me();
  ( *((&std::cout))<<"\n     and its value at 3 is: ")<<simple.prod_expr::eval((3));
   *((&std::cout))<<"\n     and its derivative with respect to y is: ";
  ( *simple.prod_expr::deriv(("y"))).print_me();
   *((&std::cout))<<"\n     and its derivative with respect to x is: ";
  ( *simple.prod_expr::deriv(("x"))).print_me();
}

