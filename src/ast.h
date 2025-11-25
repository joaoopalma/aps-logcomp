/*
 * ast.h
 * Definicao da Arvore Sintatica Abstrata (AST) para AirFryerScript
 * 
 * Este arquivo define todas as estruturas de dados usadas para representar
 * o programa AirFryerScript apos a analise sintatica.
 */

#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>

/* Tipos de dados da linguagem */
typedef enum {
    TYPE_INTEIRO,
    TYPE_FRAC,
    TYPE_BOOL,
    TYPE_TEXTO,
    TYPE_VOID,     /* Para comandos que nao retornam valor */
    TYPE_UNKNOWN   /* Para expressoes sem tipo definido ainda */
} DataType;

/* Tipos de nos da AST */
typedef enum {
    /* Programa e estrutura */
    NODE_PROGRAMA,
    NODE_RECEITA,
    NODE_PASSO,
    NODE_BLOCO,
    
    /* Declaracoes */
    NODE_DECLARACAO,
    
    /* Comandos */
    NODE_ATRIBUICAO,
    NODE_PREAQUECER,
    NODE_COZINHAR,
    NODE_AQUECER,
    NODE_AGITAR,
    NODE_SET_MODO,
    NODE_PAUSAR,
    NODE_CONTINUAR,
    NODE_PARAR,
    NODE_IMPRIMIR,
    NODE_SE,
    NODE_ENQUANTO,
    
    /* Expressoes */
    NODE_BINOP,        /* Operacao binaria: +, -, *, /, ==, <, etc */
    NODE_UNOP,         /* Operacao unaria: -, nao */
    NODE_LITERAL_INT,
    NODE_LITERAL_FRAC,
    NODE_LITERAL_BOOL,
    NODE_LITERAL_STR,
    NODE_VARIAVEL      /* Referencia a uma variavel */
} NodeKind;

/* Tipos de operadores binarios */
typedef enum {
    OP_ADD,      /* + */
    OP_SUB,      /* - */
    OP_MUL,      /* * */
    OP_DIV,      /* / */
    OP_MOD,      /* % */
    OP_EQ,       /* == */
    OP_NE,       /* != */
    OP_LT,       /* < */
    OP_LE,       /* <= */
    OP_GT,       /* > */
    OP_GE,       /* >= */
    OP_AND,      /* e */
    OP_OR        /* ou */
} BinOpKind;

/* Tipos de operadores unarios */
typedef enum {
    OP_NEG,      /* - (negacao aritmetica) */
    OP_NOT       /* nao (negacao logica) */
} UnOpKind;

/* Tipos de modo da air fryer */
typedef enum {
    MODE_BATATA,
    MODE_LEGUMES,
    MODE_NUGGETS,
    MODE_ESFIHAS
} ModoKind;

/* Unidade de tempo */
typedef enum {
    TIME_MINUTOS,
    TIME_SEGUNDOS
} TimeUnit;

