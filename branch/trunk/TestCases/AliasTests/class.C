/*
   Attempting to create SgMemberFunctionRefExp and
   SgClassNameRefExp in the Sage AST.
*/

class A {
    public:
        void foo() { int x; }
        static int sInt;
};

int main() {
    A a;
    a.foo();
    return A::sInt;
}
