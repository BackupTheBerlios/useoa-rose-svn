// #include <iostream>

class Base
{
public:
  int num;
  virtual int method(double x) { 
    // std::cout << "Base::method" << std::endl; 
  };  
  int nonvirt(double x) { 
    // std::cout << "Base::nonvirt" << std::endl; 
  }
  int noncompatible(double x, int y) { 
    // std::cout << "Base::noncompatible" << std::endl; 
  }
};

class Derived : public Base {
public:
  virtual int method(double x) { 
    // std::cout << "Derived::method" << std::endl; 
  };  
  int nonvirt(double x) { 
    // std::cout << "Derived::nonvirt" << std::endl; 
  }
  int noncompatible(double x, int y) { 
    // std::cout << "Derived::noncompatible" << std::endl; 
  }
};

int main() 
{
  // Declare pmv as a pointer to an integer member of class Base.
  // Initialize it to point to the num field of a class Base.
  // Notice it is not initialized to the num field of
  // an object of class Base!
  int Base::*pmv = &Base::num;

  // Declare pmf as a pointer to a method of class Base (or any
  // class unambiguously derived from it), which returns an int
  // and takes a double.
  // No initialization here.
  int (Base::*pmf)(double);

  Base b;
  Derived d;
  Base *bptr = &b;
  Base *dptr = &d;

  // Declare pmf to point to Base::method.  More specifically
  // it will point to the virtual pointer table entry for method.
  pmf = &Base::method;

  // Invoke Base::method (i.e., b.method) by applying .* operator to an object.
  (b.*pmf)(5.0);

  // Invoke Derived::method (i.e., d.method) by applying .* to an object.
  (d.*pmf)(5.0);

  // Invoke Base::method (i.e., bptr->method) by applying ->* 
  // operator to a pointer.
  (bptr->*pmf)(5.0);

  // Invoke Derived::method (i.e., dptr->method) by applying ->* to a pointer.
  (dptr->*pmf)(5.0);

  // Declare pmf to point to Base::nonvirt.  There is no
  // virtual pointer table entry for a non-virtual method.
  // This points to Base::nonvirt, itself.
  pmf = &Base::nonvirt;

  // All of these invoke Base::nonvirt, regardless of the (run-time) 
  // type of the receiver.
  (b.*pmf)(5.0);
  (d.*pmf)(5.0);
  (bptr->*pmf)(5.0);
  (dptr->*pmf)(5.0);
  
  // Set b.num = 5
  b.*pmv = 5;

  // Set d.num = 5
  d.*pmv = 5;

  // Set bptr->num = 5
  bptr->*pmv = 5;

  // Set dptr->num = 5
  dptr->*pmv = 5;

  return 0;
}