/* Estrutura generica para nos da AST */
typedef struct ASTNode {
    NodeKind kind;
    DataType data_type;  /* Tipo de dado (preenchido na analise semantica) */
    int line;            /* Linha no codigo fonte (para mensagens de erro) */
    
    union {
        /* NODE_PROGRAMA */
        struct {
            char *nome;
            struct ASTNode **top_level_items;
            int num_items;
        } programa;
        
        /* NODE_RECEITA */
        struct {
            char *nome;
            struct ASTNode *bloco;
        } receita;
        
        /* NODE_PASSO */
        struct {
            char *nome;
            struct ASTNode *bloco;
        } passo;
        
        /* NODE_BLOCO */
        struct {
            struct ASTNode **statements;
            int num_statements;
        } bloco;
        
        /* NODE_DECLARACAO */
        struct {
            char *nome;
            DataType tipo;
            struct ASTNode *init_expr;  /* NULL se nao tem inicializacao */
        } declaracao;
        
        /* NODE_ATRIBUICAO */
        struct {
            char *nome;
            struct ASTNode *expr;
        } atribuicao;
        
        /* NODE_PREAQUECER */
        struct {
            struct ASTNode *temperatura;  /* Expressao para temperatura */
        } preaquecer;
        
        /* NODE_COZINHAR */
        struct {
            struct ASTNode *temperatura;
            struct ASTNode *tempo;
            TimeUnit unidade;
        } cozinhar;
        
        /* NODE_AQUECER */
        struct {
            struct ASTNode *tempo;
            TimeUnit unidade;
        } aquecer;
        
        /* NODE_AGITAR */
        struct {
            struct ASTNode *tempo;  /* Momento em que agitar (em minutos) */
        } agitar;
        
        /* NODE_SET_MODO */
        struct {
            ModoKind modo;
        } set_modo;
        
        /* NODE_IMPRIMIR */
        struct {
            struct ASTNode **exprs;
            int num_exprs;
        } imprimir;
        
        /* NODE_SE */
        struct {
            struct ASTNode *condicao;
            struct ASTNode *bloco_then;
            struct ASTNode *bloco_else;  /* NULL se nao tem else */
        } se;
        
        /* NODE_ENQUANTO */
        struct {
            struct ASTNode *condicao;
            struct ASTNode *bloco;
        } enquanto;
        
        /* NODE_BINOP */
        struct {
            BinOpKind op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binop;
        
        /* NODE_UNOP */
        struct {
            UnOpKind op;
            struct ASTNode *operand;
        } unop;
        
        /* NODE_LITERAL_INT */
        struct {
            int value;
        } literal_int;
        
        /* NODE_LITERAL_FRAC */
        struct {
            double value;
        } literal_frac;
        
        /* NODE_LITERAL_BOOL */
        struct {
            int value;  /* 0 = falso, 1 = verdadeiro */
        } literal_bool;
        
        /* NODE_LITERAL_STR */
        struct {
            char *value;
        } literal_str;
        
        /* NODE_VARIAVEL */
        struct {
            char *nome;
        } variavel;
    } data;
} ASTNode;

/* Funcoes para criacao de nos da AST */

/* Criar no de programa */
ASTNode* ast_create_programa(const char *nome, ASTNode **items, int num_items);

/* Criar no de receita */
ASTNode* ast_create_receita(const char *nome, ASTNode *bloco);

/* Criar no de passo */
ASTNode* ast_create_passo(const char *nome, ASTNode *bloco);

/* Criar no de bloco */
ASTNode* ast_create_bloco(ASTNode **statements, int num_statements);

/* Criar no de declaracao */
ASTNode* ast_create_declaracao(const char *nome, DataType tipo, ASTNode *init_expr);

/* Criar no de atribuicao */
ASTNode* ast_create_atribuicao(const char *nome, ASTNode *expr);

/* Criar nos de comandos tematicos */
ASTNode* ast_create_preaquecer(ASTNode *temperatura);
ASTNode* ast_create_cozinhar(ASTNode *temperatura, ASTNode *tempo, TimeUnit unidade);
ASTNode* ast_create_aquecer(ASTNode *tempo, TimeUnit unidade);
ASTNode* ast_create_agitar(ASTNode *tempo);
ASTNode* ast_create_set_modo(ModoKind modo);
ASTNode* ast_create_pausar(void);
ASTNode* ast_create_continuar(void);
ASTNode* ast_create_parar(void);

/* Criar no de imprimir */
ASTNode* ast_create_imprimir(ASTNode **exprs, int num_exprs);

/* Criar nos de controle de fluxo */
ASTNode* ast_create_se(ASTNode *condicao, ASTNode *bloco_then, ASTNode *bloco_else);
ASTNode* ast_create_enquanto(ASTNode *condicao, ASTNode *bloco);

/* Criar nos de expressoes */
ASTNode* ast_create_binop(BinOpKind op, ASTNode *left, ASTNode *right);
ASTNode* ast_create_unop(UnOpKind op, ASTNode *operand);
ASTNode* ast_create_literal_int(int value);
ASTNode* ast_create_literal_frac(double value);
ASTNode* ast_create_literal_bool(int value);
ASTNode* ast_create_literal_str(const char *value);
ASTNode* ast_create_variavel(const char *nome);

/* Adicionar um statement a um bloco (usado durante parsing) */
void ast_bloco_add_statement(ASTNode *bloco, ASTNode *statement);

/* Adicionar um item ao programa (usado durante parsing) */
void ast_programa_add_item(ASTNode *programa, ASTNode *item);

/* Adicionar uma expressao ao imprimir (usado durante parsing) */
void ast_imprimir_add_expr(ASTNode *imprimir, ASTNode *expr);

/* Liberar memoria da AST */
void ast_free(ASTNode *node);

/* Imprimir a AST (para debug) */
void ast_print(ASTNode *node, int depth);

/* Obter nome do tipo de dado como string */
const char* ast_type_name(DataType type);

/* Obter nome do operador binario como string */
const char* ast_binop_name(BinOpKind op);

/* Obter nome do operador unario como string */
const char* ast_unop_name(UnOpKind op);

#endif /* AST_H */
