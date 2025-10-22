# AirFryerScript

Uma linguagem temática para “programar” uma air fryer, com variáveis, condicionais e laços, que compila para uma VM simples.

## Visão Geral

**AirFryerScript** permite descrever receitas e rotinas de preparo com comandos como `preaquecer`, `cozinhar`, `agitar`, `modo batata/legumes/nuggets/esfihas`, além de estruturas de programação (`var`, `se/senao`, `enquanto`).  
O objetivo é **atender à APS** de Lógica da Computação conforme a rubrica: definir a linguagem (EBNF), construir **lexer**/**parser** com **Flex/Bison** e **gerar assembly** para uma máquina virtual (VM). :contentReference[oaicite:1]{index=1}

## Alinhamento com a APS

- **Tarefa #1:** Linguagem estruturada em **EBNF** (este repositório inclui `grammar/grammar.ebnf`).  
- **Tarefa #2:** **Flex/Bison** para análise léxica/sintática (semântica/compilação ficam para a próxima etapa).  
- **Tarefa #3:** Gerar **assembly** para uma **VM** (MicrowaveVM, LLVM, JVM, .NET ou **sua própria VM**).  
- **Tarefa #4:** Exemplos que demonstrem as características da linguagem.  
- **Tarefa #5:** **PPT/PDF** com motivação, características e exemplos (no `docs/`).  
Entrega final: **01/Dez/2025 08:00**. Enviar o **nome do repositório** no Blackboard.  
Referência oficial: APS – Log Comp 2025/2. :contentReference[oaicite:2]{index=2}

> **Bônus:** Criar uma **VM nova** rende **+1 conceito** (a nota pode extrapolar A+). A VM precisa ter **≥2 registradores**, **memória/pilha/listas**, **sensores read-only** e instruções suficientes para **Turing-completude** (estilo Minsky). :contentReference[oaicite:3]{index=3}

## Linguagem

### Tipos
- `inteiro` — números inteiros (minutos, contadores).
- `frac` — números com parte fracionária (ajustes percentuais, médias, conversões).
- `bool` — `verdadeiro` / `falso`.
- `texto` — strings.

### Comandos temáticos
- `preaquecer temperatura N graus celsius;`
- `cozinhar temperatura N graus celsius tempo M minutos|segundos;`
- `aquecer tempo M minutos|segundos;`
- `agitar aos N minutos;`
- `modo batata|legumes|nuggets|esfihas;`
- `pausar; continuar; parar;`
- `imprimir("...");`

### Estruturas de controle
- `se (expr) { ... } [ senao { ... } ]` — Condicionais
- `enquanto (expr) { ... }` — **Loops com capacidade computacional completa**

#### Exemplos de Loops:
```afs
// Loop simples com contador
var i: inteiro = 0;
enquanto (i < 5) {
    cozinhar temperatura 180 graus celsius tempo 3 minutos;
    i = i + 1;
}

// Loop com múltiplas condições
var temp: inteiro = 150;
var tempo: inteiro = 0;
enquanto (temp < 200 e tempo < 30) {
    preaquecer temperatura temp graus celsius;
    temp = temp + 10;
    tempo = tempo + 5;
}

// Loops aninhados para receitas complexas
var ciclo: inteiro = 1;
enquanto (ciclo <= 3) {
    var fase: inteiro = 1;
    enquanto (fase <= 2) {
        cozinhar temperatura 200 graus celsius tempo 8 minutos;
        agitar aos 4 minutos;
        fase = fase + 1;
    }
    ciclo = ciclo + 1;
}
```

**Documentação completa de loops**: [`docs/loops_explicacao.md`](docs/loops_explicacao.md)

### Expressões
- Aritmética: `+ - * /`  
- Comparações: `== != < <= > >=`  
- Booleanos: `e`, `ou`, `nao`  
- Precedência padrão: `nao` > `* /` > `+ -` > comparações > `e` > `ou`.

### EBNF
A gramática completa está em [`grammar/grammar.ebnf`](grammar/grammar.ebnf).
