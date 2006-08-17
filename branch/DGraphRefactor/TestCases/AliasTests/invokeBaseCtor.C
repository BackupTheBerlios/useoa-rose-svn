class Base {
};

class SubClass : public Base {
 public:
  SubClass(Base & parSubClass) : myParent(parSubClass) {}
  Base &myParent;
};

class SubSubClass : public SubClass { 
 public: 
  SubSubClass(Base &par, int & aint) : SubClass(par) {} 
}; 


int main()
{
  return 0;
}
