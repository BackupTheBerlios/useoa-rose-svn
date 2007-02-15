int main() {
    int A[100];
    int i;
    int j;

    for(i = 0; i < 10; i++) {
        A[i*2] = A[2*i] * 2 + 1;
    }

    return 1;
}
