# Simulador de Propagação de Doença com MPI

[![C](https://img.shields.io/badge/C-00599C?style=flat&logo=c&logoColor=white)](https://en.wikipedia.org/wiki/C_(programming_language))
[![OpenMPI](https://img.shields.io/badge/OpenMPI-654FF0?style=flat&logo=openmpi&logoColor=white)](https://www.open-mpi.org/)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

> Implementação paralela de um simulador epidemiológico utilizando MPI (Message Passing Interface) para análise de desempenho em programação concorrente.

## Sobre o Projeto

Este projeto implementa uma simulação da propagação de uma doença fatal em uma região retangular NxM, utilizando tanto uma versão sequencial quanto uma versão paralela com MPI. O objetivo é comparar o desempenho entre diferentes números de processos e analisar os ganhos da paralelização.

### Características

- ✅ Simulação de propagação de doença com regras probabilísticas
- ✅ Implementação sequencial para baseline
- ✅ Implementação paralela com MPI
- ✅ Divisão horizontal de dados entre processos
- ✅ Comunicação não-bloqueante (MPI_Isend/Irecv)
- ✅ Balanceamento automático de carga
- ✅ Scripts para testes automatizados
- ✅ Gerador de entradas de teste configuráveis

### Regras da Simulação

Cada célula da matriz pode conter:
- **1**: Pessoa saudável
- **-1**: Pessoa contaminada
- **-2**: Pessoa morta (recém-falecida)
- **0**: Vazio (sem pessoa)

**Regras de evolução:**
1. Pessoa saudável é contaminada se tiver vizinho contaminado/morto (horizontal ou vertical)
2. Pessoa contaminada a cada iteração:
   - 10% de chance de cura
   - 30% de chance de continuar doente
   - 60% de chance de morrer
3. Pessoa morta contamina vizinhos por mais uma iteração antes de desaparecer

**Condições de parada:**
- Após N×M iterações, ou
- Quando não houver mais pessoas contaminadas

## Como Usar

### Pré-requisitos

```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y build-essential openmpi-bin openmpi-common libopenmpi-dev python3
```

### Compilação

```bash
# Compilar tudo
make all

# Ou compilar separadamente
make sequencial    # Versão sequencial
make main_mpi      # Versão MPI
```

### Execução

#### Versão Sequencial
```bash
./sequencial data/entrada_100x100.txt results/resultado_seq.txt
```

#### Versão MPI
```bash
# Com 1 processo
mpirun -np 1 ./main_mpi data/entrada_100x100.txt results/resultado_mpi_1.txt

# Com 4 processos
mpirun -np 4 ./main_mpi data/entrada_100x100.txt results/resultado_mpi_4.txt

# Com 8 processos (se não tiver 8 núcleos, use --oversubscribe)
mpirun -np 8 --oversubscribe ./main_mpi data/entrada_100x100.txt results/resultado_mpi_8.txt
```

### Testes Automatizados

```bash
# Teste rápido (1 execução de cada)
./scripts/teste_rapido.sh data/entrada_100x100.txt

# Gerar novas entradas
make gerar_entradas
```

## Estrutura do Projeto

```
MPI/
├── README.md                    # Este arquivo
├── Makefile                     # Automação de compilação e testes
│
├── src/                         # Código-fonte
│   ├── main.c                   # Implementação MPI
│   └── sequencial.c             # Implementação sequencial
│
├── scripts/                     # Scripts auxiliares
│   ├── gerar_entrada.py         # Gera arquivos de entrada
│   ├── teste_rapido.sh          # Testes rápidos
│   ├── executar_testes.sh       # Testes com média
│   └── instalar_mpi.sh          # Instalador do OpenMPI
│
├── data/                        # Arquivos de entrada
│   ├── entrada_20x20.txt        # Entrada pequena
│   ├── entrada_100x100.txt      # Entrada média
│   └── entrada_500x500.txt      # Entrada grande
│
├── results/                     # Resultados das simulações
│   └── *.txt                    # Arquivos de resultado
│
└── tests/                       # Testes unitários (futuro)
```

## Resultados Esperados

### Entrada 500×500 (250.000 células)

| Configuração | Tempo (s) | Speedup | Eficiência |
|--------------|-----------|---------|------------|
| Sequencial | 0.424582 | 1.00x | 100% |
| MPI - 1 proc | 0.529831 | 0.80x | 80% |
| MPI - 2 proc | 0.437075 | 0.97x | 49% |
| MPI - 4 proc | 0.430200 | 0.98x | 25% |
| MPI - 8 proc | 0.506254 | 0.83x | 10% |

> **Nota**: Para este tipo de problema, o overhead de comunicação MPI supera os ganhos da paralelização em matrizes pequenas/médias. Ganhos reais são observados em matrizes muito grandes (>1000×1000).

## Implementação MPI

### Estratégia de Paralelização

1. **Divisão de Dados**: Matriz dividida horizontalmente entre processos
2. **Linhas de Halo**: Cada processo mantém cópias das linhas vizinhas para verificar contaminação
3. **Comunicação**:
   - `MPI_Scatterv`: Distribui dados do processo 0 para todos
   - `MPI_Gatherv`: Coleta resultados de todos para processo 0
   - `MPI_Isend/Irecv`: Troca assíncrona de linhas de halo entre vizinhos
4. **Balanceamento**: Distribuição automática de linhas extras quando N não é divisível por P

### Exemplo de Divisão (N=100, P=8)

```
Processo 0: linhas  0-12  (13 linhas)
Processo 1: linhas 13-25  (13 linhas)
Processo 2: linhas 26-38  (13 linhas)
Processo 3: linhas 39-51  (13 linhas)
Processo 4: linhas 52-64  (12 linhas)
Processo 5: linhas 65-76  (12 linhas)
Processo 6: linhas 77-88  (12 linhas)
Processo 7: linhas 89-99  (12 linhas)
```

## Gerando Entradas Personalizadas

```bash
# Sintaxe: python3 gerar_entrada.py N M num_contaminados > arquivo
python3 scripts/gerar_entrada.py 200 200 10 > data/entrada_200x200.txt
python3 scripts/gerar_entrada.py 1000 1000 50 > data/entrada_1000x1000.txt
```

## Comandos Make

```bash
make all              # Compila tudo
make sequencial       # Compila só sequencial
make main_mpi         # Compila só MPI
make gerar_entradas   # Gera entradas de teste
make test             # Executa todos os testes
make clean            # Remove executáveis e resultados
make cleanall         # Remove tudo incluindo entradas geradas
make help             # Mostra ajuda
```

## Formato dos Arquivos

### Arquivo de Entrada
```
N M
valor[0][0] valor[0][1] ... valor[0][M-1]
valor[1][0] valor[1][1] ... valor[1][M-1]
...
valor[N-1][0] valor[N-1][1] ... valor[N-1][M-1]
```

### Arquivo de Saída
```
Total de mortos: XXX
Total de sobreviventes: YYY
Tempo de execução: Z.ZZZZZZ segundos
```

## Análise de Desempenho

### Overhead de Comunicação

O MPI introduz overhead devido a:
- Inicialização (`MPI_Init`)
- Scatter/Gather de dados
- Troca de linhas de halo
- Sincronizações (`MPI_Bcast`)

### Quando MPI é Vantajoso?

- ✅ Matrizes muito grandes (≥1000×1000)
- ✅ Muitas iterações
- ✅ Cálculos complexos por célula
- ✅ Clusters com rede de baixa latência

### Quando MPI não Compensa?

- ❌ Matrizes pequenas (<100×100)
- ❌ Poucas iterações
- ❌ Computação simples por célula
- ❌ Overhead de comunicação > ganho computacional

## Referências

1. **MPI Forum**. MPI: A Message-Passing Interface Standard. [https://www.mpi-forum.org/](https://www.mpi-forum.org/)
2. **Pacheco, P.** An Introduction to Parallel Programming. Morgan Kaufmann, 2011.
3. **OpenMPI Documentation**. [https://www.open-mpi.org/doc/](https://www.open-mpi.org/doc/)

## Licença

Este projeto está sob a licença MIT. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.

## Contexto Acadêmico

Projeto desenvolvido para a disciplina de **Programação Concorrente e Paralela**.

**Objetivo**: Implementar e analisar o desempenho de uma simulação epidemiológica usando MPI, comparando versões sequencial e paralela com diferentes números de processos.

**Data de Entrega**: 28/11/2025
