
/*  Purpose:  Test parameter bindings for the construction of a stack variable. */

class Foo {

public:
  Foo(int x) { }
};

int main()
{
    int y;
    Foo f(y);

    return 0;
}
