int main() {
    int A[100];
    int B[100];
    int C[100];
    int E[100];

    for(int i = 0 ; i < 10; i++) {
        A[i] = B[i];
        C[i] = A[i] + B[i - 1];
        E[i] = C[i + 1];
        B[i] = C[i] + 2;
    }

}
