/*
 * ast.c
 * Implementacao das funcoes para manipulacao da AST
 */

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Funcao auxiliar para alocar um no da AST */
static ASTNode* ast_alloc_node(NodeKind kind) {
    ASTNode *node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) {
        fprintf(stderr, "Erro fatal: falha ao alocar memoria para no da AST\n");
        exit(1);
    }
    node->kind = kind;
    node->data_type = TYPE_UNKNOWN;
    node->line = 0;  /* Sera preenchido pelo parser */
    return node;
}

/* Criar no de programa */
ASTNode* ast_create_programa(const char *nome, ASTNode **items, int num_items) {
    ASTNode *node = ast_alloc_node(NODE_PROGRAMA);
    node->data.programa.nome = strdup(nome);
    node->data.programa.top_level_items = items;
    node->data.programa.num_items = num_items;
    return node;
}

/* Criar no de receita */
ASTNode* ast_create_receita(const char *nome, ASTNode *bloco) {
    ASTNode *node = ast_alloc_node(NODE_RECEITA);
    node->data.receita.nome = strdup(nome);
    node->data.receita.bloco = bloco;
    return node;
}

/* Criar no de passo */
ASTNode* ast_create_passo(const char *nome, ASTNode *bloco) {
    ASTNode *node = ast_alloc_node(NODE_PASSO);
    node->data.passo.nome = strdup(nome);
    node->data.passo.bloco = bloco;
    return node;
}

/* Criar no de bloco */
ASTNode* ast_create_bloco(ASTNode **statements, int num_statements) {
    ASTNode *node = ast_alloc_node(NODE_BLOCO);
    node->data.bloco.statements = statements;
    node->data.bloco.num_statements = num_statements;
    return node;
}

/* Criar no de declaracao */
ASTNode* ast_create_declaracao(const char *nome, DataType tipo, ASTNode *init_expr) {
    ASTNode *node = ast_alloc_node(NODE_DECLARACAO);
    node->data.declaracao.nome = strdup(nome);
    node->data.declaracao.tipo = tipo;
    node->data.declaracao.init_expr = init_expr;
    return node;
}

/* Criar no de atribuicao */
ASTNode* ast_create_atribuicao(const char *nome, ASTNode *expr) {
    ASTNode *node = ast_alloc_node(NODE_ATRIBUICAO);
    node->data.atribuicao.nome = strdup(nome);
    node->data.atribuicao.expr = expr;
    return node;
}

/* Criar no de preaquecer */
ASTNode* ast_create_preaquecer(ASTNode *temperatura) {
    ASTNode *node = ast_alloc_node(NODE_PREAQUECER);
    node->data.preaquecer.temperatura = temperatura;
    return node;
}

/* Criar no de cozinhar */
ASTNode* ast_create_cozinhar(ASTNode *temperatura, ASTNode *tempo, TimeUnit unidade) {
    ASTNode *node = ast_alloc_node(NODE_COZINHAR);
    node->data.cozinhar.temperatura = temperatura;
    node->data.cozinhar.tempo = tempo;
    node->data.cozinhar.unidade = unidade;
    return node;
}

/* Criar no de aquecer */
ASTNode* ast_create_aquecer(ASTNode *tempo, TimeUnit unidade) {
    ASTNode *node = ast_alloc_node(NODE_AQUECER);
    node->data.aquecer.tempo = tempo;
    node->data.aquecer.unidade = unidade;
    return node;
}

/* Criar no de agitar */
ASTNode* ast_create_agitar(ASTNode *tempo) {
    ASTNode *node = ast_alloc_node(NODE_AGITAR);
    node->data.agitar.tempo = tempo;
    return node;
}

/* Criar no de set_modo */
ASTNode* ast_create_set_modo(ModoKind modo) {
    ASTNode *node = ast_alloc_node(NODE_SET_MODO);
    node->data.set_modo.modo = modo;
    return node;
}

/* Criar no de pausar */
ASTNode* ast_create_pausar(void) {
    return ast_alloc_node(NODE_PAUSAR);
}

/* Criar no de continuar */
ASTNode* ast_create_continuar(void) {
    return ast_alloc_node(NODE_CONTINUAR);
}

/* Criar no de parar */
ASTNode* ast_create_parar(void) {
    return ast_alloc_node(NODE_PARAR);
}

/* Criar no de imprimir */
ASTNode* ast_create_imprimir(ASTNode **exprs, int num_exprs) {
    ASTNode *node = ast_alloc_node(NODE_IMPRIMIR);
    node->data.imprimir.exprs = exprs;
    node->data.imprimir.num_exprs = num_exprs;
    return node;
}

