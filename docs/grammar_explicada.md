# AirFryerScript — Gramática Explicada

Arquivo da gramática: [`grammar/grammar.ebnf`](../grammar/grammar.ebnf)

---

## 1) Estrutura geral

- **programa**  
  Todo arquivo começa com:
  ```afs
  programa NomeDoPrograma { ... }
  ```

O bloco do programa pode conter receitas, declarações e (opcionalmente) comandos.

- **receita**
Agrupa comandos por prato/rotina:
  ```afs
  receita Batata { ... }
  ```

- **passo** (opcional)
Subdivisão dentro de uma receita, apenas organizacional:
  ```afs
  passo Preparo { ... }
  ```

- **bloco**
Qualquer trecho ```{ ... }``` com declarações e comandos (aparece em receitas, passos, ```se```, ```enquanto```, etc.).

## 2) Declarações e tipos

- **declaração de variável**
```afs
  var nome: tipo = expressão;   // inicialização é opcional
  ```

- **tipos primitivos**
    - `inteiro` — contagens, minutos, graus inteiros;

    - `frac` — números fracionários (p.ex. 1.5, 1.1 para +10%);
    
    - `bool` — verdadeiro/falso;
     
    - `texto` — strings para imprimir.

## 3) Comandos temáticos da air fryer

- **preaquecer**
  ```afs
  preaquecer temperatura 200 graus celsius;
  ```

- **cozinhar**
  ```afs
  cozinhar temperatura 200 graus celsius tempo 8 minutos;
  ```

- **aquecer** (sem temperatura; útil p/ manter quente ou pré)
  ```afs
  aquecer tempo 5 minutos;
  ```

- **agitar** (evento no meio do ciclo)
  ```afs
  agitar aos 4 minutos;
  ```

- **modo** (presets: batata, legumes, nuggets, esfihas)
  ```afs
  modo batata;
  ```

*Semântica típica* (definida no backend): cada modo seta temperatura/tempo padrão, podendo ser sobrescrito por comandos posteriores.

- **controle de execução**
  ```afs
  pausar; continuar; parar;
  ```

- **saída**
  ```afs
  imprimir("Mensagem", valor, ...);
  ```

- **atribuição**
  ```afs
  x = expressão;
  ```

## 4) Controle de fluxo (if e while)

- **condicional**
  ```afs
  se (expressao_booleana) { ... }
  senao { ... }
  ```

- **laço**
  ```afs
  enquanto (expressao_booleana) { ... }
  ```

## 5) Expressões e precedência

Produções (da menor para a maior precedência lógica/aritmética):

```afs
expr     → disj
disj     → conj  { "ou" conj }
conj     → neg   { "e"  neg }
neg      → [ "nao" ] rel
rel      → soma  [ ( "==" | "!=" | "<" | "<=" | ">" | ">=" ) soma ]
soma     → produto { ( "+" | "-" ) produto }
produto  → unario { ( "*" | "/" ) unario }
unario   → [ "-" ] primario
primario → literal | ID | "(" expr ")"
literal  → numero | STR | "verdadeiro" | "falso"
numero   → INT | DEC
```

- **Booleanos:** `e`, `ou`, `nao`

- **Comparações:** `==`, `!=`, `<`, `<=`, `>`, `>=`

- **Aritmética:** `+`, `-`, `*`, `/`
 
- **Léxico básico:** `ID` (identificadores), `STR` ("texto"), `INT` (inteiros), `DEC` (decimais para `frac`)

**Exe,plos rápidos**
```afs
  nao (x < 10 e y == 2)
  t = t * 1.1
  enquanto (contador > 0) { contador = contador - 1; }
  ```

## 6) Léxico (resumo)

- **ID**: letra inicial, seguida de letras/dígitos/`_`

- **INT**: `0`, `15`, `200`
 
- **DEC**: `15.5`, `180.0` (mapeia para `frac`)
 
- **STR**: `"qualquer texto"`
 
- **Comentários**: `//` até o fim da linha, ou `/* ... */`
 
- **Espaços**: ignorados fora de strings