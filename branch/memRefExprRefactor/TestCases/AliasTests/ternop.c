/*
   Testing the terniary operator.
*/

int main() {
    int *r, *p, **q, a, b;

    r = &a;
    p = &b;
    q = &p;

    *( r<*q ? r:*q ) = 42;

    return 0;
}
