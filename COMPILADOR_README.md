# AirFryerScript - Compilador Completo + VM

*Aluno: João Otávio Gentil Palma*


Uma linguagem de programacao tematica para "programar" uma air fryer, com compilador completo (lexer, parser, analise semantica, geracao de codigo) e maquina virtual propria.

## Visao Geral

Este projeto implementa um compilador completo para a linguagem AirFryerScript, incluindo:

1. **Front-end**: Analise lexica (Flex) e sintatica (Bison)
2. **Middle-end**: Analise semantica com checagem de tipos
3. **Back-end**: Geracao de codigo assembly para AirFryerVM
4. **Runtime**: Maquina virtual propria (AirFryerVM)

## Estrutura do Projeto

```
aps-logcomp/
├── src/                    # Codigo-fonte do compilador (C)
│   ├── airfryer.l         # Analisador lexico (Flex)
│   ├── airfryer.y         # Analisador sintatico (Bison)
│   ├── ast.h/c            # Arvore Sintatica Abstrata
│   ├── semantic.h/c       # Analise semantica
│   └── codegen.h/c        # Geracao de codigo
├── vm/                     # Maquina Virtual (Python)
│   └── airfryer_vm.py     # AirFryerVM
├── examples/               # Programas de exemplo
│   ├── batata.afs         # Exemplo com loops
│   └── solto.afs          # Exemplo com tipos frac e condicionais
├── build/                  # Arquivos compilados (gerados)
├── grammar/                # Especificacao EBNF
├── docs/                   # Documentacao da linguagem
└── Makefile               # Sistema de build

```

## A Linguagem AirFryerScript

### Tipos de Dados

- `inteiro` - Numeros inteiros
- `frac` - Numeros fracionarios (implementados como fixed-point)
- `bool` - Booleanos (verdadeiro/falso)
- `texto` - Strings (apenas para impressao)

### Comandos Tematicos

```afs
preaquecer temperatura 200 graus celsius;
cozinhar temperatura 180 graus celsius tempo 10 minutos;
aquecer tempo 5 minutos;
agitar aos 4 minutos;
modo batata;  // ou legumes, nuggets, esfihas
pausar;
continuar;
parar;
```

### Estruturas de Controle

```afs
// Condicional
se (temperatura > 180) {
    // ...
} senao {
    // ...
}

// Loop
enquanto (contador < 10) {
    // ...
    contador = contador + 1;
}
```

### Expressoes

Operadores aritmeticos: `+`, `-`, `*`, `/`, `%`
Operadores relacionais: `==`, `!=`, `<`, `<=`, `>`, `>=`
Operadores logicos: `e`, `ou`, `nao`

## AirFryerVM - Maquina Virtual

### Arquitetura

- **Registradores de escrita**: TIME, POWER, R0, R1, R2, R3
- **Sensores read-only**: TEMP, WEIGHT, MODE, STATE
- **Memoria**: Pilha (stack)
- **String table**: Para literais de texto

### Conjunto de Instrucoes (ISA)

#### Instrucoes Basicas
```
SET R n          - Define registrador R = n
INC R            - Incrementa R
DEC R            - Decrementa R
DECJZ R label    - Se R == 0 vai para label, senao R = R - 1
GOTO label       - Pula para label
PUSH R           - Empilha valor de R
POP R            - Desempilha para R
HALT             - Para a execucao
```

#### Instrucoes Aritmeticas (Inteiros)
```
ADD R1 R2        - R1 = R1 + R2
SUB R1 R2        - R1 = R1 - R2
MUL R1 R2        - R1 = R1 * R2
DIV R1 R2        - R1 = R1 / R2
MOD R1 R2        - R1 = R1 % R2
```

#### Instrucoes Aritmeticas (Fixed-Point para tipo frac)
```
ADDF R1 R2       - R1 = R1 + R2 (frac)
SUBF R1 R2       - R1 = R1 - R2 (frac)
MULF R1 R2       - R1 = (R1 * R2) / 100
DIVF R1 R2       - R1 = (R1 * 100) / R2
ITOF R           - R = R * 100
FTOI R           - R = R / 100
```

#### Instrucoes de Comparacao
```
EQ R1 R2         - R1 = (R1 == R2)
NE R1 R2         - R1 = (R1 != R2)
LT R1 R2         - R1 = (R1 < R2)
LE R1 R2         - R1 = (R1 <= R2)
GT R1 R2         - R1 = (R1 > R2)
GE R1 R2         - R1 = (R1 >= R2)
```

#### Instrucoes Logicas
```
AND R1 R2        - R1 = R1 && R2
OR R1 R2         - R1 = R1 || R2
NOT R            - R = !R
```

