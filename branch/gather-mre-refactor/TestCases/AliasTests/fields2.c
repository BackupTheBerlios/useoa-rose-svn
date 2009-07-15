/*
 * Testing the difference between FIAlias that merges all SubSetRefs
 * with corresponding NamedRef and the fix I will make where it doesn't.
 *
 * In the older version of FIAlias, s.g and s.f should end up in the
 * same union-find set and therefore aliased.  If fields are handled properly
 * then they shouldn't.
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
}
