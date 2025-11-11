#!/usr/bin/env python3
import random
import sys

def gerar_entrada(N, M, num_contaminados=5):
    """Gera um arquivo de entrada para a simulação"""
    print(f"{N} {M}")
    
    # Cria matriz com pessoas saudáveis e alguns vazios
    matriz = []
    for i in range(N):
        linha = []
        for j in range(M):
            # 80% saudável, 20% vazio
            if random.random() < 0.8:
                linha.append(1)
            else:
                linha.append(0)
        matriz.append(linha)
    
    # Adiciona contaminados aleatoriamente
    for _ in range(num_contaminados):
        i = random.randint(0, N-1)
        j = random.randint(0, M-1)
        matriz[i][j] = -1
    
    # Imprime matriz
    for linha in matriz:
        print(' '.join(map(str, linha)))

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Uso: python3 gerar_entrada.py N M [num_contaminados]")
        sys.exit(1)
    
    N = int(sys.argv[1])
    M = int(sys.argv[2])
    num_contaminados = int(sys.argv[3]) if len(sys.argv) > 3 else 5
    
    random.seed(42)  # Para resultados reproduzíveis
    gerar_entrada(N, M, num_contaminados)
