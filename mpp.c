#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

long long geraAleatorioLL() {
    int a = rand();  // Returns a pseudo-random integer
                     //    between 0 and RAND_MAX.
    int b = rand();  // same as above
    long long v = (long long)a * 100 + b;
    return v;
}

void printOutput(long long *Output, int n) {
    for(int i=0; i<n; i++) {
        printf("%d ", Output[i]);
    }
    printf("\n");
}

void printPos(int *Pos, int np) {
    for(int i=0; i<np; i++) {
        printf("%d ", Pos[i]);
    }
    printf("\n");
}

int multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
    long long pMin, pMax = 0;
    long long count = 0;
    for(int j=0; j<np; j++) {
        Pos[j] = count;
        for(int i=0; i<n; i++) {
            if(j == 0)
                pMin = 0;
            else
                pMin = P[j-1];
            pMax = P[j];
            if(Input[i] < pMax && Input[i] >= pMin) {
                Output[count] = Input[i];
                count++;
            }
        }
    }
}

int main () {
    long long Input[100] = {8, 4, 13, 7, 11, 100, 44, 3, 7, 7, 100, 110, 46, 44};
    int n = 14;   // n == numero de elementos em Input e em Output
    long long P[20] = {12, 70, 90, LLONG_MAX};   // OBS: incluir <limits.h> que tem LLONG_MAX
    int np = 4;   // np == numero de faixas no vetor P
    long long Output[100];
    int Pos[20];
    multi_partition(Input, n, P, np, Output, Pos);
    printOutput(Output, n);
    printPos(Pos, np);
    return 0;
}