/* Criar no de se */
ASTNode* ast_create_se(ASTNode *condicao, ASTNode *bloco_then, ASTNode *bloco_else) {
    ASTNode *node = ast_alloc_node(NODE_SE);
    node->data.se.condicao = condicao;
    node->data.se.bloco_then = bloco_then;
    node->data.se.bloco_else = bloco_else;
    return node;
}

/* Criar no de enquanto */
ASTNode* ast_create_enquanto(ASTNode *condicao, ASTNode *bloco) {
    ASTNode *node = ast_alloc_node(NODE_ENQUANTO);
    node->data.enquanto.condicao = condicao;
    node->data.enquanto.bloco = bloco;
    return node;
}

/* Criar no de operacao binaria */
ASTNode* ast_create_binop(BinOpKind op, ASTNode *left, ASTNode *right) {
    ASTNode *node = ast_alloc_node(NODE_BINOP);
    node->data.binop.op = op;
    node->data.binop.left = left;
    node->data.binop.right = right;
    return node;
}

/* Criar no de operacao unaria */
ASTNode* ast_create_unop(UnOpKind op, ASTNode *operand) {
    ASTNode *node = ast_alloc_node(NODE_UNOP);
    node->data.unop.op = op;
    node->data.unop.operand = operand;
    return node;
}

/* Criar no de literal inteiro */
ASTNode* ast_create_literal_int(int value) {
    ASTNode *node = ast_alloc_node(NODE_LITERAL_INT);
    node->data.literal_int.value = value;
    node->data_type = TYPE_INTEIRO;
    return node;
}

/* Criar no de literal fracionario */
ASTNode* ast_create_literal_frac(double value) {
    ASTNode *node = ast_alloc_node(NODE_LITERAL_FRAC);
    node->data.literal_frac.value = value;
    node->data_type = TYPE_FRAC;
    return node;
}

/* Criar no de literal booleano */
ASTNode* ast_create_literal_bool(int value) {
    ASTNode *node = ast_alloc_node(NODE_LITERAL_BOOL);
    node->data.literal_bool.value = value;
    node->data_type = TYPE_BOOL;
    return node;
}

/* Criar no de literal string */
ASTNode* ast_create_literal_str(const char *value) {
    ASTNode *node = ast_alloc_node(NODE_LITERAL_STR);
    node->data.literal_str.value = strdup(value);
    node->data_type = TYPE_TEXTO;
    return node;
}

/* Criar no de variavel */
ASTNode* ast_create_variavel(const char *nome) {
    ASTNode *node = ast_alloc_node(NODE_VARIAVEL);
    node->data.variavel.nome = strdup(nome);
    return node;
}

/* Adicionar um statement a um bloco */
void ast_bloco_add_statement(ASTNode *bloco, ASTNode *statement) {
    if (bloco->kind != NODE_BLOCO) {
        fprintf(stderr, "Erro interno: tentativa de adicionar statement a no que nao e bloco\n");
        return;
    }
    
    int new_size = bloco->data.bloco.num_statements + 1;
    bloco->data.bloco.statements = (ASTNode**)realloc(
        bloco->data.bloco.statements,
        new_size * sizeof(ASTNode*)
    );
    bloco->data.bloco.statements[new_size - 1] = statement;
    bloco->data.bloco.num_statements = new_size;
}

/* Adicionar um item ao programa */
void ast_programa_add_item(ASTNode *programa, ASTNode *item) {
    if (programa->kind != NODE_PROGRAMA) {
        fprintf(stderr, "Erro interno: tentativa de adicionar item a no que nao e programa\n");
        return;
    }
    
    int new_size = programa->data.programa.num_items + 1;
    programa->data.programa.top_level_items = (ASTNode**)realloc(
        programa->data.programa.top_level_items,
        new_size * sizeof(ASTNode*)
    );
    programa->data.programa.top_level_items[new_size - 1] = item;
    programa->data.programa.num_items = new_size;
}

/* Adicionar uma expressao ao imprimir */
void ast_imprimir_add_expr(ASTNode *imprimir, ASTNode *expr) {
    if (imprimir->kind != NODE_IMPRIMIR) {
        fprintf(stderr, "Erro interno: tentativa de adicionar expressao a no que nao e imprimir\n");
        return;
    }
    
    int new_size = imprimir->data.imprimir.num_exprs + 1;
    imprimir->data.imprimir.exprs = (ASTNode**)realloc(
        imprimir->data.imprimir.exprs,
        new_size * sizeof(ASTNode*)
    );
    imprimir->data.imprimir.exprs[new_size - 1] = expr;
    imprimir->data.imprimir.num_exprs = new_size;
}

