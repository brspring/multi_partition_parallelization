#ifndef MULTI_PARTITION_H
#define MULTI_PARTITION_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>

// Definição de MAX_THREADS
#define MAX_THREADS 4

typedef struct {
    long long *buffer;
    long long count;
} ThreadBuffer;

// Definições de mutexes para sincronização
extern pthread_mutex_t mutexCount;
extern pthread_mutex_t mutexPos;
extern long long count;

// Declaração da função para particionar de forma simples (sem threads)
void multi_partition_simple(long long *Input, int n, long long *P, int np, long long *Output, int *Pos);

// Declaração da função para particionar com threads
void *partition_thread(void *args);

// Declaração da função para particionar em múltiplas threads
void multi_partition(long long *Input, int n, long long *P, int np, long long *Output, int *Pos);

#endif
