/*
   Goal is to test how FIAliasAliasMap handles InvisibleLocs.
   If there is a non-visible named ref or no fixed locs then
   there should be invisible locs.
   If there is no non-visible named ref and there is
   a locally visible named ref, then there should be no
   invisible locs.
 */
void head(int *x, int *y) 
{
    int a;
    x = &a;     // *x should map to a only, no inv locs
    *x;
    *y;         // *y should map to an invisible loc
}

void tail(int *z)
{
    int b;
    z = &b;
    *z;         // *z should map to an invisible loc because &c is passed to it
}

int main()
{
    int c;
    tail(&c);
    return 0;
}
