
/*  Purpose:  Test parameter bindings for the construction of a heap variable. */

class Foo {

public:
  Foo(int x) { }
};

int main()
{
    int y;
    Foo *f = new Foo(y);

    return 0;
}
