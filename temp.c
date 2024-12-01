#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include "multi_partition.h"

pthread_mutex_t mutexPos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCount = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexOutput = PTHREAD_MUTEX_INITIALIZER;
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

void *partition_thread(void *args) {
    PartitionArgs *data = (PartitionArgs *)args;

    // Alocar buffer local
    long long buffer_size = data->n / (data->end - data->start) + 1;
    long long *localBuffer = malloc(sizeof(long long) * buffe_size);
    if (!localBuffer) {
        perror("Erro ao alocar buffer local");
        pthread_exit(NULL);
    }

    long long local_count = 0;

    for (long long j = data->start; j < data->end; j++) {
        long long pMin = (j == 0) ? LLONG_MIN : data->P[j - 1];
        long long pMax = data->P[j];

        // Ignorar partições inválidas
        if (pMin == pMax) {
            continue;
        }

        for (long long i = 0; i < data->n; i++) {
            if (data->Input[i] >= pMin && data->Input[i] < pMax) {
                localBuffer[local_count++] = data->Input[i];
            }
        }
    }

    // Atualizar `Output`
    pthread_mutex_lock(&mutexCount);
    long long start_index = *data->global_count;
    *data->global_count += local_count;
    pthread_mutex_unlock(&mutexCount);

    pthread_mutex_lock(&mutexOutput);
    for (long long i = 0; i < local_count; i++) {
        data->Output[start_index + i] = localBuffer[i];
    }
    pthread_mutex_unlock(&mutexOutput);

    free(localBuffer);
    pthread_exit(NULL);
}

void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
    pthread_t threads[MAX_THREADS];
    PartitionArgs threadData[MAX_THREADS];

    long long global_count = 0;
    long long partitionsPerThread = np / MAX_THREADS;
    long long remainder = np % MAX_THREADS;

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
            .start = start,
            .end = end,
            .global_count = &global_count
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

    // Atualizar vetor Pos
    long long offset = 0;
    for (int j = 0; j < np; j++) {
        Pos[j] = offset;
        while (offset < global_count && Output[offset] < P[j]) {
            offset++;
        }
    }
}
