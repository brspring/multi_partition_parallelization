# Paralelização da Função Multi-Partition com Pthreads e Pool de Threads

Este projeto implementa a paralelização da função multi_partition utilizando Pthreads e pool de threads, com foco na eficiência de particionamento de dados em grandes vetores. A função divide um vetor de entrada (Input) em múltiplas faixas definidas por um vetor de partições (P) e retorna os vetores particionados (Output) e as posições de início de cada faixa (Pos).

O projeto também realiza medições de desempenho em diferentes números de threads e analisa os resultados, considerando efeitos de cache, para garantir medidas confiáveis.
Funcionalidades
    - Implementação paralela com pool de threads.
    - Geração de gráficos e planilhas para análise de desempenho.
    - Scripts para execução no cluster w00 (xeon) com Slurm.
    - Verificação automática do particionamento com a função verifica_particoes.
