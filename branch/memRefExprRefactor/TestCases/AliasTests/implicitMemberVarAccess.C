
/*  Purpose:  Test a member variable access with an implicit receiver.  
 *            ROSE should make the receiver explicit. 
 */

class Foo {

public:
  Foo() { mX = 5; }
  ~Foo() { }
  int mX;
};

int main()
{
    return 0;
}
