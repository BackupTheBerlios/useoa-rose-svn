/*
   Testing field accesses through ptrs and direct.
*/

void main() {
    int n;
    struct test {
        int a;
        int b;
        int *p;
    } *x, y, **z;

    x = &y;
    z = &x;

    (*z)->p = &n;
    x->p = y.p;
    (*x).p = &n;
}
