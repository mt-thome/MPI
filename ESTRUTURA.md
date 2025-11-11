# 📁 Estrutura do Projeto - Simulador MPI

## 🌳 Árvore de Diretórios

```
MPI/
├── 📄 README.md                    # Documentação principal (GitHub)
├── 📄 LICENSE                      # Licença MIT
├── 📄 Makefile                     # Automação de build e testes
├── 📄 .gitignore                   # Arquivos ignorados pelo Git
│
├── 📁 src/                         # CÓDIGO-FONTE
│   ├── main.c                      # ⭐ Implementação MPI paralela
│   └── sequencial.c                # ⭐ Implementação sequencial
│
├── 📁 scripts/                     # SCRIPTS AUXILIARES
│   ├── gerar_entrada.py            # Gerador de arquivos de entrada
│   ├── teste_rapido.sh             # Testes rápidos (1 execução)
│   ├── executar_testes.sh          # Testes completos (3 execuções + média)
│   └── instalar_mpi.sh             # Instalador automático do OpenMPI
│
├── 📁 data/                        # ARQUIVOS DE ENTRADA
│   ├── entrada_20x20.txt           # Entrada pequena (400 células)
│   ├── entrada_50x50.txt           # Entrada pequena-média (2.500 células)
│   ├── entrada_100x100.txt         # Entrada média (10.000 células)
│   └── entrada_500x500.txt         # Entrada grande (250.000 células)
│
├── 📁 results/                     # RESULTADOS DAS SIMULAÇÕES
│   └── *.txt                       # Arquivos de resultado gerados
│
├── 📁 docs/                        # DOCUMENTAÇÃO ADICIONAL
│   ├── GUIA_USO.md                 # Guia detalhado passo a passo
│   ├── TEMPLATE_RELATORIO.md       # Template estruturado para relatório
│   ├── INICIO_RAPIDO.txt           # Resumo visual dos comandos
│   └── README_old.md               # README anterior (backup)
│
└── 📁 tests/                       # TESTES UNITÁRIOS (futuro)
```

## 🎯 Arquivos Principais

### 📌 Raiz do Projeto

| Arquivo | Descrição | Tipo |
|---------|-----------|------|
| `README.md` | Documentação principal do projeto | Markdown |
| `LICENSE` | Licença MIT | Texto |
| `Makefile` | Automação de compilação e testes | Make |
| `.gitignore` | Arquivos ignorados pelo Git | Config |

### 📌 Código-Fonte (`src/`)

| Arquivo | Descrição | Linhas | Funcionalidade |
|---------|-----------|--------|----------------|
| `main.c` | Versão MPI paralela | ~343 | Divisão de dados, comunicação, halo |
| `sequencial.c` | Versão sequencial | ~250 | Baseline para comparação |

### 📌 Scripts (`scripts/`)

| Script | Função | Uso |
|--------|--------|-----|
| `gerar_entrada.py` | Gera matrizes aleatórias | `python3 gerar_entrada.py N M K > arquivo.txt` |
| `teste_rapido.sh` | Executa 1 teste de cada | `./teste_rapido.sh data/entrada.txt` |
| `executar_testes.sh` | Executa 3× e calcula média | `./executar_testes.sh data/entrada.txt` |
| `instalar_mpi.sh` | Instala OpenMPI | `./instalar_mpi.sh` |

### 📌 Dados (`data/`)

| Arquivo | Tamanho | Células | Contaminados | Status |
|---------|---------|---------|--------------|--------|
| `entrada_20x20.txt` | 20×20 | 400 | 2 | ✅ Incluído no Git |
| `entrada_50x50.txt` | 50×50 | 2.500 | 3 | ⚠️ Gerado |
| `entrada_100x100.txt` | 100×100 | 10.000 | 5 | ⚠️ Gerado |
| `entrada_500x500.txt` | 500×500 | 250.000 | 10 | ⚠️ Gerado |

### 📌 Documentação (`docs/`)

| Documento | Conteúdo |
|-----------|----------|
| `GUIA_USO.md` | Instruções detalhadas de instalação, compilação e execução |
| `TEMPLATE_RELATORIO.md` | Estrutura completa para o relatório acadêmico |
| `INICIO_RAPIDO.txt` | Resumo visual com comandos principais |
| `README_old.md` | README técnico anterior (backup) |

## 🔄 Fluxo de Trabalho

### 1️⃣ Setup Inicial
```bash
git clone <repo>
cd MPI
./scripts/instalar_mpi.sh    # Instala OpenMPI
make all                       # Compila tudo
```

### 2️⃣ Gerar Entradas
```bash
make gerar_entradas
# OU
python3 scripts/gerar_entrada.py 100 100 5 > data/entrada_custom.txt
```

### 3️⃣ Executar Testes
```bash
# Teste rápido
./scripts/teste_rapido.sh data/entrada_100x100.txt

# Teste completo (3 execuções)
./scripts/executar_testes.sh data/entrada_500x500.txt

# Via Make
make test ENTRADA=data/entrada_100x100.txt
```

### 4️⃣ Analisar Resultados
```bash
ls -lh results/
cat results/resultado_seq.txt
cat results/resultado_mpi_4.txt
```

## 📦 Dependências

### Obrigatórias
- **GCC** (compilador C)
- **OpenMPI** ou MPICH
- **Make**
- **Python 3** (para gerar entradas)

### Opcionais
- **bc** (para cálculos no script de testes)
- **tree** (para visualizar estrutura)

## 🚀 Comandos Principais

```bash
# Compilação
make all              # Compila tudo
make sequencial       # Só sequencial
make main_mpi         # Só MPI

# Execução
make run_seq          # Sequencial
make run_mpi_4        # MPI com 4 processos
make test             # Todos os testes

# Limpeza
make clean            # Remove binários e resultados
make cleanall         # Remove tudo incluindo entradas geradas

# Ajuda
make help             # Mostra todos os comandos
```

## 📊 Tamanho dos Arquivos

```
src/main.c           ~12 KB  (código MPI)
src/sequencial.c     ~8 KB   (código sequencial)
data/entrada_20x20   ~1 KB
data/entrada_100x100 ~30 KB
data/entrada_500x500 ~8 MB
```

## 🎨 Convenções

### Nomenclatura de Arquivos
- **Entrada**: `entrada_NxM.txt` onde N e M são dimensões
- **Resultado**: `resultado_<tipo>_<config>.txt`
  - Exemplo: `resultado_mpi_4.txt` (MPI com 4 processos)
  - Exemplo: `resultado_seq.txt` (sequencial)

### Organização de Código
- Funções em inglês (mais profissional)
- Comentários em português (contexto acadêmico)
- Constantes em MAIÚSCULAS (`VAZIO`, `CONTAMINADO`, etc.)

## ✅ Checklist de Entrega

- [x] Código sequencial implementado
- [x] Código MPI implementado
- [x] Makefile funcional
- [x] Scripts de teste
- [x] Gerador de entradas
- [x] Documentação completa
- [x] README para GitHub
- [x] Template de relatório
- [x] LICENSE
- [x] .gitignore
- [ ] Relatório preenchido com resultados
- [ ] Análise de desempenho completa

## 🔗 Links Úteis

- [Documentação OpenMPI](https://www.open-mpi.org/doc/)
- [MPI Tutorial](https://mpitutorial.com/)
- [Guia de Uso Completo](GUIA_USO.md)
- [Template de Relatório](TEMPLATE_RELATORIO.md)

---

**Data de criação**: 11/11/2025  
**Última atualização**: 11/11/2025  
**Versão**: 1.0
