/* CFG for the for-loop */

int main() {
    int i;
    double x,y,f;

    f = 0.0;
    for (i=0; i<20; i++) {
        x = 3*i;
        y = 2*x;
        f += x + y;
    }
}

