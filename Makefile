CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -lpthread

# Nome do executável
all: mpp

# Regra para compilar o executável
mpp: mpp.o verifica_particoes.o
	$(CC) $(CFLAGS) -o mpp mpp.o verifica_particoes.o $(LDFLAGS)

# Regra para compilar mpp.o
mpp.o: mpp.c verifica_particoes.h multi_partition.h
	$(CC) $(CFLAGS) -c mpp.c

# Regra para compilar verifica_particoes.o
verifica_particoes.o: verifica_particoes.c verifica_particoes.h
	$(CC) $(CFLAGS) -c verifica_particoes.c

# Regra para compilar multi_partition.o
multi_partition.o: multi_partition.c multi_partition.h
	$(CC) $(CFLAGS) -c multi_partition.c

# Regra para rodar o programa
run:
	./mpp

# Limpeza dos arquivos objeto e do executável
clean:
	rm -f *.o mpp
