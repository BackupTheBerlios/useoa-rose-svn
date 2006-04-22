/*
  func-ptrs.C

  Testing the AliasIRInterface implementations for C/C++ compilers.
  Specifically looking at all the different ways we can use function ptrs.
  Got most of the code from http://www.newty.de/fpt/fpt.html.
*/

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
using namespace std;

// C, for function ptr example
int DoIt  (float a, char b, char c){ printf("DoIt\n");   return (int)a+(int)b+(int)c; }
int DoMore(float a, char b, char c){ printf("DoMore\n"); return (int)a-(int)b+(int)c; }
// C++
class TMyClass
{
public:
   int DoIt(float a, char b, char c){ cout << "TMyClass::DoIt"<< endl; return (int)a+(int)b+(int)c;};
   int DoMore(float a, char b, char c) const
     { cout << "TMyClass::DoMore" << endl; return (int)a-(int)b+(int)c; };

   /* more of TMyClass */
};

int main()
{
    // function ptrs
    // got examples from http://www.newty.de/fpt/fpt.html
    int (*pt2Function)(float, char, char) = NULL;                        // C

    pt2Function = DoIt;      // short form
    pt2Function = &DoMore;   // correct assignment using address operator

    //TMyClass tmc;

    int (TMyClass::*pt2Member)(float, char, char) = 0; 
    int (TMyClass::*pt2ConstMember)(float, char, char) const = 0;
    pt2Member = &TMyClass::DoIt; // correct assignment using address operator
    pt2ConstMember = &TMyClass::DoMore; 

    // 2.5 calling a function using a function pointer
    int result1 = pt2Function    (12, 'a', 'b');          // C short way
    int result2 = (*pt2Function) (12, 'a', 'b');          // C

    TMyClass instance1;
    int result3 = (instance1.*pt2Member)(12, 'a', 'b');   // C++
    TMyClass* instance2 = new TMyClass;
    int result4 = (instance2->*pt2Member)(12, 'a', 'b');  // C++, instance2 is a pointer
    delete instance2;



    return 0;
}

//---------------------------------------------------------------------
// 2.6 How to Pass a Function Pointer
// <pt2Func> is a pointer to a function which returns an int and takes a float and two char
void PassPtr(int (*pt2Func)(float, char, char))
{
   int result = (*pt2Func)(12, 'a', 'b');     // call using function pointer
   cout << result << endl;
}

// execute example code - 'DoIt' is a suitable function like defined above in 2.1-4
void Pass_A_Function_Pointer()
{
   cout << endl << "Executing 'Pass_A_Function_Pointer'" << endl;
   PassPtr(&DoIt);
}

float Plus(float a, float b)
{ return a + b; }

float Minus(float a, float b)
{ return a - b; }

//---------------------------------------------------------------------
// 2.7 How to Return a Function Pointer
//     'Plus' and 'Minus' are defined above. They return a float and take two float


// Direct solution: Function takes a char and returns a pointer to a
// function which is taking two floats and returns a float. <opCode>
// specifies which function to return
float (*GetPtr1(const char opCode))(float, float)
{
   if(opCode == '+')
      return &Plus;
   else
      return &Minus; // default if invalid operator was passed
}


// Solution using a typedef: Define a pointer to a function which is taking
// two floats and returns a float
typedef float(*pt2Func)(float, float);

// Function takes a char and returns a function pointer which is defined
// with the typedef above. <opCode> specifies which function to return
pt2Func GetPtr2(const char opCode)
{
   if(opCode == '+')
      return &Plus;
   else
      return &Minus; // default if invalid operator was passed
}

// Execute example code
void Return_A_Function_Pointer()
{
   cout << endl << "Executing 'Return_A_Function_Pointer'" << endl;

   // define a function pointer and initialize it to NULL
   float (*pt2Function)(float, float) = NULL;

   pt2Function=GetPtr1('+');   // get function pointer from function 'GetPtr1'
   cout << (*pt2Function)(2, 4) << endl;   // call function using the pointer


   pt2Function=GetPtr2('-');   // get function pointer from function 'GetPtr2'
   cout << (*pt2Function)(2, 4) << endl;   // call function using the pointer
}

//------------------------------------------------------------------------------------
// 2.8 How to Use Arrays of Function Pointers

// C ---------------------------------------------------------------------------------

// type-definition: 'pt2Function' now can be used as type
typedef int (*pt2Function)(float, char, char);

// illustrate how to work with an array of function pointers
void Array_Of_Function_Pointers()
{
   printf("\nExecuting 'Array_Of_Function_Pointers'\n");

   // define arrays and ini each element to NULL, <funcArr1> and <funcArr2> are arrays
   // with 10 pointers to functions which return an int and take a float and two char

   // first way using the typedef
   pt2Function funcArr1[10] = {NULL};

   // 2nd way directly defining the array
   int (*funcArr2[10])(float, char, char) = {NULL};


   // assign the function's address - 'DoIt' and 'DoMore' are suitable functions
   // like defined above in 2.1-4
   funcArr1[0] = funcArr2[1] = &DoIt;
   funcArr1[1] = funcArr2[0] = &DoMore;

   /* more assignments */

   // calling a function using an index to address the function pointer
   printf("%d\n", funcArr1[1](12, 'a', 'b'));         //  short form
   printf("%d\n", (*funcArr1[0])(12, 'a', 'b'));      // "correct" way of calling
   printf("%d\n", (*funcArr2[1])(56, 'a', 'b'));
   printf("%d\n", (*funcArr2[0])(34, 'a', 'b'));
}


// C++ ---------------------------------------------------------------------

// type-definition: 'pt2Member' now can be used as type
typedef int (TMyClass::*pt2Member)(float, char, char);

// illustrate how to work with an array of member function pointers
void Array_Of_Member_Function_Pointers()
{
   cout << endl << "Executing 'Array_Of_Member_Function_Pointers'" << endl;

   // define arrays and ini each element to NULL, <funcArr1> and <funcArr2> are
   // arrays with 10 pointers to member functions which return an int and take
   // a float and two char

   // first way using the typedef
   pt2Member funcArr1[10] = {NULL};

   // 2nd way of directly defining the array
   int (TMyClass::*funcArr2[10])(float, char, char) = {NULL};


   // assign the function's address - 'DoIt' and 'DoMore' are suitable member
   //  functions of class TMyClass like defined above in 2.1-4
   funcArr1[0] = funcArr2[1] = &TMyClass::DoIt; 
   
   //nd use an array of function pointers in C and C++. The first way uses a typedef, the second way directly defines the array. It's up to you which way you prefer.


   // doesn't compile with g++
   //funcArr1[1] = funcArr2[0] = &TMyClass::DoMore;
   /* more assignments */

   // calling a function using an index to address the member function pointer
   // note: an instance of TMyClass is needed to call the member functions
   TMyClass instance;
   cout << (instance.*funcArr1[1])(12, 'a', 'b') << endl;
   cout << (instance.*funcArr1[0])(12, 'a', 'b') << endl;
   cout << (instance.*funcArr2[1])(34, 'a', 'b') << endl;
   cout << (instance.*funcArr2[0])(89, 'a', 'b') << endl;
}