/* Liberar memoria da AST */
void ast_free(ASTNode *node) {
    if (!node) return;
    
    switch (node->kind) {
        case NODE_PROGRAMA:
            free(node->data.programa.nome);
            for (int i = 0; i < node->data.programa.num_items; i++) {
                ast_free(node->data.programa.top_level_items[i]);
            }
            free(node->data.programa.top_level_items);
            break;
            
        case NODE_RECEITA:
            free(node->data.receita.nome);
            ast_free(node->data.receita.bloco);
            break;
            
        case NODE_PASSO:
            free(node->data.passo.nome);
            ast_free(node->data.passo.bloco);
            break;
            
        case NODE_BLOCO:
            for (int i = 0; i < node->data.bloco.num_statements; i++) {
                ast_free(node->data.bloco.statements[i]);
            }
            free(node->data.bloco.statements);
            break;
            
        case NODE_DECLARACAO:
            free(node->data.declaracao.nome);
            ast_free(node->data.declaracao.init_expr);
            break;
            
        case NODE_ATRIBUICAO:
            free(node->data.atribuicao.nome);
            ast_free(node->data.atribuicao.expr);
            break;
            
        case NODE_PREAQUECER:
            ast_free(node->data.preaquecer.temperatura);
            break;
            
        case NODE_COZINHAR:
            ast_free(node->data.cozinhar.temperatura);
            ast_free(node->data.cozinhar.tempo);
            break;
            
        case NODE_AQUECER:
            ast_free(node->data.aquecer.tempo);
            break;
            
        case NODE_AGITAR:
            ast_free(node->data.agitar.tempo);
            break;
            
        case NODE_SET_MODO:
            /* Nada para liberar */
            break;
            
        case NODE_PAUSAR:
        case NODE_CONTINUAR:
        case NODE_PARAR:
            /* Nada para liberar */
            break;
            
        case NODE_IMPRIMIR:
            for (int i = 0; i < node->data.imprimir.num_exprs; i++) {
                ast_free(node->data.imprimir.exprs[i]);
            }
            free(node->data.imprimir.exprs);
            break;
            
        case NODE_SE:
            ast_free(node->data.se.condicao);
            ast_free(node->data.se.bloco_then);
            ast_free(node->data.se.bloco_else);
            break;
            
        case NODE_ENQUANTO:
            ast_free(node->data.enquanto.condicao);
            ast_free(node->data.enquanto.bloco);
            break;
            
        case NODE_BINOP:
            ast_free(node->data.binop.left);
            ast_free(node->data.binop.right);
            break;
            
        case NODE_UNOP:
            ast_free(node->data.unop.operand);
            break;
            
        case NODE_LITERAL_INT:
        case NODE_LITERAL_FRAC:
        case NODE_LITERAL_BOOL:
            /* Nada para liberar */
            break;
            
        case NODE_LITERAL_STR:
            free(node->data.literal_str.value);
            break;
            
        case NODE_VARIAVEL:
            free(node->data.variavel.nome);
            break;
    }
    
    free(node);
}

