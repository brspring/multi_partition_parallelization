#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>

#define MAX_THREADS 4

// int multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
//     long long pMin, pMax = 0;
//     long long count = 0;

//     for(int j=0; j<np; j++) {
//         Pos[j] = count;
//         for(int i=0; i<n; i++) {
//             if(j == 0)
//                 pMin = 0;
//             else
//                 pMin = P[j-1];
//             pMax = P[j];
//             if(Input[i] < pMax && Input[i] >= pMin) {
//                 Output[count] = Input[i];
//                 count++;
//             }
//         }
//     }
// }

long long geraAleatorioLL() {
    int a = rand() % 10;  // Returns a pseudo-random integer
                     //    between 0 and RAND_MAX.
    int b = rand() % 10;  // same as above
    long long v = (long long)a * 100 + b;
    return v;
}

void printOutput(long long *Output, int n) {
    for(int i=0; i<n; i++) {
        printf("%lld ", Output[i]);
    }
    printf("\n");
}

void printPos(int *Pos, int np) {
    for(int i=0; i<np; i++) {
        printf("%d ", Pos[i]);
    }
    printf("\n");
}

typedef struct {
    long long *Input;
    int n;
    long long *P;
    int np;
    long long *Output;
    int *Pos;
    int start;
    int end;
} PartitionArgs;

pthread_mutex_t mutexCount = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexPos = PTHREAD_MUTEX_INITIALIZER;
long long count = 0;

void *partition_thread(void *args) {
    PartitionArgs *data = (PartitionArgs *)args;

    for (int j = data->start; j < data->end; j++) {
        long long pMin = (j == 0) ? 0 : data->P[j - 1];
        long long pMax = data->P[j];

        pthread_mutex_lock(&mutexPos);
        data->Pos[j] = count;
        pthread_mutex_unlock(&mutexPos);

        for (int i = 0; i < data->n; i++) {
            if (data->Input[i] >= pMin && data->Input[i] < pMax) {
                pthread_mutex_lock(&mutexCount);
                data->Output[count] = data->Input[i];
                count++;
                pthread_mutex_unlock(&mutexCount);
            }
        }
    }

    return NULL;
}

void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
    pthread_t threads[MAX_THREADS];
    PartitionArgs threadData[MAX_THREADS];

    int partitionsPerThread = np / MAX_THREADS;
    int remainder = np % MAX_THREADS;

    int currentPartition = 0;

    for (int t = 0; t < MAX_THREADS; t++) {
        int start = currentPartition;
        int end = start + partitionsPerThread + (t < remainder ? 1 : 0);

        threadData[t] = (PartitionArgs){
            .Input = Input,
            .n = n,
            .P = P,
            .np = np,
            .Output = Output,
            .Pos = Pos,
            .start = start,
            .end = end,
        };

        pthread_create(&threads[t], NULL, partition_thread, &threadData[t]);
        currentPartition = end;
    }

    for (int t = 0; t < MAX_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }
}

long long *geraVetor(long long n) {
    long long *vetor = malloc(sizeof(long long)*n);
    if(!vetor)
        return NULL;
    
    for(int i=0; i<n; i++) {
        vetor[i] = geraAleatorioLL();
    }

    return vetor;
}

int main () {
    srand(time(NULL));
    long long Input[100] = {8, 4, 13, 7, 11, 100, 44, 3, 7, 7, 100, 110, 46, 44};
    int n = 14;   // n == numero de elementos em Input e em Output
    long long P[20] = {12, 70, 90, LLONG_MAX};   // OBS: incluir <limits.h> que tem LLONG_MAX
    int np = 4;   // np == numero de faixas no vetor P
    long long Output[100];
    int Pos[20];

    long long n2 = 14;
    long long *Input2 = geraVetor(n2);
    long long *P2 = geraVetor(4);
    long long *Output2 = geraVetor(n2);

    multi_partition(Input2, n2, P2, np, Output2, Pos);

    printOutput(Input2, n2);
    printOutput(P2, np);
    printOutput(Output2, n2);
    printPos(Pos, np);
    return 0;
}