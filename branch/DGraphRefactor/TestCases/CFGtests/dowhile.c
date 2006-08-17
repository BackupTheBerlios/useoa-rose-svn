/* dowhile.c */

int main() {
    int i;
    double x,y,f;

    i=0;
    f = 0.0;
    do {
        x = 3*i;
        y = 2*x;
        f += x + y;
        i++;
    } while (i<20);
}
