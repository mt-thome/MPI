#!/bin/bash

# Script simplificado para testes rápidos
ENTRADA=$1
RESULTS_DIR="results"

if [ -z "$ENTRADA" ]; then
    echo "Uso: $0 <arquivo_entrada>"
    exit 1
fi

# Cria diretório de resultados se não existir
mkdir -p $RESULTS_DIR

echo "========================================"
echo "TESTES DE DESEMPENHO"
echo "========================================"
echo "Entrada: $ENTRADA"
echo ""

echo "--- Teste Sequencial ---"
./sequencial $ENTRADA $RESULTS_DIR/resultado_seq_tmp.txt
tempo_seq=$(grep "Tempo de execução:" $RESULTS_DIR/resultado_seq_tmp.txt | awk '{print $4}')
echo "Tempo: $tempo_seq segundos"
echo ""

echo "--- MPI com 1 processo ---"
mpirun -np 1 ./main_mpi $ENTRADA $RESULTS_DIR/resultado_mpi1_tmp.txt 2>/dev/null
tempo_mpi1=$(grep "Tempo de execução:" $RESULTS_DIR/resultado_mpi1_tmp.txt | awk '{print $4}')
echo "Tempo: $tempo_mpi1 segundos"
echo ""

echo "--- MPI com 2 processos ---"
mpirun -np 2 ./main_mpi $ENTRADA $RESULTS_DIR/resultado_mpi2_tmp.txt 2>/dev/null
tempo_mpi2=$(grep "Tempo de execução:" $RESULTS_DIR/resultado_mpi2_tmp.txt | awk '{print $4}')
echo "Tempo: $tempo_mpi2 segundos"
echo ""

echo "--- MPI com 4 processos ---"
mpirun -np 4 ./main_mpi $ENTRADA $RESULTS_DIR/resultado_mpi4_tmp.txt 2>/dev/null
tempo_mpi4=$(grep "Tempo de execução:" $RESULTS_DIR/resultado_mpi4_tmp.txt | awk '{print $4}')
echo "Tempo: $tempo_mpi4 segundos"
echo ""

echo "--- MPI com 8 processos ---"
mpirun -np 8 --oversubscribe ./main_mpi $ENTRADA $RESULTS_DIR/resultado_mpi8_tmp.txt 2>/dev/null
tempo_mpi8=$(grep "Tempo de execução:" $RESULTS_DIR/resultado_mpi8_tmp.txt | awk '{print $4}')
echo "Tempo: $tempo_mpi8 segundos"
echo ""

echo "========================================"
echo "RESUMO DOS RESULTADOS"
echo "========================================"
echo ""
printf "%-25s %20s\n" "Configuração" "Tempo (s)"
echo "------------------------------------------------"
printf "%-25s %20s\n" "Sequencial" "$tempo_seq"
printf "%-25s %20s\n" "MPI - 1 processo" "$tempo_mpi1"
printf "%-25s %20s\n" "MPI - 2 processos" "$tempo_mpi2"
printf "%-25s %20s\n" "MPI - 4 processos" "$tempo_mpi4"
printf "%-25s %20s\n" "MPI - 8 processos" "$tempo_mpi8"
echo ""

# Calcular speedup (se bc estiver disponível)
if command -v bc &> /dev/null; then
    echo "Speedup relativo ao sequencial:"
    echo "  MPI-1: $(echo "scale=2; $tempo_seq / $tempo_mpi1" | bc)x"
    echo "  MPI-2: $(echo "scale=2; $tempo_seq / $tempo_mpi2" | bc)x"
    echo "  MPI-4: $(echo "scale=2; $tempo_seq / $tempo_mpi4" | bc)x"
    echo "  MPI-8: $(echo "scale=2; $tempo_seq / $tempo_mpi8" | bc)x"
fi

# Limpar arquivos temporários
rm -f $RESULTS_DIR/resultado_*_tmp.txt

echo ""
echo "Teste concluído!"
