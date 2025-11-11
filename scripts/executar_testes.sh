#!/bin/bash

# Script para executar testes múltiplos e calcular média dos tempos
# Uso: ./executar_testes.sh <arquivo_entrada>

if [ $# -lt 1 ]; then
    echo "Uso: $0 <arquivo_entrada>"
    exit 1
fi

ENTRADA=$1
NUM_EXECUCOES=3

echo "========================================"
echo "TESTES DE DESEMPENHO - SIMULAÇÃO DE DOENÇA"
echo "========================================"
echo "Arquivo de entrada: $ENTRADA"
echo "Número de execuções por teste: $NUM_EXECUCOES"
echo ""

# Função para extrair tempo de execução do arquivo de resultado
extrair_tempo() {
    grep "Tempo de execução:" $1 | awk '{print $4}'
}

# Função para calcular média
calcular_media() {
    awk '{sum+=$1} END {print sum/NR}' <<< "$@"
}

# ========================================
# TESTE 1: SEQUENCIAL
# ========================================
echo "========================================
TESTE 1: VERSÃO SEQUENCIAL
========================================
echo "Executando $NUM_EXECUCOES vezes..."
tempos_seq=""
for i in $(seq 1 $NUM_EXECUCOES); do
    echo "  Execução $i..."
    ./sequencial $ENTRADA resultado_seq_temp.txt > /dev/null 2>&1
    tempo=$(extrair_tempo resultado_seq_temp.txt)
    tempos_seq="$tempos_seq $tempo"
    echo "    Tempo: $tempo segundos"
done

media_seq=$(echo "$tempos_seq" | tr ' ' '\n' | awk '{sum+=$1; count++} END {print sum/count}')
echo "Média: $media_seq segundos"
echo ""

# ========================================
# TESTE 2: MPI COM 1 PROCESSO
# ========================================
echo "========================================"
echo "TESTE 2: MPI COM 1 PROCESSO"
echo "========================================"
echo "Executando $NUM_EXECUCOES vezes..."
tempos_mpi1=""
for i in $(seq 1 $NUM_EXECUCOES); do
    echo "  Execução $i..."
    mpirun -np 1 ./main_mpi $ENTRADA resultado_mpi1_temp.txt > /dev/null 2>&1
    tempo=$(extrair_tempo resultado_mpi1_temp.txt)
    tempos_mpi1="$tempos_mpi1 $tempo"
    echo "    Tempo: $tempo segundos"
done

media_mpi1=$(echo "$tempos_mpi1" | tr ' ' '\n' | awk '{sum+=$1; count++} END {print sum/count}')
echo "Média: $media_mpi1 segundos"
echo ""

# ========================================
# TESTE 3: MPI COM 2 PROCESSOS
# ========================================
echo "========================================"
echo "TESTE 3: MPI COM 2 PROCESSOS"
echo "========================================"
echo "Executando $NUM_EXECUCOES vezes..."
tempos_mpi2=""
for i in $(seq 1 $NUM_EXECUCOES); do
    echo "  Execução $i..."
    mpirun -np 2 ./main_mpi $ENTRADA resultado_mpi2_temp.txt > /dev/null 2>&1
    tempo=$(extrair_tempo resultado_mpi2_temp.txt)
    tempos_mpi2="$tempos_mpi2 $tempo"
    echo "    Tempo: $tempo segundos"
done

media_mpi2=$(echo "$tempos_mpi2" | tr ' ' '\n' | awk '{sum+=$1; count++} END {print sum/count}')
echo "Média: $media_mpi2 segundos"
echo ""

# ========================================
# TESTE 4: MPI COM 4 PROCESSOS
# ========================================
echo "========================================"
echo "TESTE 4: MPI COM 4 PROCESSOS"
echo "========================================"
echo "Executando $NUM_EXECUCOES vezes..."
tempos_mpi4=""
for i in $(seq 1 $NUM_EXECUCOES); do
    echo "  Execução $i..."
    mpirun -np 4 ./main_mpi $ENTRADA resultado_mpi4_temp.txt > /dev/null 2>&1
    tempo=$(extrair_tempo resultado_mpi4_temp.txt)
    tempos_mpi4="$tempos_mpi4 $tempo"
    echo "    Tempo: $tempo segundos"
done

media_mpi4=$(echo "$tempos_mpi4" | tr ' ' '\n' | awk '{sum+=$1; count++} END {print sum/count}')
echo "Média: $media_mpi4 segundos"
echo ""

# ========================================
# TESTE 5: MPI COM 8 PROCESSOS
# ========================================
echo "========================================"
echo "TESTE 5: MPI COM 8 PROCESSOS"
echo "========================================"
echo "Executando $NUM_EXECUCOES vezes..."
tempos_mpi8=""
for i in $(seq 1 $NUM_EXECUCOES); do
    echo "  Execução $i..."
    mpirun -np 8 ./main_mpi $ENTRADA resultado_mpi8_temp.txt > /dev/null 2>&1
    tempo=$(extrair_tempo resultado_mpi8_temp.txt)
    tempos_mpi8="$tempos_mpi8 $tempo"
    echo "    Tempo: $tempo segundos"
done

media_mpi8=$(echo "$tempos_mpi8" | tr ' ' '\n' | awk '{sum+=$1; count++} END {print sum/count}')
echo "Média: $media_mpi8 segundos"
echo ""

# ========================================
# RESUMO DOS RESULTADOS
# ========================================
echo "========================================"
echo "RESUMO DOS RESULTADOS"
echo "========================================"
echo ""
printf "%-25s %15s %15s\n" "Configuração" "Tempo Médio (s)" "Speedup"
echo "----------------------------------------------------------------"
printf "%-25s %15.6f %15s\n" "Sequencial" "$media_seq" "1.00x"
printf "%-25s %15.6f %15.2fx\n" "MPI - 1 processo" "$media_mpi1" "$(echo "scale=2; $media_seq / $media_mpi1" | bc)"
printf "%-25s %15.6f %15.2fx\n" "MPI - 2 processos" "$media_mpi2" "$(echo "scale=2; $media_seq / $media_mpi2" | bc)"
printf "%-25s %15.6f %15.2fx\n" "MPI - 4 processos" "$media_mpi4" "$(echo "scale=2; $media_seq / $media_mpi4" | bc)"
printf "%-25s %15.6f %15.2fx\n" "MPI - 8 processos" "$media_mpi8" "$(echo "scale=2; $media_seq / $media_mpi8" | bc)"
echo ""

# Salva resultados em arquivo
RESULTADO_FILE="resultados_$(date +%Y%m%d_%H%M%S).txt"
{
    echo "RESULTADOS DOS TESTES - $(date)"
    echo "Arquivo de entrada: $ENTRADA"
    echo ""
    printf "%-25s %15s %15s\n" "Configuração" "Tempo Médio (s)" "Speedup"
    echo "----------------------------------------------------------------"
    printf "%-25s %15.6f %15s\n" "Sequencial" "$media_seq" "1.00x"
    printf "%-25s %15.6f %15.2fx\n" "MPI - 1 processo" "$media_mpi1" "$(echo "scale=2; $media_seq / $media_mpi1" | bc)"
    printf "%-25s %15.6f %15.2fx\n" "MPI - 2 processos" "$media_mpi2" "$(echo "scale=2; $media_seq / $media_mpi2" | bc)"
    printf "%-25s %15.6f %15.2fx\n" "MPI - 4 processos" "$media_mpi4" "$(echo "scale=2; $media_seq / $media_mpi4" | bc)"
    printf "%-25s %15.6f %15.2fx\n" "MPI - 8 processos" "$media_mpi8" "$(echo "scale=2; $media_seq / $media_mpi8" | bc)"
} > $RESULTADO_FILE

echo "Resultados salvos em: $RESULTADO_FILE"

# Limpa arquivos temporários
rm -f resultado_*_temp.txt

echo ""
echo "Testes concluídos!"
