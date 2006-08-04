int strcmp( char *s, char *t)
{
    if (s[0]==t[0]) {
        // do something
    }
}

int main() {
    char a[] = "testing";
    int x = strcmp(a,a);

    a[3] = 'b';
    char v = a[2];
}
