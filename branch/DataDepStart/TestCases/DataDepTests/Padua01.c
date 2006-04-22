int main() {
    int A[100];
    int i;

    for(i = 0; i < 100; i++) {
        A[2*i] = i;
        i = A[2*i+1];
    }
}
