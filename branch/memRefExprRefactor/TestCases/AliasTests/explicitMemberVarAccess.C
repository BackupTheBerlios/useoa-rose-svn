
/*  Purpose:  Test a member variable access with an explicit receiver.  
 */

class Foo {

public:
  Foo() { this->mX = 5; }
  ~Foo() { }
  int mX;
};

int main()
{
    return 0;
}
