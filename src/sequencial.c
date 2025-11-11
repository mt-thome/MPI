#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Estados possíveis
#define VAZIO 0
#define SAUDAVEL 1
#define CONTAMINADO -1
#define MORTO -2
#define MORTO_ANTERIOR -3  // Morto na iteração anterior (ainda contamina)

typedef struct {
    int **matriz;
    int N;  // linhas
    int M;  // colunas
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

// Função para ler a região do arquivo
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
int tem_contaminado_vizinho(int **matriz, int N, int M, int i, int j) {
    // Vizinho acima
    if (i > 0 && (matriz[i-1][j] == CONTAMINADO || matriz[i-1][j] == MORTO || matriz[i-1][j] == MORTO_ANTERIOR))
        return 1;
    // Vizinho abaixo
    if (i < N-1 && (matriz[i+1][j] == CONTAMINADO || matriz[i+1][j] == MORTO || matriz[i+1][j] == MORTO_ANTERIOR))
        return 1;
    // Vizinho esquerda
    if (j > 0 && (matriz[i][j-1] == CONTAMINADO || matriz[i][j-1] == MORTO || matriz[i][j-1] == MORTO_ANTERIOR))
        return 1;
    // Vizinho direita
    if (j < M-1 && (matriz[i][j+1] == CONTAMINADO || matriz[i][j+1] == MORTO || matriz[i][j+1] == MORTO_ANTERIOR))
        return 1;
    
    return 0;
}

// Aplica as regras da simulação
void simular_iteracao(Regiao *regiao) {
    int **nova_matriz = alocar_matriz(regiao->N, regiao->M);
    
    for (int i = 0; i < regiao->N; i++) {
        for (int j = 0; j < regiao->M; j++) {
            int estado_atual = regiao->matriz[i][j];
            
            if (estado_atual == VAZIO) {
                nova_matriz[i][j] = VAZIO;
            }
            else if (estado_atual == SAUDAVEL) {
                // Saudável pode ser contaminado se tiver vizinho contaminado/morto
                if (tem_contaminado_vizinho(regiao->matriz, regiao->N, regiao->M, i, j)) {
                    nova_matriz[i][j] = CONTAMINADO;
                } else {
                    nova_matriz[i][j] = SAUDAVEL;
                }
            }
            else if (estado_atual == CONTAMINADO) {
                // Contaminado: 10% cura, 30% continua doente, 60% morre
                int prob = rand() % 10000;
                if (prob < 1000) {  // 0-999: cura (10%)
                    nova_matriz[i][j] = SAUDAVEL;
                } else if (prob < 4000) {  // 1000-3999: continua doente (30%)
                    nova_matriz[i][j] = CONTAMINADO;
                } else {  // 4000-9999: morre (60%)
                    nova_matriz[i][j] = MORTO;
                }
            }
            else if (estado_atual == MORTO) {
                // Morto vira MORTO_ANTERIOR (ainda contamina)
                nova_matriz[i][j] = MORTO_ANTERIOR;
            }
            else if (estado_atual == MORTO_ANTERIOR) {
                // Morto anterior desaparece
                nova_matriz[i][j] = VAZIO;
            }
        }
    }
    
    // Substitui a matriz antiga pela nova
    liberar_matriz(regiao->matriz, regiao->N);
    regiao->matriz = nova_matriz;
}

// Conta população
void contar_populacao(Regiao *regiao, int *saudaveis, int *contaminados, int *mortos) {
    *saudaveis = 0;
    *contaminados = 0;
    *mortos = 0;
    
    for (int i = 0; i < regiao->N; i++) {
        for (int j = 0; j < regiao->M; j++) {
            if (regiao->matriz[i][j] == SAUDAVEL) {
                (*saudaveis)++;
            } else if (regiao->matriz[i][j] == CONTAMINADO) {
                (*contaminados)++;
            } else if (regiao->matriz[i][j] == MORTO || regiao->matriz[i][j] == MORTO_ANTERIOR) {
                (*mortos)++;
            }
        }
    }
}

// Verifica se a simulação deve terminar
int deve_continuar(int saudaveis, int contaminados) {
    return contaminados > 0;  // Continua se ainda há contaminados
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
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> [arquivo_saida]\n", argv[0]);
        return 1;
    }
    
    const char *arquivo_entrada = argv[1];
    const char *arquivo_saida = argc > 2 ? argv[2] : "resultado_sequencial.txt";
    
    // Inicializa gerador de números aleatórios
    srand(time(NULL));
    
    // Lê a entrada
    Regiao regiao = ler_entrada(arquivo_entrada);
    
    int max_iteracoes = regiao.N * regiao.M;
    int saudaveis, contaminados, mortos;
    
    printf("Iniciando simulação sequencial...\n");
    printf("Região: %dx%d\n", regiao.N, regiao.M);
    printf("Máximo de iterações: %d\n\n", max_iteracoes);
    
    clock_t inicio = clock();
    
    int iteracao = 0;
    do {
        simular_iteracao(&regiao);
        iteracao++;
        
        contar_populacao(&regiao, &saudaveis, &contaminados, &mortos);
        
        if (iteracao % 100 == 0 || iteracao == 1) {
            printf("Iteração %d: Saudáveis=%d, Contaminados=%d, Mortos=%d\n", 
                   iteracao, saudaveis, contaminados, mortos);
        }
        
    } while (deve_continuar(saudaveis, contaminados) && iteracao < max_iteracoes);
    
    clock_t fim = clock();
    double tempo_execucao = (double)(fim - inicio) / CLOCKS_PER_SEC;
    
    printf("\nSimulação finalizada!\n");
    printf("Iterações executadas: %d\n", iteracao);
    printf("Saudáveis: %d\n", saudaveis);
    printf("Contaminados: %d\n", contaminados);
    printf("Mortos: %d\n", mortos);
    printf("Tempo de execução: %.6f segundos\n", tempo_execucao);
    
    int total_sobreviventes = saudaveis + contaminados;
    salvar_resultado(arquivo_saida, mortos, total_sobreviventes, tempo_execucao);
    
    printf("Resultado salvo em %s\n", arquivo_saida);
    
    liberar_matriz(regiao.matriz, regiao.N);
    
    return 0;
}
