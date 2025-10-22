# Loops na AirFryerScript

## Estrutura do Loop: `enquanto`

A AirFryerScript implementa loops usando a palavra-chave `enquanto`:

```afs
enquanto (condição_booleana) {
    // comandos a serem repetidos
}
```

## Características dos Loops

### 1. **Condições Flexíveis**
Os loops suportam qualquer expressão booleana:
- Comparações: `<`, `<=`, `>`, `>=`, `==`, `!=`
- Operadores lógicos: `e`, `ou`, `nao`
- Variáveis e valores literais

### 2. **Controle de Variáveis**
Variáveis podem ser:
- Declaradas antes do loop
- Modificadas dentro do loop
- Usadas como contadores ou condições

### 3. **Aninhamento**
Loops podem ser aninhados para operações complexas.

## Exemplos Práticos

### Exemplo 1: Loop Básico (Contador)
```afs
programa LoopBasico {
  receita ContadorSimples {
    var i: inteiro = 0;
    
    enquanto (i < 3) {
      imprimir("Cozinhando porção", i);
      cozinhar temperatura 180 graus celsius tempo 5 minutos;
      i = i + 1;
    }
  }
}
```

### Exemplo 2: Loop com Condição Variável
```afs
programa LoopCondicional {
  receita AjusteTemperatura {
    var temperatura: inteiro = 120;
    var tentativas: inteiro = 0;
    
    enquanto (temperatura < 200) {
      imprimir("Aquecendo... Temperatura:", temperatura);
      aquecer tempo 2 minutos;
      temperatura = temperatura + 30;
      tentativas = tentativas + 1;
    }
    
    imprimir("Temperatura ideal alcançada em", tentativas, "tentativas");
  }
}
```

### Exemplo 3: Loops Aninhados
```afs
programa LoopsAninhados {
  receita ProcessamentoLotes {
    var lote: inteiro = 1;
    var peca: inteiro = 1;
    
    enquanto (lote <= 2) {
      imprimir("=== Lote", lote, "===");
      peca = 1;
      
      enquanto (peca <= 3) {
        imprimir("Processando peça", peca, "do lote", lote);
        cozinhar temperatura 160 graus celsius tempo 4 minutos;
        peca = peca + 1;
      }
      
      lote = lote + 1;
    }
  }
}
```

### Exemplo 4: Loop com Condições Complexas
```afs
programa LoopComplexo {
  receita MonitoramentoContinuo {
    var ciclos: inteiro = 0;
    var temperatura: inteiro = 100;
    var crocancia: frac = 0.5;
    
    enquanto (ciclos < 10 e temperatura < 200 e crocancia < 0.9) {
      // Processo de cozimento adaptativo
      se (temperatura < 150) {
        aquecer tempo 3 minutos;
        temperatura = temperatura + 25;
      } senao {
        cozinhar temperatura temperatura graus celsius tempo 2 minutos;
        crocancia = crocancia + 0.1;
      }
      
      ciclos = ciclos + 1;
      imprimir("Ciclo", ciclos, "- Temp:", temperatura, "Crocância:", crocancia);
    }
  }
}
```

## Operadores Suportados em Loops

### Comparação
- `<` - menor que
- `<=` - menor ou igual
- `>` - maior que  
- `>=` - maior ou igual
- `==` - igual
- `!=` - diferente

### Lógicos
- `e` - E lógico (AND)
- `ou` - OU lógico (OR)
- `nao` - NÃO lógico (NOT)

### Aritméticos (para contadores)
- `+` - adição
- `-` - subtração  
- `*` - multiplicação
- `/` - divisão

## Casos de Uso Práticos

### 1. **Repetição de Processos**
```afs
// Cozinhar múltiplas porções
enquanto (porcoes_restantes > 0) {
    cozinhar temperatura 180 graus celsius tempo 8 minutos;
    porcoes_restantes = porcoes_restantes - 1;
}
```

### 2. **Ajuste Automático**
```afs
// Ajustar temperatura até ideal
enquanto (temperatura_atual < temperatura_desejada) {
    aquecer tempo 1 minutos;
    temperatura_atual = temperatura_atual + 10;
}
```

### 3. **Processamento em Lotes**
```afs
// Processar vários lotes
enquanto (lote_atual <= total_lotes) {
    modo batata;
    cozinhar temperatura 200 graus celsius tempo 12 minutos;
    lote_atual = lote_atual + 1;
}
```

## Integração com Comandos da Air Fryer

Os loops funcionam perfeitamente com todos os comandos temáticos:

- `preaquecer` - dentro de loops para múltiplos ciclos
- `cozinhar` - para processar várias porções
- `aquecer` - para ajustes graduais
- `agitar` - em intervalos regulares
- `modo` - para alternar entre configurações
- `imprimir` - para logging durante loops
- `se/senao` - para lógica condicional dentro de loops

## Conclusão

A AirFryerScript oferece um sistema de loops robusto e flexível que permite:

1. **Repetição controlada** de operações de cozimento
2. **Condições complexas** com múltiplos operadores
3. **Aninhamento** para operações hierárquicas  
4. **Integração completa** com todos os comandos da linguagem
5. **Controle fino** através de variáveis e expressões