/* Imprimir a AST (para debug) */
void ast_print(ASTNode *node, int depth) {
    if (!node) return;
    
    /* Indentacao */
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    switch (node->kind) {
        case NODE_PROGRAMA:
            printf("PROGRAMA: %s\n", node->data.programa.nome);
            for (int i = 0; i < node->data.programa.num_items; i++) {
                ast_print(node->data.programa.top_level_items[i], depth + 1);
            }
            break;
            
        case NODE_RECEITA:
            printf("RECEITA: %s\n", node->data.receita.nome);
            ast_print(node->data.receita.bloco, depth + 1);
            break;
            
        case NODE_PASSO:
            printf("PASSO: %s\n", node->data.passo.nome);
            ast_print(node->data.passo.bloco, depth + 1);
            break;
            
        case NODE_BLOCO:
            printf("BLOCO (%d statements)\n", node->data.bloco.num_statements);
            for (int i = 0; i < node->data.bloco.num_statements; i++) {
                ast_print(node->data.bloco.statements[i], depth + 1);
            }
            break;
            
        case NODE_DECLARACAO:
            printf("DECLARACAO: %s : %s\n", 
                   node->data.declaracao.nome,
                   ast_type_name(node->data.declaracao.tipo));
            if (node->data.declaracao.init_expr) {
                ast_print(node->data.declaracao.init_expr, depth + 1);
            }
            break;
            
        case NODE_ATRIBUICAO:
            printf("ATRIBUICAO: %s =\n", node->data.atribuicao.nome);
            ast_print(node->data.atribuicao.expr, depth + 1);
            break;
            
        case NODE_PREAQUECER:
            printf("PREAQUECER\n");
            ast_print(node->data.preaquecer.temperatura, depth + 1);
            break;
            
        case NODE_COZINHAR:
            printf("COZINHAR (%s)\n", 
                   node->data.cozinhar.unidade == TIME_MINUTOS ? "minutos" : "segundos");
            ast_print(node->data.cozinhar.temperatura, depth + 1);
            ast_print(node->data.cozinhar.tempo, depth + 1);
            break;
            
        case NODE_AQUECER:
            printf("AQUECER (%s)\n",
                   node->data.aquecer.unidade == TIME_MINUTOS ? "minutos" : "segundos");
            ast_print(node->data.aquecer.tempo, depth + 1);
            break;
            
        case NODE_AGITAR:
            printf("AGITAR\n");
            ast_print(node->data.agitar.tempo, depth + 1);
            break;
            
        case NODE_SET_MODO:
            printf("MODO: ");
            switch (node->data.set_modo.modo) {
                case MODE_BATATA: printf("batata\n"); break;
                case MODE_LEGUMES: printf("legumes\n"); break;
                case MODE_NUGGETS: printf("nuggets\n"); break;
                case MODE_ESFIHAS: printf("esfihas\n"); break;
            }
            break;
            
        case NODE_PAUSAR:
            printf("PAUSAR\n");
            break;
            
        case NODE_CONTINUAR:
            printf("CONTINUAR\n");
            break;
            
        case NODE_PARAR:
            printf("PARAR\n");
            break;
            
        case NODE_IMPRIMIR:
            printf("IMPRIMIR (%d expressoes)\n", node->data.imprimir.num_exprs);
            for (int i = 0; i < node->data.imprimir.num_exprs; i++) {
                ast_print(node->data.imprimir.exprs[i], depth + 1);
            }
            break;
            
        case NODE_SE:
            printf("SE\n");
            ast_print(node->data.se.condicao, depth + 1);
            ast_print(node->data.se.bloco_then, depth + 1);
            if (node->data.se.bloco_else) {
                for (int i = 0; i < depth + 1; i++) printf("  ");
                printf("SENAO\n");
                ast_print(node->data.se.bloco_else, depth + 1);
            }
            break;
            
        case NODE_ENQUANTO:
            printf("ENQUANTO\n");
            ast_print(node->data.enquanto.condicao, depth + 1);
            ast_print(node->data.enquanto.bloco, depth + 1);
            break;
            
        case NODE_BINOP:
            printf("BINOP: %s\n", ast_binop_name(node->data.binop.op));
            ast_print(node->data.binop.left, depth + 1);
            ast_print(node->data.binop.right, depth + 1);
            break;
            
        case NODE_UNOP:
            printf("UNOP: %s\n", ast_unop_name(node->data.unop.op));
            ast_print(node->data.unop.operand, depth + 1);
            break;
            
        case NODE_LITERAL_INT:
            printf("INT: %d\n", node->data.literal_int.value);
            break;
            
        case NODE_LITERAL_FRAC:
            printf("FRAC: %.2f\n", node->data.literal_frac.value);
            break;
            
        case NODE_LITERAL_BOOL:
            printf("BOOL: %s\n", node->data.literal_bool.value ? "verdadeiro" : "falso");
            break;
            
        case NODE_LITERAL_STR:
            printf("STRING: \"%s\"\n", node->data.literal_str.value);
            break;
            
        case NODE_VARIAVEL:
            printf("VAR: %s\n", node->data.variavel.nome);
            break;
    }
}

/* Obter nome do tipo de dado como string */
const char* ast_type_name(DataType type) {
    switch (type) {
        case TYPE_INTEIRO: return "inteiro";
        case TYPE_FRAC: return "frac";
        case TYPE_BOOL: return "bool";
        case TYPE_TEXTO: return "texto";
        case TYPE_VOID: return "void";
        case TYPE_UNKNOWN: return "unknown";
        default: return "???";
    }
}

/* Obter nome do operador binario como string */
const char* ast_binop_name(BinOpKind op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        case OP_LT: return "<";
        case OP_LE: return "<=";
        case OP_GT: return ">";
        case OP_GE: return ">=";
        case OP_AND: return "e";
        case OP_OR: return "ou";
        default: return "???";
    }
}

/* Obter nome do operador unario como string */
const char* ast_unop_name(UnOpKind op) {
    switch (op) {
        case OP_NEG: return "-";
        case OP_NOT: return "nao";
        default: return "???";
    }
}
