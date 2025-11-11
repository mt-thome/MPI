#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Estados possíveis
#define VAZIO 0
#define SAUDAVEL 1
#define CONTAMINADO -1
#define MORTO -2
#define MORTO_ANTERIOR -3

typedef struct {
    int **matriz;
    int N;  
    int M;  
    int linhas_locais;  // linhas que este processo controla
    int inicio_linha;   // primeira linha deste processo na matriz global
} Regiao;

// Função para alocar a matriz
int** alocar_matriz(int N, int M) {
    int **matriz = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        matriz[i] = (int *)malloc(M * sizeof(int));
    }
    return matriz;
}

// Função para liberar a matriz
void liberar_matriz(int **matriz, int N) {
    for (int i = 0; i < N; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

// Função para ler a região do arquivo (apenas processo 0)
Regiao ler_entrada(const char *arquivo) {
    FILE *fp = fopen(arquivo, "r");
    if (!fp) {
        fprintf(stderr, "Erro ao abrir arquivo %s\n", arquivo);
        exit(1);
    }
    
    Regiao regiao;
    fscanf(fp, "%d %d", &regiao.N, &regiao.M);
    
    regiao.matriz = alocar_matriz(regiao.N, regiao.M);
    
    for (int i = 0; i < regiao.N; i++) {
        for (int j = 0; j < regiao.M; j++) {
            fscanf(fp, "%d", &regiao.matriz[i][j]);
        }
    }
    
    fclose(fp);
    return regiao;
}

// Verifica se há contaminado ou morto na vizinhança
int tem_contaminado_vizinho(int **matriz, int linhas, int M, int i, int j) {
    // Vizinho acima
    if (i > 0 && (matriz[i-1][j] == CONTAMINADO || matriz[i-1][j] == MORTO || matriz[i-1][j] == MORTO_ANTERIOR))
        return 1;
    // Vizinho abaixo
    if (i < linhas-1 && (matriz[i+1][j] == CONTAMINADO || matriz[i+1][j] == MORTO || matriz[i+1][j] == MORTO_ANTERIOR))
        return 1;
    // Vizinho esquerda
    if (j > 0 && (matriz[i][j-1] == CONTAMINADO || matriz[i][j-1] == MORTO || matriz[i][j-1] == MORTO_ANTERIOR))
        return 1;
    // Vizinho direita
    if (j < M-1 && (matriz[i][j+1] == CONTAMINADO || matriz[i][j+1] == MORTO || matriz[i][j+1] == MORTO_ANTERIOR))
        return 1;
    
    return 0;
}

// Aplica as regras da simulação na região local
void simular_iteracao_local(int **matriz, int **nova_matriz, int linhas, int M) {
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < M; j++) {
            int estado_atual = matriz[i][j];
            
            if (estado_atual == VAZIO) {
                nova_matriz[i][j] = VAZIO;
            }
            else if (estado_atual == SAUDAVEL) {
                if (tem_contaminado_vizinho(matriz, linhas, M, i, j)) {
                    nova_matriz[i][j] = CONTAMINADO;
                } else {
                    nova_matriz[i][j] = SAUDAVEL;
                }
            }
            else if (estado_atual == CONTAMINADO) {
                int prob = rand() % 10000;
                if (prob < 1000) {
                    nova_matriz[i][j] = SAUDAVEL;
                } else if (prob < 4000) {
                    nova_matriz[i][j] = CONTAMINADO;
                } else {
                    nova_matriz[i][j] = MORTO;
                }
            }
            else if (estado_atual == MORTO) {
                nova_matriz[i][j] = MORTO_ANTERIOR;
            }
            else if (estado_atual == MORTO_ANTERIOR) {
                nova_matriz[i][j] = VAZIO;
            }
        }
    }
}

