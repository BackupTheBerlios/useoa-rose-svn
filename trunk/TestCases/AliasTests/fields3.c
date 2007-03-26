/*
 * fields3.c
 *
 * s.g and s.f should end up aliased.
 */
void main() {
    int x, y;
    int *p;
    struct test {
        int *f;
        int *g;
    } s;

    p = &x;
    s.f = &y;
    s.g = p;
    s.f = &x;
}
