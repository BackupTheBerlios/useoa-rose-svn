/*
   Testing assignments that return a value.
   Specifically want to check that their memory reference expressions are
   generated correctly.
 */

int g;

int* foo(int** p) {
    int *a, b, c;
    *p = a = &b;
    return *p = a = &g;
}

int main() {
    int *q, *r;
    r = foo(&q);
    *r;
    return 0;
}
