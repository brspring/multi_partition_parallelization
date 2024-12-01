#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include "multi_partition.h"

pthread_mutex_t mutexPos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCount = PTHREAD_MUTEX_INITIALIZER;
long long count = 0;

void multi_partition_simple(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
    long long pMin, pMax = 0;
    long long count = 0;

    for(int j=0; j<np; j++) {
        Pos[j] = count;
        pMin = (j==0) ? 0 : P[j-1];
        pMax = P[j];
        for(int i=0; i<n; i++) {
            if(Input[i] >= pMin && Input[i] < pMax) {
                Output[count] = Input[i];
                count++;
            }
        }
    }
}

// void *partition_thread(void *args) {
//     PartitionArgs *data = (PartitionArgs *)args;

//     for (long long j = data->start; j < data->end; j++) {
//         long long pMin = (j == 0) ? 0 : data->P[j - 1];
//         long long pMax = data->P[j];

//         pthread_mutex_lock(&mutexPos);
//         data->Pos[j] = count;
//         pthread_mutex_unlock(&mutexPos);

//         for (long long i = 0; i < data->n; i++) {
//             if (data->Input[i] >= pMin && data->Input[i] < pMax) {
//                 pthread_mutex_lock(&mutexCount);
//                 data->Output[count] = data->Input[i];
//                 count++;
//                 pthread_mutex_unlock(&mutexCount);
//             }
//         }
//     }

//     return NULL;
// }

// void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
//     pthread_t threads[MAX_THREADS];
//     PartitionArgs threadData[MAX_THREADS];

//     long long partitionsPerThread = np / MAX_THREADS;
//     long long remainder = np % MAX_THREADS;

//     long long currentPartition = 0;

//     for (long long t = 0; t < MAX_THREADS; t++) {
//         long long start = currentPartition;
//         long long end = start + partitionsPerThread + (t < remainder ? 1 : 0);

//         threadData[t] = (PartitionArgs){
//             .Input = Input,
//             .n = n,
//             .P = P,
//             .np = np,
//             .Output = Output,
//             .Pos = Pos,
//             .start = start,
//             .end = end,
//         };

//         pthread_create(&threads[t], NULL, partition_thread, &threadData[t]);
//         currentPartition = end;
//     }

//     for (long long t = 0; t < MAX_THREADS; t++) {
//         pthread_join(threads[t], NULL);
//     }
// }

// Função que será executada por cada thread
void *partition_thread(void *args) {
    PartitionArgs *data = (PartitionArgs *)args;
    
    // Contador local para cada thread
    long long local_count = 0;
    
    for (long long j = data->start; j < data->end; j++) {
        long long pMin = (j == 0) ? 0 : data->P[j - 1];
        long long pMax = data->P[j];

        // Atualiza a posição de forma sincronizada
        pthread_mutex_lock(&mutexPos);
        data->Pos[j] = *data->global_count + local_count; // A posição é calculada com base no contador global + local
        pthread_mutex_unlock(&mutexPos);

        // Processa os elementos da partição
        for (long long i = 0; i < data->n; i++) {
            if (data->Input[i] >= pMin && data->Input[i] < pMax) {
                // Preenche o vetor de saída
                data->Output[*data->global_count + local_count] = data->Input[i];
                local_count++;  // Incrementa o contador local
            }
        }
    }

    // Atualiza o contador global de forma sincronizada
    pthread_mutex_lock(&mutexCount);
    *data->global_count += local_count;
    pthread_mutex_unlock(&mutexCount);
    printf("start = %lld, end = %lld\n", data->start, data->end);

    return NULL;
}

// Função principal para dividir o trabalho entre as threads
void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
    pthread_t threads[MAX_THREADS];
    PartitionArgs threadData[MAX_THREADS];

    // Variável global de contagem, compartilhada entre todas as threads
    long long global_count = 0;

    long long partitionsPerThread = np / MAX_THREADS;
    long long remainder = np % MAX_THREADS;

    long long currentPartition = 0;

    // Criação das threads
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
            .local_count = &global_count, // Contador local
            .global_count = &global_count, // Contador global compartilhado
        };

        pthread_create(&threads[t], NULL, partition_thread, &threadData[t]);
        currentPartition = end;
    }

    // Espera todas as threads terminarem
    for (long long t = 0; t < MAX_THREADS; t++) {
        pthread_join(threads[t], NULL);
    }
}