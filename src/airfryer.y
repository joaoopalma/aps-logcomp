%{
/*
 * airfryer.y
 * Parser para AirFryerScript usando Bison
 * Versao atualizada com AST estruturada, analise semantica e geracao de codigo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantic.h"
#include "codegen.h"

extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern int line_num;

void yyerror(const char *s);

/* Raiz da AST */
ASTNode* root = NULL;
%}

/* Listas temporarias para construcao de nos durante parsing */
%code requires {
    #include "ast.h"
    
    typedef struct {
        ASTNode **items;
        int count;
        int capacity;
    } NodeList;
}

%code {
    NodeList* nodelist_create();
    void nodelist_add(NodeList *list, ASTNode *node);
    void nodelist_free(NodeList *list);
}

%union {
    int int_val;
    double double_val;
    char *str_val;
    ASTNode *node_val;
    DataType type_val;
    ModoKind modo_val;
    TimeUnit time_unit_val;
    BinOpKind binop_val;
    UnOpKind unop_val;
    NodeList *list_val;
}

/* Tokens terminais */
%token <str_val> ID STR_LITERAL
%token <int_val> INT_LITERAL
%token <double_val> DEC_LITERAL

/* Palavras-chave */
%token PROGRAMA RECEITA PASSO
%token VAR INTEIRO FRAC BOOL TEXTO
%token PREAQUECER COZINHAR AQUECER AGITAR MODO
%token TEMPERATURA GRAUS CELSIUS TEMPO MINUTOS SEGUNDOS AOS
%token BATATA LEGUMES NUGGETS ESFIHAS
%token PAUSAR CONTINUAR PARAR IMPRIMIR
%token SE SENAO ENQUANTO
%token VERDADEIRO FALSO
%token E OU NAO

/* Operadores */
%token EQ NE LE GE LT GT
%token PLUS MINUS MULT DIV MOD
%token ASSIGN

/* Delimitadores */
%token LBRACE RBRACE LPAREN RPAREN SEMICOLON COLON COMMA

/* Tipos nao-terminais */
%type <node_val> programa receita passo bloco
%type <node_val> declaracao comando atribuicao
%type <node_val> preaquecer cozinhar aquecer agitar set_modo
%type <node_val> pausar continuar parar imprimir
%type <node_val> condicional repeticao
%type <node_val> temperatura_espec
%type <node_val> expr disj conj neg rel soma produto unario primario
%type <node_val> literal
%type <list_val> top_level_list declaracao_comando_list expr_list
%type <type_val> tipo
%type <modo_val> modo_tipo
%type <time_unit_val> unidade_tempo

/* Precedencia e associatividade */
%left OU
%left E
%right NAO
%left EQ NE LT LE GT GE
%left PLUS MINUS
%left MULT DIV MOD
%right UMINUS

%%

/* ===== PROGRAMA E ORGANIZACAO ===== */

programa:
    PROGRAMA ID LBRACE top_level_list RBRACE {
        /* Criar no do programa com todos os itens */
        $$ = ast_create_programa($2, $4->items, $4->count);
        free($4);  /* Liberar a lista temporaria (mas nao os itens) */
        free($2);
        root = $$;
    }
    ;

top_level_list:
    /* vazio */ {
        $$ = nodelist_create();
    }
    | top_level_list receita {
        $$ = $1;
        nodelist_add($$, $2);
    }
    | top_level_list declaracao {
        $$ = $1;
        nodelist_add($$, $2);
    }
    | top_level_list comando {
        $$ = $1;
        nodelist_add($$, $2);
    }
    ;

receita:
    RECEITA ID bloco {
        $$ = ast_create_receita($2, $3);
        free($2);
    }
    ;

passo:
    PASSO ID bloco {
        $$ = ast_create_passo($2, $3);
        free($2);
    }
    ;

bloco:
    LBRACE declaracao_comando_list RBRACE {
        $$ = ast_create_bloco($2->items, $2->count);
        free($2);
    }
    ;

declaracao_comando_list:
    /* vazio */ {
        $$ = nodelist_create();
    }
    | declaracao_comando_list declaracao {
        $$ = $1;
        nodelist_add($$, $2);
    }
    | declaracao_comando_list comando {
        $$ = $1;
        nodelist_add($$, $2);
    }
    ;

/* ===== DECLARACOES ===== */

declaracao:
    VAR ID COLON tipo SEMICOLON {
        $$ = ast_create_declaracao($2, $4, NULL);
        $$->line = line_num;
        free($2);
    }
    | VAR ID COLON tipo ASSIGN expr SEMICOLON {
        $$ = ast_create_declaracao($2, $4, $6);
        $$->line = line_num;
        free($2);
    }
    ;

tipo:
    INTEIRO { $$ = TYPE_INTEIRO; }
    | FRAC { $$ = TYPE_FRAC; }
    | BOOL { $$ = TYPE_BOOL; }
    | TEXTO { $$ = TYPE_TEXTO; }
    ;

/* ===== COMANDOS ===== */

