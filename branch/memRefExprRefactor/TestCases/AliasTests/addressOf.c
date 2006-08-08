/* Testing to see how various addressOf expression are compiled into an AST.
 */

int main() {
    int a, b, c;
    int *p, **q;
    int v[3];
    struct temp {
        int hello;
    } mystruct;

    p = &a;
    q = &p;

    p = &**q;
    p = &v[2];
    p = &*p;
    p = &mystruct.hello;
    return 0;
}
