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
    long long a = rand() % 10;  // Returns a pseudo-random integer
                     //    between 0 and RAND_MAX.
    long long b = rand() % 10;  // same as above
    long long v = (long long)a * 100 + b;
    return v;
}

void printVetor(long long *Output, long long n) {
    for(long long i=0; i<n; i++) {
        printf("%lld ", Output[i]);
    }
    printf("\n");
}

typedef struct {
    long long *Input;
    long long n;
    long long *P;
    long long np;
    long long *Output;
    long long *Pos;
    long long start;
    long long end;
} PartitionArgs;

pthread_mutex_t mutexCount = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexPos = PTHREAD_MUTEX_INITIALIZER;
long long count = 0;

void *partition_thread(void *args) {
    PartitionArgs *data = (PartitionArgs *)args;

    for (long long j = data->start; j < data->end; j++) {
        long long pMin = (j == 0) ? 0 : data->P[j - 1];
        long long pMax = data->P[j];

        pthread_mutex_lock(&mutexPos);
        data->Pos[j] = count;
        pthread_mutex_unlock(&mutexPos);

        for (long long i = 0; i < data->n; i++) {
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

void multi_partition(long long *Input, long long n, long long *P, long long np, long long *Output, long long *Pos) {
    pthread_t threads[MAX_THREADS];
    PartitionArgs threadData[MAX_THREADS];

    long long partitionsPerThread = np / MAX_THREADS;
    long long remainder = np % MAX_THREADS;

    long long currentPartition = 0;

    for (long long t = 0; t < MAX_THREADS; t++) {
        long long start = currentPartition;
        long long end = start + partitionsPerThread + (t < remainder ? 1 : 0);

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

    for (long long t = 0; t < MAX_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }
}

int compara(const void *a, const void *b) {
    long long valA = *(const long long *)a;
    long long valB = *(const long long *)b;
    if (valA < valB) return -1;
    if (valA > valB) return 1;
    return 0;
}

// Função para gerar o vetor
long long *geraVetor(long long n, int ordena) {
    long long *vetor = malloc(sizeof(long long) * n);
    if (!vetor)
        return NULL;
    
    for (long long i = 0; i < n; i++) {
        vetor[i] = geraAleatorioLL(); // Supondo que essa função existe
    }
    
    if (ordena) {
        qsort(vetor, n, sizeof(long long), compara);
    }
    
    return vetor;
}

int main () {
    srand(time(NULL));
    long long n = 8000000;
    long long np = 1000;
    long long *Input = geraVetor(n, 0);
    long long *P = geraVetor(np, 1);
    long long *Output = geraVetor(n, 0);
    long long *Pos = geraVetor(np, 0);
    
    multi_partition(Input, n, P, np, Output, Pos);

    // printVetor(Input, n);
    // printVetor(P, np);
    // printVetor(Output, n);
    // printVetor(Pos, np);
    return 0;
}