comando:
    atribuicao SEMICOLON { $$ = $1; }
    | preaquecer SEMICOLON { $$ = $1; }
    | cozinhar SEMICOLON { $$ = $1; }
    | aquecer SEMICOLON { $$ = $1; }
    | agitar SEMICOLON { $$ = $1; }
    | set_modo SEMICOLON { $$ = $1; }
    | pausar SEMICOLON { $$ = $1; }
    | continuar SEMICOLON { $$ = $1; }
    | parar SEMICOLON { $$ = $1; }
    | imprimir SEMICOLON { $$ = $1; }
    | condicional { $$ = $1; }
    | repeticao { $$ = $1; }
    | passo { $$ = $1; }
    | bloco { $$ = $1; }
    ;

atribuicao:
    ID ASSIGN expr {
        $$ = ast_create_atribuicao($1, $3);
        $$->line = line_num;
        free($1);
    }
    ;

preaquecer:
    PREAQUECER temperatura_espec {
        $$ = ast_create_preaquecer($2);
        $$->line = line_num;
    }
    ;

cozinhar:
    COZINHAR temperatura_espec TEMPO expr unidade_tempo {
        $$ = ast_create_cozinhar($2, $4, $5);
        $$->line = line_num;
    }
    ;

aquecer:
    AQUECER TEMPO expr unidade_tempo {
        $$ = ast_create_aquecer($3, $4);
        $$->line = line_num;
    }
    ;

agitar:
    AGITAR AOS expr MINUTOS {
        $$ = ast_create_agitar($3);
        $$->line = line_num;
    }
    ;

set_modo:
    MODO modo_tipo {
        $$ = ast_create_set_modo($2);
        $$->line = line_num;
    }
    ;

modo_tipo:
    BATATA { $$ = MODE_BATATA; }
    | LEGUMES { $$ = MODE_LEGUMES; }
    | NUGGETS { $$ = MODE_NUGGETS; }
    | ESFIHAS { $$ = MODE_ESFIHAS; }
    ;

unidade_tempo:
    MINUTOS { $$ = TIME_MINUTOS; }
    | SEGUNDOS { $$ = TIME_SEGUNDOS; }
    ;

pausar:
    PAUSAR {
        $$ = ast_create_pausar();
        $$->line = line_num;
    }
    ;

continuar:
    CONTINUAR {
        $$ = ast_create_continuar();
        $$->line = line_num;
    }
    ;

parar:
    PARAR {
        $$ = ast_create_parar();
        $$->line = line_num;
    }
    ;

imprimir:
    IMPRIMIR LPAREN expr_list RPAREN {
        $$ = ast_create_imprimir($3->items, $3->count);
        $$->line = line_num;
        free($3);
    }
    ;

expr_list:
    expr {
        $$ = nodelist_create();
        nodelist_add($$, $1);
    }
    | expr_list COMMA expr {
        $$ = $1;
        nodelist_add($$, $3);
    }
    ;

condicional:
    SE LPAREN expr RPAREN bloco {
        $$ = ast_create_se($3, $5, NULL);
        $$->line = line_num;
    }
    | SE LPAREN expr RPAREN bloco SENAO bloco {
        $$ = ast_create_se($3, $5, $7);
        $$->line = line_num;
    }
    ;

repeticao:
    ENQUANTO LPAREN expr RPAREN bloco {
        $$ = ast_create_enquanto($3, $5);
        $$->line = line_num;
    }
    ;

temperatura_espec:
    TEMPERATURA expr GRAUS CELSIUS {
        $$ = $2;
    }
    ;

/* ===== EXPRESSOES ===== */

expr:
    disj { $$ = $1; }
    ;

disj:
    conj { $$ = $1; }
    | disj OU conj {
        $$ = ast_create_binop(OP_OR, $1, $3);
        $$->line = line_num;
    }
    ;

conj:
    neg { $$ = $1; }
    | conj E neg {
        $$ = ast_create_binop(OP_AND, $1, $3);
        $$->line = line_num;
    }
    ;

neg:
    rel { $$ = $1; }
    | NAO neg {
        $$ = ast_create_unop(OP_NOT, $2);
        $$->line = line_num;
    }
    ;

rel:
    soma { $$ = $1; }
    | soma EQ soma {
        $$ = ast_create_binop(OP_EQ, $1, $3);
        $$->line = line_num;
    }
    | soma NE soma {
        $$ = ast_create_binop(OP_NE, $1, $3);
        $$->line = line_num;
    }
    | soma LT soma {
        $$ = ast_create_binop(OP_LT, $1, $3);
        $$->line = line_num;
    }
    | soma LE soma {
        $$ = ast_create_binop(OP_LE, $1, $3);
        $$->line = line_num;
    }
    | soma GT soma {
        $$ = ast_create_binop(OP_GT, $1, $3);
        $$->line = line_num;
    }
    | soma GE soma {
        $$ = ast_create_binop(OP_GE, $1, $3);
        $$->line = line_num;
    }
    ;

