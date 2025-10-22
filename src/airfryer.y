%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int yylex();
extern int yyparse();
extern FILE *yyin;
extern int line_num;

void yyerror(const char *s);

/* Estrutura para a árvore sintática (simplificada para esta fase) */
typedef struct node {
    char *type;
    char *value;
    struct node **children;
    int num_children;
} node_t;

node_t* create_node(const char* type, const char* value);
void add_child(node_t* parent, node_t* child);
void print_tree(node_t* root, int depth);
void free_tree(node_t* root);

node_t* root = NULL;
%}

%union {
    int int_val;
    double double_val;
    char *str_val;
    struct node *node_val;
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

/* Tipos não-terminais */
%type <node_val> programa top_level receita passo bloco
%type <node_val> declaracao tipo comando atribuicao
%type <node_val> preaquecer cozinhar aquecer agitar set_modo
%type <node_val> pausar continuar parar imprimir
%type <node_val> condicional repeticao
%type <node_val> temperatura_espec duracao
%type <node_val> expr disj conj neg rel soma produto unario primario
%type <node_val> literal numero
%type <node_val> top_level_list declaracao_comando_list expr_list

/* Precedência e associatividade */
%left OU
%left E
%right NAO
%left EQ NE LT LE GT GE
%left PLUS MINUS
%left MULT DIV MOD
%right UMINUS  /* operador unário menos */

%%

/* ---------- Programa e organização ---------- */
programa:
    PROGRAMA ID LBRACE top_level_list RBRACE {
        $$ = create_node("programa", $2);
        add_child($$, $4);
        root = $$;
        printf("Análise sintática concluída com sucesso!\n");
    }
    ;

top_level_list:
    /* vazio */ {
        $$ = create_node("top_level_list", "");
    }
    | top_level_list top_level {
        $$ = $1;
        add_child($$, $2);
    }
    ;

top_level:
    receita { $$ = $1; }
    | declaracao { $$ = $1; }
    | comando { $$ = $1; }
    ;

receita:
    RECEITA ID bloco {
        $$ = create_node("receita", $2);
        add_child($$, $3);
    }
    ;

passo:
    PASSO ID bloco {
        $$ = create_node("passo", $2);
        add_child($$, $3);
    }
    ;

bloco:
    LBRACE declaracao_comando_list RBRACE {
        $$ = create_node("bloco", "");
        add_child($$, $2);
    }
    ;

declaracao_comando_list:
    /* vazio */ {
        $$ = create_node("declaracao_comando_list", "");
    }
    | declaracao_comando_list declaracao {
        $$ = $1;
        add_child($$, $2);
    }
    | declaracao_comando_list comando {
        $$ = $1;
        add_child($$, $2);
    }
    ;

/* ---------- Declarações ---------- */
declaracao:
    VAR ID COLON tipo SEMICOLON {
        $$ = create_node("declaracao", $2);
        add_child($$, $4);
    }
    | VAR ID COLON tipo ASSIGN expr SEMICOLON {
        $$ = create_node("declaracao_com_init", $2);
        add_child($$, $4);
        add_child($$, $6);
    }
    ;

tipo:
    INTEIRO { $$ = create_node("tipo", "inteiro"); }
    | FRAC { $$ = create_node("tipo", "frac"); }
    | BOOL { $$ = create_node("tipo", "bool"); }
    | TEXTO { $$ = create_node("tipo", "texto"); }
    ;

/* ---------- Comandos ---------- */
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
        $$ = create_node("atribuicao", $1);
        add_child($$, $3);
    }
    ;

preaquecer:
    PREAQUECER temperatura_espec {
        $$ = create_node("preaquecer", "");
        add_child($$, $2);
    }
    ;

cozinhar:
    COZINHAR temperatura_espec TEMPO duracao {
        $$ = create_node("cozinhar", "");
        add_child($$, $2);
        add_child($$, $4);
    }
    ;

aquecer:
    AQUECER TEMPO duracao {
        $$ = create_node("aquecer", "");
        add_child($$, $3);
    }
    ;

agitar:
    AGITAR AOS expr MINUTOS {
        $$ = create_node("agitar", "");
        add_child($$, $3);
    }
    ;

set_modo:
    MODO BATATA { $$ = create_node("modo", "batata"); }
    | MODO LEGUMES { $$ = create_node("modo", "legumes"); }
    | MODO NUGGETS { $$ = create_node("modo", "nuggets"); }
    | MODO ESFIHAS { $$ = create_node("modo", "esfihas"); }
    ;

pausar:
    PAUSAR { $$ = create_node("pausar", ""); }
    ;

continuar:
    CONTINUAR { $$ = create_node("continuar", ""); }
    ;

parar:
    PARAR { $$ = create_node("parar", ""); }
    ;

imprimir:
    IMPRIMIR LPAREN expr_list RPAREN {
        $$ = create_node("imprimir", "");
        add_child($$, $3);
    }
    ;

expr_list:
    expr {
        $$ = create_node("expr_list", "");
        add_child($$, $1);
    }
    | expr_list COMMA expr {
        $$ = $1;
        add_child($$, $3);
    }
    ;