#### Instrucoes de Salto Condicional
```
JZ R label       - Se R == 0 vai para label
JNZ R label      - Se R != 0 vai para label
```

#### Instrucoes de Impressao
```
PRINT            - Imprime TIME (compatibilidade)
PRINTI R         - Imprime R como inteiro
PRINTF R         - Imprime R como frac (divide por 100)
PRINTB R         - Imprime R como bool
SPRINT id        - Imprime string da string table
```

#### Instrucoes de String
```
SDEF id "texto"  - Define string na string table
```

#### Instrucoes Tematicas
```
SETMODE n        - Define modo (0=manual, 1=batata, 2=legumes, 3=nuggets, 4=esfihas)
PAUSE            - Pausa execucao
RESUME           - Resume execucao
STOP             - Para execucao
```

## Como Usar

### Pre-requisitos

```bash
# Compilador
sudo apt-get install flex bison gcc make

# VM
python3 (versao 3.7+)
```

### Compilacao do Compilador

```bash
make
```

### Compilar um Programa AirFryerScript

```bash
./build/airfryer_parser <programa.afs> -o <saida.mwasm>

# Exemplo:
./build/airfryer_parser examples/batata.afs -o build/batata.mwasm
```

### Executar na VM

```bash
python3 vm/airfryer_vm.py <programa.mwasm>

# Exemplo:
python3 vm/airfryer_vm.py build/batata.mwasm
```

### Opcoes do Compilador

```bash
./build/airfryer_parser <arquivo.afs> [-o <saida.mwasm>] [-debug]
```

- `-o <arquivo>`: Especifica arquivo de saida (padrao: stdout)
- `-debug`: Imprime a AST apos parsing

### Opcoes da VM

```bash
python3 vm/airfryer_vm.py <arquivo.mwasm> [-v] [-d]
```

- `-v, --verbose`: Modo verbose (mostra estado apos cada instrucao)
- `-d, --debug`: Modo debug (passo a passo interativo)

## Exemplos

### Exemplo 1: batata.afs

```afs
programa MinhaAir {
  receita Batata {
    preaquecer temperatura 200 graus celsius;
    aquecer tempo 5 minutos;

    var i: inteiro = 0;
    enquanto (i < 3) {
      cozinhar temperatura 200 graus celsius tempo 8 minutos;
      agitar aos 8 minutos;
      i = i + 1;
    }

    imprimir("Batata finalizada!");
    parar;
  }
}
```

### Exemplo 2: solto.afs

```afs
programa TesteSolto {
  modo legumes;
  var t: inteiro = 10;
  var croc: frac = 1.1;

  se (croc > 1.0) {
    t = t * 1.1;
  } senao {
    t = t;
  }

  cozinhar temperatura 170 graus celsius tempo t minutos;
  imprimir("Legumes prontos em", t, "minutos.");
  parar;
}
```

## Implementacao Tecnica

### Pipeline de Compilacao

```
Codigo Fonte (.afs)
    ↓
[Flex] Analise Lexica
    ↓
Tokens
    ↓
[Bison] Analise Sintatica
    ↓
AST (Arvore Sintatica Abstrata)
    ↓
[semantic.c] Analise Semantica
  - Tabela de simbolos
  - Checagem de tipos
  - Verificacao de declaracoes
    ↓
AST Anotada (com tipos)
    ↓
[codegen.c] Geracao de Codigo
  - Alocacao de registradores
  - Traducao de expressoes
  - Geracao de labels
    ↓
Assembly AirFryerVM (.mwasm)
    ↓
[airfryer_vm.py] Execucao
  - Interpretacao de instrucoes
  - Gerenciamento de memoria
  - Controle de fluxo
    ↓
Saida do Programa
```

### Decisoes de Design

#### Tipos Fixed-Point para `frac`
Numeros fracionarios sao representados como inteiros escalados por 100, permitindo operacoes aritmeticas sem ponto flutuante na VM.

#### Alocacao Estatica de Registradores
Cada variavel e mapeada para um dos 4 registradores de proposito geral (R0-R3). Limitacao atual: maximo de 4 variaveis simultaneas.

#### String Table Pre-compilada
Literais de string sao coletados durante geracao de codigo e emitidos no inicio do assembly via instrucoes SDEF.

#### Instrucoes de Comparacao Destrutivas
Instrucoes como LT e GT modificam o primeiro operando para conter o resultado (0 ou 1), simplificando a geracao de codigo condicional.

## Limitacoes Conhecidas

1. **Maximo de 4 variaveis simultaneas**: devido a alocacao estatica em R0-R3
2. **Operacoes com strings limitadas**: apenas impressao, sem concatenacao
3. **Sem otimizacoes**: codigo gerado e direto mas nao otimizado
4. **Sem garbage collection**: strings na string table nao sao liberadas


