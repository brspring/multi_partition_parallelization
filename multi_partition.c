#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include "multi_partition.h"

// Estrutura para os argumentos passados para cada thread
typedef struct {
    long long *Input;
    int n;
    long long *P;
    int np;
    long long *Output;
    int *Pos;
    long long start;
    long long end;
    long long *Output_local;
    long long local_count;
    long long thread_id; // Identificador único da thread
} PartitionArgs;

// Função que calcula a partição e preenche o vetor local
void *partition_thread(void *args) {
    PartitionArgs *data = (PartitionArgs *)args;
    long long pMin, pMax;
    long long count = 0;

    // Preenche o vetor local de saída
    for (long long j = data->start; j < data->end; j++) {
        pMin = (j == 0) ? LLONG_MIN : data->P[j - 1];
        pMax = data->P[j];

        // Calcula o número de elementos no intervalo [pMin, pMax] e preenche o vetor local
        for (long long i = 0; i < data->n; i++) {
            if (data->Input[i] >= pMin && data->Input[i] < pMax) {
                data->Output_local[count++] = data->Input[i];  // Armazena o valor no vetor local
            }
        }
    }

    data->local_count = count; // Armazena o número de elementos encontrados pela thread
    pthread_exit(NULL);
}

// Função principal de partição
void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos) {
    pthread_t threads[MAX_THREADS];
    PartitionArgs threadData[MAX_THREADS];
    long long partitionsPerThread = np / MAX_THREADS;
    long long remainder = np % MAX_THREADS;
    long long offset = 0;
    long long currentPos = 0;
    long long soma = 0;
    long long start;
    long long end;
    for(int i=0; i<np; i++) {
        Pos[i] = 0;
    }
    // Dividir o trabalho entre as threads
    long long currentPartition = 0;
    for (int t = 0; t < MAX_THREADS; t++) {
        start = currentPartition;
        end = start + partitionsPerThread + (t < remainder ? 1 : 0);

        threadData[t] = (PartitionArgs){
            .Input = Input,
            .n = n,
            .P = P,
            .np = np,
            .Output = Output,
            .Pos = Pos,
            .start = start,
            .end = end,
            .Output_local = (long long *)malloc(sizeof(long long) * n), // Aloca espaço para a saída local
            .local_count = 0,
            .thread_id = t // Atribui o id da thread
        };

        // Criar thread para calcular a partição
        if (pthread_create(&threads[t], NULL, partition_thread, &threadData[t]) != 0) {
            perror("Erro ao criar thread");
            exit(EXIT_FAILURE);
        }

        currentPartition = end;
    }
    // Espera todas as threads terminarem e processa os resultados na ordem que terminaram
    Pos[currentPos] = 0;
    for (int t = 0; t < MAX_THREADS; t++) {
        long long minThreadId = -1;

        // Acha a thread que terminou primeiro
        for (int i = 0; i < MAX_THREADS; i++) {
            if (threads[i] == 0) continue; // Ignora threads já processadas
            if (minThreadId == -1 || threadData[i].thread_id < minThreadId) {
                minThreadId = threadData[i].thread_id;
            }
        }

        // Processa a thread que terminou
        pthread_join(threads[minThreadId], NULL);  // Espera a thread com menor id

        // Preenche o vetor Output com os resultados locais
        for (long long i = 0; i < threadData[minThreadId].local_count; i++) {
            Output[offset++] = threadData[minThreadId].Output_local[i];
            start = (currentPos==0) ? 0 : P[currentPos-1];

            while(!(Output[offset-1] >= start && Output[offset-1] < P[currentPos])) {
                currentPos++;
                Pos[currentPos] = soma;
            }
            soma++;
        }
        // Marca a thread como processada
        threads[minThreadId] = 0;
    }
}
