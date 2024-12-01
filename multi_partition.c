#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include "multi_partition.h"

pthread_mutex_t mutexPos = PTHREAD_MUTEX_INITIALIZER;

void *partition_thread(void *args) {
    PartitionArgs *data = (PartitionArgs *)args;

    long long pMin, pMax;
    long long start_index = data->Pos[data->start]; // Início da região que a thread deve preencher
    long long offset = 0;

    for (long long j = data->start; j < data->end; j++) {
        pMin = (j == 0) ? LLONG_MIN : data->P[j - 1];
        pMax = data->P[j];

        // Ignorar partições inválidas
        if (pMin == pMax) {
            continue;
        }

        for (long long i = 0; i < data->n; i++) {
            if (data->Input[i] >= pMin && data->Input[i] < pMax) {
                data->Output[start_index + offset] = data->Input[i];
                offset++;
            }
        }
    }

    pthread_exit(NULL);
}

void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
    pthread_t threads[MAX_THREADS];
    PartitionArgs threadData[MAX_THREADS];

    long long partitionsPerThread = np / MAX_THREADS;
    long long remainder = np % MAX_THREADS;

    // Calcular o vetor Pos
    long long count = 0;
    for (int j = 0; j < np; j++) {
        Pos[j] = count;
        long long pMin = (j == 0) ? LLONG_MIN : P[j - 1];
        long long pMax = P[j];

        for (int i = 0; i < n; i++) {
            if (Input[i] >= pMin && Input[i] < pMax) {
                count++;
            }
        }
    }

    // Dividir trabalho entre as threads
    long long currentPartition = 0;

    for (int t = 0; t < MAX_THREADS; t++) {
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

    for (int t = 0; t < MAX_THREADS; t++) {
        if (pthread_join(threads[t], NULL) != 0) {
            perror("Erro ao aguardar thread");
            exit(EXIT_FAILURE);
        }
    }
}
