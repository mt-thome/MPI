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
    int **matrix;
    int N;  
    int M;  
    int local_lines;  
    int first_line;   
} block;

// Função para alocar a matriz
int** alloc_matrix(int N, int M) {
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
    
    region.matrix = alloc_matrix(region.N, region.M);
    
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
int has_contaminated_neighbor(int **matrix, int lines, int M, int i, int j) {
    // Vizinho acima
    if (i > 0 && (matrix[i-1][j] == CONTAMINADO || matrix[i-1][j] == MORTO || matrix[i-1][j] == MORTO_ANTERIOR))
        return 1;
    // Vizinho abaixo
    if (i < lines-1 && (matrix[i+1][j] == CONTAMINADO || matrix[i+1][j] == MORTO || matrix[i+1][j] == MORTO_ANTERIOR))
        return 1;
    // Vizinho esquerda
    if (j > 0 && (matrix[i][j-1] == CONTAMINADO || matrix[i][j-1] == MORTO || matrix[i][j-1] == MORTO_ANTERIOR))
        return 1;
    // Vizinho direita
    if (j < M-1 && (matrix[i][j+1] == CONTAMINADO || matrix[i][j+1] == MORTO || matrix[i][j+1] == MORTO_ANTERIOR))
        return 1;
    return 0;
}

// Aplica as regras da simulação na região local
void simulate_local_iteration(int **matrix, int **new_matrix, int lines, int M) {
    for (int i = 0; i < lines; i++) {
        for (int j = 0; j < M; j++) {
            int current_state = matrix[i][j];
            
            if (current_state == VAZIO) {
                new_matrix[i][j] = VAZIO;
            }
            else if (current_state == SAUDAVEL) {
                if (has_contaminated_neighbor(matrix, lines, M, i, j)) {
                    new_matrix[i][j] = CONTAMINADO;
                } else {
                    new_matrix[i][j] = SAUDAVEL;
                }
            }
            else if (current_state == CONTAMINADO) {
                int prob = rand() % 10000;
                if (prob < 1000) {
                    new_matrix[i][j] = SAUDAVEL;
                } else if (prob < 4000) {
                    new_matrix[i][j] = CONTAMINADO;
                } else {
                    new_matrix[i][j] = MORTO;
                }
            }
            else if (current_state == MORTO) {
                new_matrix[i][j] = MORTO_ANTERIOR;
            }
            else if (current_state == MORTO_ANTERIOR) {
                new_matrix[i][j] = VAZIO;
            }
        }
    }
}

// Conta população local
void count_local_population(int **matrix, int lines, int M, int *healthy, int *contaminated, int *dead) {
    *healthy = 0;
    *contaminated = 0;
    *dead = 0;
    
    for (int i = 0; i < lines; i++) {
        for (int j = 0; j < M; j++) {
            if (matrix[i][j] == SAUDAVEL) {
                (*healthy)++;
            } else if (matrix[i][j] == CONTAMINADO) {
                (*contaminated)++;
            } else if (matrix[i][j] == MORTO || matrix[i][j] == MORTO_ANTERIOR) {
                (*dead)++;
            }
        }
    }
}

// Salva resultado
void save_result(const char *file, int total_dead, int total_survivors, double time) {
    FILE *fp = fopen(file, "w");
    if (!fp) {
        fprintf(stderr, "Erro ao criar arquivo de saída\n");
        return;
    }
    
    fprintf(fp, "Total de mortos: %d\n", total_dead);
    fprintf(fp, "Total de sobreviventes: %d\n", total_survivors);
    fprintf(fp, "Tempo de execução: %.6f segundos\n", time);
    
    fclose(fp);
}

int main(int argc, char *argv[]) {
    int rank, size;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "Uso: %s <file_in> [file_out]\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }
    
    const char *file_in = argv[1];
    const char *file_out = argc > 2 ? argv[2] : "resultado_mpi.txt";
    
    srand(time(NULL) + rank);
    
    int N, M;
    int **global_matrix = NULL;
    
    double start_time = MPI_Wtime();
    
    if (rank == 0) {
        block region = read_in(file_in);
        N = region.N;
        M = region.M;
        global_matrix = region.matrix;
        
        printf("Iniciando simulação MPI com %d processos...\n", size);
        printf("Região: %dx%d\n", N, M);
        printf("Máximo de iterações: %d\n\n", N * M);
    }
    
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    
    int lines_per_process = N / size;
    int extra_lines = N % size;
    
    int local_lines = lines_per_process + (rank < extra_lines ? 1 : 0);
    
    // int start_line = rank * lines_per_process + (rank < extra_lines ? rank : extra_lines);
    
    int halo_lines = local_lines + 2;  // +2 para linhas de halo
    int **local_matrix = alloc_matrix(halo_lines, M);
    int **new_local_matrix = alloc_matrix(halo_lines, M);
    
    int *sendcounts = NULL;
    int *displs = NULL;
    int *recvbuf = (int *)malloc(local_lines * M * sizeof(int));
    
    if (rank == 0) {
        sendcounts = (int *)malloc(size * sizeof(int));
        displs = (int *)malloc(size * sizeof(int));
        
        int offset = 0;
        for (int i = 0; i < size; i++) {
            int lines = lines_per_process + (i < extra_lines ? 1 : 0);
            sendcounts[i] = lines * M;
            displs[i] = offset;
            offset += sendcounts[i];
        }
    }
    
    int max_iterations = N * M;
    int iteration = 0;
    int continue_simulation = 1;
    
    while (continue_simulation && iteration < max_iterations) {
        if (rank == 0) {
            int *buffer = (int *)malloc(N * M * sizeof(int));
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    buffer[i * M + j] = global_matrix[i][j];
                }
            }
            MPI_Scatterv(buffer, sendcounts, displs, MPI_INT, recvbuf, local_lines * M, MPI_INT, 0, MPI_COMM_WORLD);
            free(buffer);
        } else {
            MPI_Scatterv(NULL, NULL, NULL, MPI_INT, recvbuf, local_lines * M, MPI_INT, 0, MPI_COMM_WORLD);
        }
        
        for (int i = 0; i < local_lines; i++) {
            for (int j = 0; j < M; j++) {
                local_matrix[i + 1][j] = recvbuf[i * M + j];
            }
        }
        
        MPI_Request requests[4];
        int req_count = 0;
        
        if (rank > 0) {
            MPI_Isend(local_matrix[1], M, MPI_INT, rank - 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Irecv(local_matrix[0], M, MPI_INT, rank - 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            for (int j = 0; j < M; j++) {
                local_matrix[0][j] = VAZIO;
            }
        }
        
        if (rank < size - 1) {
            MPI_Isend(local_matrix[local_lines], M, MPI_INT, rank + 1, 1, MPI_COMM_WORLD, &requests[req_count++]);
            MPI_Irecv(local_matrix[local_lines + 1], M, MPI_INT, rank + 1, 0, MPI_COMM_WORLD, &requests[req_count++]);
        } else {
            for (int j = 0; j < M; j++) {
                local_matrix[local_lines + 1][j] = VAZIO;
            }
        }
        
        MPI_Waitall(req_count, requests, MPI_STATUSES_IGNORE);
        
        simulate_local_iteration(local_matrix, new_local_matrix, halo_lines, M);
        
        for (int i = 0; i < local_lines; i++) {
            for (int j = 0; j < M; j++) {
                recvbuf[i * M + j] = new_local_matrix[i + 1][j];
            }
        }
        
        if (rank == 0) {
            int *buffer = (int *)malloc(N * M * sizeof(int));
            MPI_Gatherv(recvbuf, local_lines * M, MPI_INT, buffer, sendcounts, displs, MPI_INT, 0, MPI_COMM_WORLD);
            
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    global_matrix[i][j] = buffer[i * M + j];
                }
            }
            free(buffer);
        } else {
            MPI_Gatherv(recvbuf, local_lines * M, MPI_INT, NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
        }
        
        iteration++;
        
        if (rank == 0) {
            int healthy, infected, dead;
            count_local_population(global_matrix, N, M, &healthy, &infected, &dead);
            
            continue_simulation = (infected > 0);
            
            if (iteration % 100 == 0 || iteration == 1) { 
                printf("Iteração %d: Saudáveis=%d, Contaminados=%d, Mortos=%d\n", iteration, healthy, infected, dead);
            }
        }
        
        MPI_Bcast(&continue_simulation, 1, MPI_INT, 0, MPI_COMM_WORLD);
    }
    
    double end_time = MPI_Wtime();
    double execution_time = end_time - start_time;
    
    if (rank == 0) {
        int healthy, infected, dead;
        count_local_population(global_matrix, N, M, &healthy, &infected, &dead);
        
        printf("\nSimulação finalizada!\n");
        printf("Iterações executadas: %d\n", iteration);
        printf("Saudáveis: %d\n", healthy);
        printf("Contaminados: %d\n", infected);
        printf("Mortos: %d\n", dead);
        printf("Tempo de execução: %.6f segundos\n", execution_time);
        
        int total_survivors = healthy + infected;
        save_result(file_out, dead, total_survivors, execution_time);
        
        printf("Resultado salvo em %s\n", file_out);
        
        free_matrix(global_matrix, N);
        free(sendcounts);
        free(displs);
    }
    
    free_matrix(local_matrix, halo_lines);
    free_matrix(new_local_matrix, halo_lines);
    free(recvbuf);
    
    MPI_Finalize();
    return 0;
}
