/* while.c */

int main() {
    int i;
    double x,y,f;

    i=0;
    f = 0.0;
    while (i<20) {
        x = 3*i;
        y = 2*x;
        f += x + y;
        i++;
    }
}