condicional:
    SE LPAREN expr RPAREN bloco {
        $$ = create_node("se", "");
        add_child($$, $3);
        add_child($$, $5);
    }
    | SE LPAREN expr RPAREN bloco SENAO bloco {
        $$ = create_node("se_senao", "");
        add_child($$, $3);
        add_child($$, $5);
        add_child($$, $7);
    }
    ;

repeticao:
    ENQUANTO LPAREN expr RPAREN bloco {
        $$ = create_node("enquanto", "");
        add_child($$, $3);
        add_child($$, $5);
    }
    ;

temperatura_espec:
    TEMPERATURA expr GRAUS CELSIUS {
        $$ = create_node("temperatura", "");
        add_child($$, $2);
    }
    ;

duracao:
    expr MINUTOS {
        $$ = create_node("duracao_minutos", "");
        add_child($$, $1);
    }
    | expr SEGUNDOS {
        $$ = create_node("duracao_segundos", "");
        add_child($$, $1);
    }
    ;

/* ---------- Expressões ---------- */
expr:
    disj { $$ = $1; }
    ;

disj:
    conj { $$ = $1; }
    | disj OU conj {
        $$ = create_node("ou", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    ;

conj:
    neg { $$ = $1; }
    | conj E neg {
        $$ = create_node("e", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    ;

neg:
    rel { $$ = $1; }
    | NAO neg {
        $$ = create_node("nao", "");
        add_child($$, $2);
    }
    ;

rel:
    soma { $$ = $1; }
    | soma EQ soma {
        $$ = create_node("==", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | soma NE soma {
        $$ = create_node("!=", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | soma LT soma {
        $$ = create_node("<", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | soma LE soma {
        $$ = create_node("<=", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | soma GT soma {
        $$ = create_node(">", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | soma GE soma {
        $$ = create_node(">=", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    ;

soma:
    produto { $$ = $1; }
    | soma PLUS produto {
        $$ = create_node("+", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | soma MINUS produto {
        $$ = create_node("-", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    ;

produto:
    unario { $$ = $1; }
    | produto MULT unario {
        $$ = create_node("*", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | produto DIV unario {
        $$ = create_node("/", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    | produto MOD unario {
        $$ = create_node("%", "");
        add_child($$, $1);
        add_child($$, $3);
    }
    ;

unario:
    primario { $$ = $1; }
    | MINUS unario %prec UMINUS {
        $$ = create_node("unario_minus", "");
        add_child($$, $2);
    }
    ;

primario:
    literal { $$ = $1; }
    | ID {
        $$ = create_node("id", $1);
    }
    | LPAREN expr RPAREN {
        $$ = $2;
    }
    ;

literal:
    numero { $$ = $1; }
    | STR_LITERAL {
        $$ = create_node("string", $1);
    }
    | VERDADEIRO {
        $$ = create_node("bool", "verdadeiro");
    }
    | FALSO {
        $$ = create_node("bool", "falso");
    }
    ;

numero:
    INT_LITERAL {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%d", $1);
        $$ = create_node("int", strdup(buffer));
    }
    | DEC_LITERAL {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2f", $1);
        $$ = create_node("frac", strdup(buffer));
    }
    ;

%%

/* ---------- Funções auxiliares ---------- */

void yyerror(const char *s) {
    printf("Erro sintático na linha %d: %s\n", line_num, s);
}

node_t* create_node(const char* type, const char* value) {
    node_t* node = malloc(sizeof(node_t));
    node->type = strdup(type);
    node->value = value ? strdup(value) : strdup("");
    node->children = NULL;
    node->num_children = 0;
    return node;
}

void add_child(node_t* parent, node_t* child) {
    parent->children = realloc(parent->children, 
                              (parent->num_children + 1) * sizeof(node_t*));
    parent->children[parent->num_children] = child;
    parent->num_children++;
}

void print_tree(node_t* root, int depth) {
    if (!root) return;
    
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    printf("<%s>", root->type);
    if (strlen(root->value) > 0) {
        printf(": %s", root->value);
    }
    printf("\n");
    
    for (int i = 0; i < root->num_children; i++) {
        print_tree(root->children[i], depth + 1);
    }
}

void free_tree(node_t* root) {
    if (!root) return;
    
    for (int i = 0; i < root->num_children; i++) {
        free_tree(root->children[i]);
    }
    
    free(root->type);
    free(root->value);
    free(root->children);
    free(root);
}

int main(int argc, char **argv) {
    if (argc > 1) {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            printf("Erro: não foi possível abrir o arquivo %s\n", argv[1]);
            return 1;
        }
        yyin = file;
    }
    
    printf("Iniciando análise de %s...\n", argc > 1 ? argv[1] : "entrada padrão");
    
    if (yyparse() == 0) {
        printf("\n=== Árvore Sintática ===\n");
        print_tree(root, 0);
        free_tree(root);
        printf("\nParsing bem-sucedido!\n");
    } else {
        printf("Parsing falhou.\n");
        return 1;
    }
    
    if (argc > 1) {
        fclose(yyin);
    }
    
    return 0;
}