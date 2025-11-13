#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define VAZIO 0
#define SAUDAVEL 1
#define CONTAMINADO -1
#define MORTO -2
#define MORTO_ANTERIOR -3  

typedef struct {
    int **matrix;
    int N;  
    int M;  
} block;

// Função para alocar a matriz
int** allocate_matrix(int N, int M) {
    int **matrix = (int **)malloc(N * sizeof(int *));
    for (int i = 0; i < N; i++) {
        matrix[i] = (int *)malloc(M * sizeof(int));
    }
    return matrix;
}

// Função para liberar a matriz
void free_matrix(int **matrix, int N) {
    for (int i = 0; i < N; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

// Função para ler a região do arquivo
block read_in(const char *file_in) {
    FILE *fp = fopen(file_in, "r");
    if (!fp) {
        fprintf(stderr, "Erro ao abrir arquivo %s\n", file_in);
        exit(1);
    }
    
    block region;
    if (fscanf(fp, "%d %d", &region.N, &region.M) != 2) {
        fprintf(stderr, "Erro ao ler dimensões do arquivo\n");
        fclose(fp);
        exit(1);
    }
    
    region.matrix = allocate_matrix(region.N, region.M);
    
    for (int i = 0; i < region.N; i++) {
        for (int j = 0; j < region.M; j++) {
            if (fscanf(fp, "%d", &region.matrix[i][j]) != 1) {
                fprintf(stderr, "Erro ao ler valor na posição [%d][%d]\n", i, j);
                fclose(fp);
                exit(1);
            }
        }
    }
    
    fclose(fp);
    return region;
}

// Verifica se há contaminado ou morto na vizinhança
int has_contaminated_neighbor(int **matrix, int N, int M, int i, int j) {
    if (i > 0 && (matrix[i-1][j] == CONTAMINADO || matrix[i-1][j] == MORTO || matrix[i-1][j] == MORTO_ANTERIOR))
        return 1;
    if (i < N-1 && (matrix[i+1][j] == CONTAMINADO || matrix[i+1][j] == MORTO || matrix[i+1][j] == MORTO_ANTERIOR))
        return 1;
    if (j > 0 && (matrix[i][j-1] == CONTAMINADO || matrix[i][j-1] == MORTO || matrix[i][j-1] == MORTO_ANTERIOR))
        return 1;
    if (j < M-1 && (matrix[i][j+1] == CONTAMINADO || matrix[i][j+1] == MORTO || matrix[i][j+1] == MORTO_ANTERIOR))
        return 1;
    
    return 0;
}

// Aplica as regras da simulação
void simulate_interation(block *region) {
    int **new_matrix = allocate_matrix(region->N, region->M);
    
    for (int i = 0; i < region->N; i++) {
        for (int j = 0; j < region->M; j++) {
            int curr = region->matrix[i][j];
            
            if (curr == VAZIO) {
                new_matrix[i][j] = VAZIO;
            }
            else if (curr == SAUDAVEL) {
                if (has_contaminated_neighbor(region->matrix, region->N, region->M, i, j)) {
                    new_matrix[i][j] = CONTAMINADO;
                } else {
                    new_matrix[i][j] = SAUDAVEL;
                }
            }
            else if (curr == CONTAMINADO) {
                int prob = rand() % 10000;
                if (prob < 1000) {  
                    new_matrix[i][j] = SAUDAVEL;
                } else if (prob < 4000) {  
                    new_matrix[i][j] = CONTAMINADO;
                } else {  
                    new_matrix[i][j] = MORTO;
                }
            }
            else if (curr == MORTO) {
                new_matrix[i][j] = MORTO_ANTERIOR;
            }
            else if (curr == MORTO_ANTERIOR) {
                new_matrix[i][j] = VAZIO;
            }
        }
    }
    
    free_matrix(region->matrix, region->N);
    region->matrix = new_matrix;
}

// Conta população
void count_population(block *region, int *healthy, int *contaminated, int *deads) {
    *healthy = 0;
    *contaminated = 0;
    *deads = 0;
    
    for (int i = 0; i < region->N; i++) {
        for (int j = 0; j < region->M; j++) {
            if (region->matrix[i][j] == SAUDAVEL) {
                (*healthy)++;
            } else if (region->matrix[i][j] == CONTAMINADO) {
                (*contaminated)++;
            } else if (region->matrix[i][j] == MORTO || region->matrix[i][j] == MORTO_ANTERIOR) {
                (*deads)++;
            }
        }
    }
}

// Verifica se a simulação deve terminar
int should_continue(int healthy, int contaminated) {
    return contaminated > 0;
}

// Salva resultado
void save_result(const char *file, int total_deads, int total_survivors, double time) {
    FILE *fp = fopen(file, "w");
    if (!fp) {
        fprintf(stderr, "Erro ao criar arquivo de saída\n");
        return;
    }
    
    fprintf(fp, "Total de mortos: %d\n", total_deads);
    fprintf(fp, "Total de sobreviventes: %d\n", total_survivors);
    fprintf(fp, "Tempo de execução: %.6f segundos\n", time);
    
    fclose(fp);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_entrada> [arquivo_saida]\n", argv[0]);
        return 1;
    }
    
    const char *input_file = argv[1];
    const char *output_file = argc > 2 ? argv[2] : "resultado_sequencial.txt";
    
    srand(time(NULL));
    
    block region = read_in(input_file);
    
    int max_iterations = region.N * region.M;
    int healthy, contaminated, deads;
    
    printf("Iniciando simulação sequencial...\n");
    printf("Região: %dx%d\n", region.N, region.M);
    printf("Máximo de iterações: %d\n\n", max_iterations);
    
    clock_t start = clock();
    
    int iteration = 0;
    do {
        simulate_interation(&region);
        iteration++;
        
        count_population(&region, &healthy, &contaminated, &deads);
        
        if (iteration % 100 == 0 || iteration == 1) {
            printf("Iteração %d: Saudáveis=%d, Contaminados=%d, Mortos=%d\n", 
                   iteration, healthy, contaminated, deads);
        }
        
    } while (should_continue(healthy, contaminated) && iteration < max_iterations);
    
    clock_t end = clock();
    double execution_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("\nSimulação finalizada!\n");
    printf("Iterações executadas: %d\n", iteration);
    printf("Saudáveis: %d\n", healthy);
    printf("Contaminados: %d\n", contaminated);
    printf("Mortos: %d\n", deads);
    printf("Tempo de execução: %.6f segundos\n", execution_time);
    
    int total_survivors = healthy + contaminated;
    save_result(output_file, deads, total_survivors, execution_time);
    
    printf("Resultado salvo em %s\n", output_file);
    
    free_matrix(region.matrix, region.N);
    
    return 0;
}
