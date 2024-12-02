#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include "verifica_particoes.h"
#define NTIMES 10
#define RAND_MAX_CUSTOM 5
#define MAX_THREADS 4
int nThreads;
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
    pthread_t threads[nThreads];
    PartitionArgs threadData[nThreads];
    long long partitionsPerThread = np / nThreads;
    long long remainder = np % nThreads;
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
    for (int t = 0; t < nThreads; t++) {
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
    for (int t = 0; t < nThreads; t++) {
        long long minThreadId = -1;

        // Acha a thread que terminou primeiro
        for (int i = 0; i < nThreads; i++) {
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

long long geraAleatorioLL() {
    long long a = rand();
    long long b = rand();
    long long v = (long long)a * 100 + b;
    return v;
}

void printVetor(long long *Output, int n) {
    for(int i=0; i<n; i++) {
        printf("%lld ", Output[i]);
    }
    printf("\n");
}

void printVetorPos(int *Pos, int n) {
    for(int i=0; i<n; i++) {
        printf("%d ", Pos[i]);
    }
    printf("\n");
}

int compara(const void *a, const void *b) {
    long long valA = *(const long long *)a;
    long long valB = *(const long long *)b;
    if (valA < valB) return -1;
    if (valA > valB) return 1;
    return 0;
}

// Função para gerar o vetor
long long *geraVetor(int n, int ordena) {
    long long *vetor = malloc(sizeof(long long) * n);
    if (!vetor)
        return NULL;
    
    for (long long i = 0; i < n; i++) {
        vetor[i] = geraAleatorioLL(); // Supondo que essa função existe
    }
    
    if (ordena) {
        qsort(vetor, n, sizeof(long long), compara);
        vetor[n-1] = LLONG_MAX;
    }
    
    return vetor;
}

// Função para gerar o vetor Pos
int *geraVetorPos(int n) {
    int *vetor = malloc(sizeof(int) * n);
    if (!vetor)
        return NULL;
    
    for (int i = 0; i < n; i++) {
        vetor[i] = geraAleatorioLL(); // Supondo que essa função existe
    }    
    return vetor;
}

int main (int argc, char *argv[]) {
    // Recebe o número de threads pelo argv
    if (argc != 2) {
        printf("Uso: %s <n> <np>\n", argv[0]);
        return 1;
    }
    nThreads = atoi(argv[1]);
    int n = 8000000;
    int np = 10;
    srand(time(NULL));
    
    // Abre arquivo
    FILE *file = fopen("result.csv", "a");
    if (!file) {
        return 1;
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    if(fileSize == 0)
        fprintf(file, "threads;tempo;meps;parte\n");
    fseek(file, 0, SEEK_SET);

    for(int i=0; i<NTIMES; i++) {
        long long *Input = geraVetor(n, 0);
        long long *P = geraVetor(np, 1);
        long long *Output = geraVetor(n, 0);
        int *Pos = geraVetorPos(np);
        if(!Output || !Pos){
            fprintf(stderr, "Falha na alocação de memória\n");
            free(Input);
            free(P);
            return 1;
        }

        struct timespec start, end;
        printf("--- Executando o multi_partition ---\n");
        printf("n = %d\nnp = %d\n", n, np);

        clock_gettime(CLOCK_REALTIME, &start);
        multi_partition(Input, n, P, np, Output, Pos);
        clock_gettime(CLOCK_REALTIME, &end);
        // Imprime o tempo de execução
        double tempo = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

        printf("Tempo de execução: %f segundos\n", tempo);
        fprintf(file, "%d;%f;%f;A\n", nThreads, tempo, n/tempo);
        // Verificação do particionamento
        printf("--- Verificando partições ---\n");
        verifica_particoes(Input, n, P, np, Output, Pos);
    }
    // printf("Vetor P = ");
    // printVetor(P, np);
    // printf("Vetor Pos = ");
    // printVetorPos(Pos, np);
    // printf("Vetor Input = ");
    // printVetor(Input, n);
    // printf("Vetor Output = ");
    // printVetor(Output, n);

    // Limpeza da memória alocada
    // free(Input);
    // free(P);
    // free(Output);
    // free(Pos);
    return 0;
}
