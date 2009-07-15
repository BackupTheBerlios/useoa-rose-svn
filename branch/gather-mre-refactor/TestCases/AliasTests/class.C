/*
   Attempting to create SgMemberFunctionRefExp and
   SgClassNameRefExp in the Sage AST.
*/

class B {
};

class A {
    B& mB;
    public:
        A(B &p) : mB(p) {}
        void foo() { int x; }
        static int sInt;
};

int main() {
    B b;
    A a(b);
    a.foo();
    return A::sInt;
}
