/*****************************************
Test code to probe the generation of default functions 
in ROSE

myClass1: has user-defined functions and used
	intend to test if the compiler will see existing ones
myClass2: used twice, without any user-defined functions
	compiler should generate all for it
myClass3: not used and without constructor
	compiler will not do anything to it. 
Not true!  We have changed the behavior such that
undefined special methods are always defined, even
if they are not used. bwhite. 6/21/06
By C. Liao
Date: June 9, 2006
******************************************/

class myClass1{
 private:
   int _count;
   char* _data;
   float _weight;
 public:
  // Should create the following:
  //     myClass1() { }
  //     myClass1(myClass1 &rhs) 
  //         : _count(rhs._count), _data(rhs._data), _weight(rhs._weight) { }
    ~myClass1(){}  //destructor
    myClass1 & operator=(const myClass1 &rhs)//assignment operator
    {
      if (this==&rhs) return *this;
      _count=rhs._count;
      _weight=rhs._weight;
      return *this;
    }
};

class myClass2
{
  // Should create the following:
  //     myClass2() { }
  //     myClass2(myClass2 &rhs) :  _count(rhs._count) { }
  //     ~myClass2() { }
  //     myClass2 &operator=(myClass2 &rhs) {
  //         if (this == &rhs) {
  //             return *this;
  //         } else {
  //         }
  //         this -> _count = rhs._count;
  //         return *this;
  //     }
private:
  myClass1 A;
  int _count;

};

// Should create the following:
//     myClass3() { }
//     myClass3(myClass3 &rhs) :  _count(rhs._count) { }
//     ~myClass3() { }
//     myClass3 &operator=(myClass3 &rhs) {
//         if (this == &rhs) {
//             return *this;
//         } else {
//         }
//         this -> _count = rhs._count;
//         return *this;
//     }
class myClass3{};

int main(int argc, char* argv[])
{
myClass1 *k1=new myClass1;
myClass2 *k2=new myClass2;
//myClass1 object2(*k1);
//myClass1 *k3=new myClass1;
// *k3= *k1;
return 0;
}

