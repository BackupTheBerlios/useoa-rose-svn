/* while.c */

int main() {
    int i,j;
    double x,y,f;

    f = 0.0;
    for (i=0; i<20; i++) {
        for (j=0; j<20; j++) {
            x = 3*i;
            y = 2*j;
            f += x + y;
        }
    }
}

