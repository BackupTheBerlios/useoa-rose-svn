
/*  Purpose:  Test access of static member variables.   Should not
 *            have an implicit 'this' added.
 */

class Foo {

public:
  static int sX;
  
};

int main()
{
    Foo::sX = 0;
    return 0;
}
