# Compilador C
CC = gcc
MPICC = mpicc

# Flags de compilação
CFLAGS = -Wall -O2
LDFLAGS = -lm

# Diretórios
SRC_DIR = src
DATA_DIR = data
RESULTS_DIR = results
SCRIPTS_DIR = scripts

# Arquivos fonte
SRC_SEQ = $(SRC_DIR)/sequencial.c
SRC_MPI = $(SRC_DIR)/main.c

# Executáveis
BIN_SEQ = sequencial
BIN_MPI = main_mpi

# Arquivos de entrada
ENTRADA_PEQUENA = $(DATA_DIR)/entrada_20x20.txt
ENTRADA_MEDIA = $(DATA_DIR)/entrada_100x100.txt
ENTRADA_GRANDE = $(DATA_DIR)/entrada_500x500.txt

# Arquivo padrão
ENTRADA = $(ENTRADA_MEDIA)

.PHONY: all clean test gerar_entradas run_seq run_mpi

all: $(BIN_SEQ) $(BIN_MPI)

# Compila versão sequencial
$(BIN_SEQ): $(SRC_SEQ)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Compila versão MPI
$(BIN_MPI): $(SRC_MPI)
	$(MPICC) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Gera arquivos de entrada de diferentes tamanhos
gerar_entradas:
	@echo "Gerando arquivos de entrada..."
	python3 $(SCRIPTS_DIR)/gerar_entrada.py 20 20 2 > $(ENTRADA_PEQUENA)
	python3 $(SCRIPTS_DIR)/gerar_entrada.py 100 100 5 > $(ENTRADA_MEDIA)
	python3 $(SCRIPTS_DIR)/gerar_entrada.py 500 500 10 > $(ENTRADA_GRANDE)
	@echo "Arquivos de entrada gerados!"

# Executa versão sequencial
run_seq: $(BIN_SEQ)
	./$(BIN_SEQ) $(ENTRADA) $(RESULTS_DIR)/resultado_sequencial.txt

# Executa versão MPI com 1 processo
run_mpi_1: $(BIN_MPI)
	mpirun -np 1 ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_1.txt

# Executa versão MPI com 2 processos
run_mpi_2: $(BIN_MPI)
	mpirun -np 2 ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_2.txt

# Executa versão MPI com 4 processos
run_mpi_4: $(BIN_MPI)
	mpirun -np 4 ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_4.txt

# Executa versão MPI com 8 processos
run_mpi_8: $(BIN_MPI)
	mpirun -np 8 --oversubscribe ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_8.txt

# Executa todos os testes
test: $(BIN_SEQ) $(BIN_MPI)
	@echo "=== Executando testes ==="
	@echo ""
	@echo "--- Versão Sequencial ---"
	./$(BIN_SEQ) $(ENTRADA) $(RESULTS_DIR)/resultado_sequencial.txt
	@echo ""
	@echo "--- MPI com 1 processo ---"
	mpirun -np 1 ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_1.txt
	@echo ""
	@echo "--- MPI com 2 processos ---"
	mpirun -np 2 ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_2.txt
	@echo ""
	@echo "--- MPI com 4 processos ---"
	mpirun -np 4 ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_4.txt
	@echo ""
	@echo "--- MPI com 8 processos ---"
	mpirun -np 8 --oversubscribe ./$(BIN_MPI) $(ENTRADA) $(RESULTS_DIR)/resultado_mpi_8.txt
	@echo ""
	@echo "=== Resultados salvos em $(RESULTS_DIR)/ ==="

# Limpa arquivos compilados e resultados
clean:
	rm -f $(BIN_SEQ) $(BIN_MPI)
	rm -f $(RESULTS_DIR)/*.txt
	rm -f *.o

# Limpa tudo incluindo entradas geradas
cleanall: clean
	rm -f $(DATA_DIR)/entrada_100x100.txt $(DATA_DIR)/entrada_500x500.txt

# Ajuda
help:
	@echo "Makefile para projeto MPI - Simulação de Propagação de Doença"
	@echo ""
	@echo "Alvos disponíveis:"
	@echo "  make all              - Compila todos os programas"
	@echo "  make gerar_entradas   - Gera arquivos de entrada de teste"
	@echo "  make run_seq          - Executa versão sequencial"
	@echo "  make run_mpi_1        - Executa MPI com 1 processo"
	@echo "  make run_mpi_2        - Executa MPI com 2 processos"
	@echo "  make run_mpi_4        - Executa MPI com 4 processos"
	@echo "  make run_mpi_8        - Executa MPI com 8 processos"
	@echo "  make test             - Executa todos os testes"
	@echo "  make clean            - Remove executáveis e resultados"
	@echo "  make cleanall         - Remove tudo incluindo entradas geradas"
	@echo ""
	@echo "Para usar entrada diferente:"
	@echo "  make run_seq ENTRADA=entrada_20x20.txt"
