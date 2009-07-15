int main() {
    int A[100];
    int i;

    for(i = 0; i < 10; i++) {
        A[i] = 1 + 2 * A[i-1];
    }
}