soma:
    produto { $$ = $1; }
    | soma PLUS produto {
        $$ = ast_create_binop(OP_ADD, $1, $3);
        $$->line = line_num;
    }
    | soma MINUS produto {
        $$ = ast_create_binop(OP_SUB, $1, $3);
        $$->line = line_num;
    }
    ;

produto:
    unario { $$ = $1; }
    | produto MULT unario {
        $$ = ast_create_binop(OP_MUL, $1, $3);
        $$->line = line_num;
    }
    | produto DIV unario {
        $$ = ast_create_binop(OP_DIV, $1, $3);
        $$->line = line_num;
    }
    | produto MOD unario {
        $$ = ast_create_binop(OP_MOD, $1, $3);
        $$->line = line_num;
    }
    ;

unario:
    primario { $$ = $1; }
    | MINUS unario %prec UMINUS {
        $$ = ast_create_unop(OP_NEG, $2);
        $$->line = line_num;
    }
    ;

primario:
    literal { $$ = $1; }
    | ID {
        $$ = ast_create_variavel($1);
        $$->line = line_num;
        free($1);
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    ;

literal:
    INT_LITERAL {
        $$ = ast_create_literal_int($1);
        $$->line = line_num;
    }
    | DEC_LITERAL {
        $$ = ast_create_literal_frac($1);
        $$->line = line_num;
    }
    | STR_LITERAL {
        $$ = ast_create_literal_str($1);
        $$->line = line_num;
        free($1);
    }
    | VERDADEIRO {
        $$ = ast_create_literal_bool(1);
        $$->line = line_num;
    }
    | FALSO {
        $$ = ast_create_literal_bool(0);
        $$->line = line_num;
    }
    ;

%%

/* ===== FUNCOES AUXILIARES ===== */

void yyerror(const char *s) {
    fprintf(stderr, "Erro sintatico na linha %d: %s\n", line_num, s);
}

/* Criar uma nova lista de nos */
NodeList* nodelist_create() {
    NodeList *list = (NodeList*)malloc(sizeof(NodeList));
    list->capacity = 8;
    list->count = 0;
    list->items = (ASTNode**)malloc(list->capacity * sizeof(ASTNode*));
    return list;
}

/* Adicionar um no a lista */
void nodelist_add(NodeList *list, ASTNode *node) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->items = (ASTNode**)realloc(list->items, list->capacity * sizeof(ASTNode*));
    }
    list->items[list->count++] = node;
}

/* Liberar a lista (mas nao os nos) */
void nodelist_free(NodeList *list) {
    free(list->items);
    free(list);
}

/* ===== MAIN ===== */

int main(int argc, char **argv) {
    /* Verificar argumentos */
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.afs> [-o <saida.mwasm>] [-debug]\n", argv[0]);
        return 1;
    }
    
    /* Abrir arquivo de entrada */
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo %s\n", argv[1]);
        return 1;
    }
    yyin = file;
    
    /* Verificar opcoes */
    int debug_mode = 0;
    FILE *output = stdout;
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-debug") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output = fopen(argv[i + 1], "w");
            if (!output) {
                fprintf(stderr, "Erro: nao foi possivel criar arquivo de saida %s\n", argv[i + 1]);
                output = stdout;
            }
            i++;
        }
    }
    
    /* Parser */
    fprintf(stderr, "Iniciando analise de %s...\n", argv[1]);
    
    if (yyparse() != 0) {
        fprintf(stderr, "Erro: falha na analise sintatica.\n");
        fclose(file);
        if (output != stdout) fclose(output);
        return 1;
    }
    
    fprintf(stderr, "Analise sintatica concluida com sucesso.\n");
    
    /* Debug: imprimir AST */
    if (debug_mode) {
        fprintf(stderr, "\n=== Arvore Sintatica Abstrata ===\n");
        ast_print(root, 0);
        fprintf(stderr, "\n");
    }
    
    /* Analise semantica */
    fprintf(stderr, "Realizando analise semantica...\n");
    SemanticErrorList *errors = error_list_create();
    
    if (!semantic_analyze(root, errors)) {
        fprintf(stderr, "\n");
        error_list_print(errors);
        fprintf(stderr, "\nErro: falha na analise semantica.\n");
        error_list_free(errors);
        ast_free(root);
        fclose(file);
        if (output != stdout) fclose(output);
        return 1;
    }
    
    fprintf(stderr, "Analise semantica concluida com sucesso.\n");
    error_list_free(errors);
    
    /* Geracao de codigo */
    fprintf(stderr, "Gerando codigo assembly...\n");
    CodeGenerator *codegen = codegen_create(output);
    
    if (!codegen_generate(codegen, root)) {
        fprintf(stderr, "Erro: falha na geracao de codigo.\n");
        codegen_free(codegen);
        ast_free(root);
        fclose(file);
        if (output != stdout) fclose(output);
        return 1;
    }
    
    fprintf(stderr, "Codigo gerado com sucesso.\n");
    
    /* Limpeza */
    codegen_free(codegen);
    ast_free(root);
    fclose(file);
    if (output != stdout) fclose(output);
    
    fprintf(stderr, "Compilacao concluida!\n");
    
    return 0;
}