// Conta população local
void contar_populacao_local(int **matriz, int linhas, int M, int *saudaveis, int *contaminados, int *mortos) {
    *saudaveis = 0;
    *contaminados = 0;
    *mortos = 0;
    
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < M; j++) {
            if (matriz[i][j] == SAUDAVEL) {
                (*saudaveis)++;
            } else if (matriz[i][j] == CONTAMINADO) {
                (*contaminados)++;
            } else if (matriz[i][j] == MORTO || matriz[i][j] == MORTO_ANTERIOR) {
                (*mortos)++;
            }
        }
    }
}

// Salva resultado
void salvar_resultado(const char *arquivo, int total_mortos, int total_sobreviventes, double tempo) {
    FILE *fp = fopen(arquivo, "w");
    if (!fp) {
        fprintf(stderr, "Erro ao criar arquivo de saída\n");
        return;
    }
    
    fprintf(fp, "Total de mortos: %d\n", total_mortos);
    fprintf(fp, "Total de sobreviventes: %d\n", total_sobreviventes);
    fprintf(fp, "Tempo de execução: %.6f segundos\n", tempo);
    
    fclose(fp);
}

int main(int argc, char *argv[]) {
    int rank, size;
    
    // Inicializa MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <arquivo_entrada> [arquivo_saida]\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    const char *arquivo_entrada = argv[1];
    const char *arquivo_saida = argc > 2 ? argv[2] : "resultado_mpi.txt";
    
    // Inicializa gerador de números aleatórios (diferente para cada processo)
    srand(time(NULL) + rank);
    
    int N, M;
    int **matriz_global = NULL;
    
    double tempo_inicio = MPI_Wtime();
    
    // Processo 0 lê a entrada
    if (rank == 0) {
        Regiao regiao = ler_entrada(arquivo_entrada);
        N = regiao.N;
        M = regiao.M;
        matriz_global = regiao.matriz;
        
        printf("Iniciando simulação MPI com %d processos...\n", size);
        printf("Região: %dx%d\n", N, M);
        printf("Máximo de iterações: %d\n\n", N * M);
    }
    
    // Broadcast das dimensões para todos os processos
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    // Calcula quantas linhas cada processo vai processar
    int linhas_por_processo = N / size;
    int linhas_extras = N % size;
    
    // Cada processo tem linhas_por_processo + (1 se rank < linhas_extras)
    int linhas_locais = linhas_por_processo + (rank < linhas_extras ? 1 : 0);
    
    // Calcula onde cada processo começa
    int inicio_linha = rank * linhas_por_processo + (rank < linhas_extras ? rank : linhas_extras);
    
    // Aloca matriz local (com linhas de halo para vizinhos)
    int linhas_com_halo = linhas_locais + 2;  // +2 para linhas de halo
    int **matriz_local = alocar_matriz(linhas_com_halo, M);
    int **nova_matriz_local = alocar_matriz(linhas_com_halo, M);
    
    // Preparar para scatter
    int *sendcounts = NULL;
    int *displs = NULL;
    int *recvbuf = (int *)malloc(linhas_locais * M * sizeof(int));
    
    if (rank == 0) {
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; i++) {
            int linhas = linhas_por_processo + (i < linhas_extras ? 1 : 0);
            sendcounts[i] = linhas * M;
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }
    
    int max_iteracoes = N * M;
    int iteracao = 0;
    int continuar = 1;
    
    while (continuar && iteracao < max_iteracoes) {
        // Distribui dados do processo 0 para todos
        if (rank == 0) {
            // Converte matriz 2D em array 1D para scatter
            int *buffer = (int *)malloc(N * M * sizeof(int));
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    buffer[i * M + j] = matriz_global[i][j];
                }
            }
            MPI_Scatterv(buffer, sendcounts, displs, MPI_INT, recvbuf, 
                        linhas_locais * M, MPI_INT, 0, MPI_COMM_WORLD);
            free(buffer);
        } else {
            MPI_Scatterv(NULL, NULL, NULL, MPI_INT, recvbuf, 
                        linhas_locais * M, MPI_INT, 0, MPI_COMM_WORLD);
        }
        
        // Copia para matriz local (índice 1 a linhas_locais, deixando 0 e linhas_locais+1 para halo)
        for (int i = 0; i < linhas_locais; i++) {
            for (int j = 0; j < M; j++) {
                matriz_local[i + 1][j] = recvbuf[i * M + j];
            }
        }
        
        // Troca linhas de halo com vizinhos
        MPI_Request requests[4];
        int req_count = 0;
        
        // Enviar linha superior para processo anterior e receber do processo anterior
        if (rank > 0) {
            MPI_Isend(matriz_local[1], M, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Irecv(matriz_local[0], M, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            // Primeira linha não tem vizinho acima
            for (int j = 0; j < M; j++) {
                matriz_local[0][j] = VAZIO;
            }
        }
        
        // Enviar linha inferior para próximo processo e receber do próximo processo
        if (rank < size - 1) {
            MPI_Isend(matriz_local[linhas_locais], M, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Irecv(matriz_local[linhas_locais + 1], M, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            // Última linha não tem vizinho abaixo
            for (int j = 0; j < M; j++) {
                matriz_local[linhas_locais + 1][j] = VAZIO;
            }
        }
        
        MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        
        // Simula a iteração localmente
        simular_iteracao_local(matriz_local, nova_matriz_local, linhas_com_halo, M);
        
        // Copia resultado de volta (ignorando linhas de halo)
        for (int i = 0; i < linhas_locais; i++) {
            for (int j = 0; j < M; j++) {
                recvbuf[i * M + j] = nova_matriz_local[i + 1][j];
            }
        }
        
        // Reúne resultados no processo 0
        if (rank == 0) {
            int *buffer = (int *)malloc(N * M * sizeof(int));
            MPI_Gatherv(recvbuf, linhas_locais * M, MPI_INT, buffer, sendcounts, displs, 
                       MPI_INT, 0, MPI_COMM_WORLD);
            
            // Converte buffer de volta para matriz 2D
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    matriz_global[i][j] = buffer[i * M + j];
                }
            }
            free(buffer);
        } else {
            MPI_Gatherv(recvbuf, linhas_locais * M, MPI_INT, NULL, NULL, NULL, 
                       MPI_INT, 0, MPI_COMM_WORLD);
        }
        
        iteracao++;
        
        // Processo 0 verifica se deve continuar
        if (rank == 0) {
            int saudaveis, contaminados, mortos;
            contar_populacao_local(matriz_global, N, M, &saudaveis, &contaminados, &mortos);
            
            continuar = (contaminados > 0);
            
            if (iteracao % 100 == 0 || iteracao == 1) { 
                printf("Iteração %d: Saudáveis=%d, Contaminados=%d, Mortos=%d\n", iteracao, saudaveis, contaminados, mortos);
            }
        }
        
        // Broadcast da decisão de continuar
        MPI_Bcast(&continuar, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    
    double tempo_fim = MPI_Wtime();
    double tempo_execucao = tempo_fim - tempo_inicio;
    
    // Processo 0 salva resultado final
    if (rank == 0) {
        int saudaveis, contaminados, mortos;
        contar_populacao_local(matriz_global, N, M, &saudaveis, &contaminados, &mortos);
        
        printf("\nSimulação finalizada!\n");
        printf("Iterações executadas: %d\n", iteracao);
        printf("Saudáveis: %d\n", saudaveis);
        printf("Contaminados: %d\n", contaminados);
        printf("Mortos: %d\n", mortos);
        printf("Tempo de execução: %.6f segundos\n", tempo_execucao);
        
        int total_sobreviventes = saudaveis + contaminados;
        salvar_resultado(arquivo_saida, mortos, total_sobreviventes, tempo_execucao);
        
        printf("Resultado salvo em %s\n", arquivo_saida);
        
        liberar_matriz(matriz_global, N);
        free(sendcounts);
        free(displs);
    }
    
    // Libera memória
    liberar_matriz(matriz_local, linhas_com_halo);
    liberar_matriz(nova_matriz_local, linhas_com_halo);
    free(recvbuf);
    
    MPI_Finalize();
    return 0;
}
