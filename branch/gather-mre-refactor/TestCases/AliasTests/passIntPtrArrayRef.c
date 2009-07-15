//#include <stdlib.h>

int blah( int *s, int *t)
{
    if (s[0]==t[0]) {
        // do something
    }
    return 0;
}

int main() {
//    int *a = (int*)malloc(sizeof(int)*4);
    int a;
    int x = blah(&a,&a);
}
