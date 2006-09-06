class Base {
public:
};

class SubClass : public Base {
 public:
  SubClass(Base & parSubClass) : myParent(parSubClass) {}
  Base &myParent;
};

class Foo { public: Foo(Foo &f) { } };

class SubSubClass : public SubClass { 
 public: 
  SubSubClass(Base &par, int & aint, Foo &f) : SubClass(par), mF(f) {} 
  Foo mF;
}; 


int main()
{
  return 0;
}
