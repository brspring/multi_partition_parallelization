#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include "verifica_particoes.h"
#include "multi_partition.h"

#define NTIMES 10
#define RAND_MAX_CUSTOM 5

long long geraAleatorioLL() {
    long long a = rand() % RAND_MAX_CUSTOM;
    long long b = rand() % RAND_MAX_CUSTOM;
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
    // Verifica se os argumentos foram passados corretamente
    if (argc != 3) {
        printf("Uso: %s <n> <np>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);  // Converte o argumento para inteiro
    int np = atoi(argv[2]);
    srand(time(NULL));
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