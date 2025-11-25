/*
 * semantic.c
 * Implementacao da analise semantica
 */

#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Tamanho inicial dos arrays dinamicos */
#define INITIAL_CAPACITY 16

/* ===== TABELA DE SIMBOLOS ===== */

/* Criar uma nova tabela de simbolos */
SymbolTable* symtable_create(void) {
    SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->symbols = (Symbol*)malloc(INITIAL_CAPACITY * sizeof(Symbol));
    table->num_symbols = 0;
    table->capacity = INITIAL_CAPACITY;
    table->current_scope = 0;
    return table;
}

/* Liberar memoria da tabela de simbolos */
void symtable_free(SymbolTable *table) {
    if (!table) return;
    
    for (int i = 0; i < table->num_symbols; i++) {
        free(table->symbols[i].name);
    }
    free(table->symbols);
    free(table);
}

/* Entrar em um novo escopo */
void symtable_enter_scope(SymbolTable *table) {
    table->current_scope++;
}

/* Sair do escopo atual */
void symtable_exit_scope(SymbolTable *table) {
    /* Remove todos os simbolos do escopo atual */
    int i = 0;
    while (i < table->num_symbols) {
        if (table->symbols[i].scope_level == table->current_scope) {
            /* Remove este simbolo */
            free(table->symbols[i].name);
            /* Move os simbolos seguintes para tras */
            for (int j = i; j < table->num_symbols - 1; j++) {
                table->symbols[j] = table->symbols[j + 1];
            }
            table->num_symbols--;
        } else {
            i++;
        }
    }
    
    table->current_scope--;
}

/* Adicionar um simbolo na tabela */
int symtable_add(SymbolTable *table, const char *name, DataType type, int is_initialized) {
    /* Verificar se ja existe no escopo atual */
    for (int i = 0; i < table->num_symbols; i++) {
        if (table->symbols[i].scope_level == table->current_scope &&
            strcmp(table->symbols[i].name, name) == 0) {
            return 0;  /* Ja existe */
        }
    }
    
    /* Expandir array se necessario */
    if (table->num_symbols >= table->capacity) {
        table->capacity *= 2;
        table->symbols = (Symbol*)realloc(table->symbols, 
                                         table->capacity * sizeof(Symbol));
    }
    
    /* Adicionar novo simbolo */
    Symbol *sym = &table->symbols[table->num_symbols];
    sym->name = strdup(name);
    sym->type = type;
    sym->is_initialized = is_initialized;
    sym->scope_level = table->current_scope;
    table->num_symbols++;
    
    return 1;  /* Sucesso */
}

/* Buscar um simbolo na tabela */
Symbol* symtable_lookup(SymbolTable *table, const char *name) {
    /* Procura de tras para frente para encontrar a declaracao mais recente */
    for (int i = table->num_symbols - 1; i >= 0; i--) {
        if (strcmp(table->symbols[i].name, name) == 0) {
            return &table->symbols[i];
        }
    }
    return NULL;  /* Nao encontrado */
}

/* Marcar uma variavel como inicializada */
void symtable_mark_initialized(SymbolTable *table, const char *name) {
    Symbol *sym = symtable_lookup(table, name);
    if (sym) {
        sym->is_initialized = 1;
    }
}

/* ===== LISTA DE ERROS ===== */

/* Criar uma nova lista de erros */
SemanticErrorList* error_list_create(void) {
    SemanticErrorList *list = (SemanticErrorList*)malloc(sizeof(SemanticErrorList));
    list->errors = (SemanticError*)malloc(INITIAL_CAPACITY * sizeof(SemanticError));
    list->num_errors = 0;
    list->capacity = INITIAL_CAPACITY;
    return list;
}

/* Liberar memoria da lista de erros */
void error_list_free(SemanticErrorList *list) {
    if (!list) return;
    
    for (int i = 0; i < list->num_errors; i++) {
        free(list->errors[i].message);
    }
    free(list->errors);
    free(list);
}

/* Adicionar um erro a lista */
void error_list_add(SemanticErrorList *list, const char *message, int line) {
    /* Expandir array se necessario */
    if (list->num_errors >= list->capacity) {
        list->capacity *= 2;
        list->errors = (SemanticError*)realloc(list->errors,
                                              list->capacity * sizeof(SemanticError));
    }
    
    SemanticError *err = &list->errors[list->num_errors];
    err->message = strdup(message);
    err->line = line;
    list->num_errors++;
}

/* Imprimir todos os erros */
void error_list_print(SemanticErrorList *list) {
    if (list->num_errors == 0) {
        printf("Analise semantica: nenhum erro encontrado.\n");
        return;
    }
    
    printf("Erros semanticos encontrados:\n");
    for (int i = 0; i < list->num_errors; i++) {
        printf("  Linha %d: %s\n", list->errors[i].line, list->errors[i].message);
    }
}

/* ===== VERIFICACAO DE TIPOS ===== */

/* Verificar se dois tipos sao compativeis */
int types_compatible(DataType type1, DataType type2) {
    if (type1 == type2) return 1;
    
    /* inteiro e frac sao compativeis (com conversao implicita) */
    if ((type1 == TYPE_INTEIRO && type2 == TYPE_FRAC) ||
        (type1 == TYPE_FRAC && type2 == TYPE_INTEIRO)) {
        return 1;
    }
    
    return 0;
}

/* Obter o tipo resultante de uma operacao binaria */
DataType binop_result_type(BinOpKind op, DataType left, DataType right) {
    switch (op) {
        case OP_ADD:
        case OP_SUB:
        case OP_MUL:
        case OP_DIV:
        case OP_MOD:
            /* Operacoes aritmeticas */
            if (left == TYPE_FRAC || right == TYPE_FRAC) {
                return TYPE_FRAC;  /* Se algum e frac, resultado e frac */
            }
            if (left == TYPE_INTEIRO && right == TYPE_INTEIRO) {
                return TYPE_INTEIRO;
            }
            return TYPE_UNKNOWN;  /* Erro de tipo */
            
        case OP_EQ:
        case OP_NE:
        case OP_LT:
        case OP_LE:
        case OP_GT:
        case OP_GE:
            /* Operacoes de comparacao: resultado e sempre bool */
            if (types_compatible(left, right)) {
                return TYPE_BOOL;
            }
            return TYPE_UNKNOWN;
            
        case OP_AND:
        case OP_OR:
            /* Operacoes logicas: ambos devem ser bool */
            if (left == TYPE_BOOL && right == TYPE_BOOL) {
                return TYPE_BOOL;
            }
            return TYPE_UNKNOWN;
            
        default:
            return TYPE_UNKNOWN;
    }
}

/* Obter o tipo resultante de uma operacao unaria */
DataType unop_result_type(UnOpKind op, DataType operand) {
    switch (op) {
        case OP_NEG:
            /* Negacao aritmetica: preserva o tipo */
            if (operand == TYPE_INTEIRO || operand == TYPE_FRAC) {
                return operand;
            }
            return TYPE_UNKNOWN;
            
        case OP_NOT:
            /* Negacao logica: deve ser bool */
            if (operand == TYPE_BOOL) {
                return TYPE_BOOL;
            }
            return TYPE_UNKNOWN;
            
        default:
            return TYPE_UNKNOWN;
    }
}

/* Verificar se um tipo pode ser usado em contexto booleano */
int type_is_boolean(DataType type) {
    return type == TYPE_BOOL;
}

/* Converter tipo para string descritivo */
const char* type_description(DataType type) {
    return ast_type_name(type);
}

/* ===== ANALISE SEMANTICA ===== */

/* Funcoes auxiliares para analise */
static void analyze_node(ASTNode *node, SymbolTable *table, SemanticErrorList *errors);
static void analyze_expr(ASTNode *node, SymbolTable *table, SemanticErrorList *errors);

/* Analisar uma expressao e determinar seu tipo */
static void analyze_expr(ASTNode *node, SymbolTable *table, SemanticErrorList *errors) {
    if (!node) return;
    
    char error_msg[256];
    
    switch (node->kind) {
        case NODE_LITERAL_INT:
        case NODE_LITERAL_FRAC:
        case NODE_LITERAL_BOOL:
        case NODE_LITERAL_STR:
            /* Literais ja tem tipo definido */
            break;
            
        case NODE_VARIAVEL: {
            /* Verificar se a variavel foi declarada */
            Symbol *sym = symtable_lookup(table, node->data.variavel.nome);
            if (!sym) {
                snprintf(error_msg, sizeof(error_msg),
                        "Variavel '%s' nao declarada", node->data.variavel.nome);
                error_list_add(errors, error_msg, node->line);
                node->data_type = TYPE_UNKNOWN;
            } else {
                node->data_type = sym->type;
            }
            break;
        }
            
        case NODE_BINOP: {
            /* Analisar operandos */
            analyze_expr(node->data.binop.left, table, errors);
            analyze_expr(node->data.binop.right, table, errors);
            
            DataType left_type = node->data.binop.left->data_type;
            DataType right_type = node->data.binop.right->data_type;
            
            /* Determinar tipo do resultado */
            DataType result_type = binop_result_type(node->data.binop.op, left_type, right_type);
            
            if (result_type == TYPE_UNKNOWN) {
                snprintf(error_msg, sizeof(error_msg),
                        "Operacao '%s' invalida para tipos '%s' e '%s'",
                        ast_binop_name(node->data.binop.op),
                        type_description(left_type),
                        type_description(right_type));
                error_list_add(errors, error_msg, node->line);
            }
            
            node->data_type = result_type;
            break;
        }
            
        case NODE_UNOP: {
            /* Analisar operando */
            analyze_expr(node->data.unop.operand, table, errors);
            
            DataType operand_type = node->data.unop.operand->data_type;
            DataType result_type = unop_result_type(node->data.unop.op, operand_type);
            
            if (result_type == TYPE_UNKNOWN) {
                snprintf(error_msg, sizeof(error_msg),
                        "Operacao '%s' invalida para tipo '%s'",
                        ast_unop_name(node->data.unop.op),
                        type_description(operand_type));
                error_list_add(errors, error_msg, node->line);
            }
            
            node->data_type = result_type;
            break;
        }
            
        default:
            /* Nao e uma expressao */
            break;
    }
}

/* Analisar um no da AST */
static void analyze_node(ASTNode *node, SymbolTable *table, SemanticErrorList *errors) {
    if (!node) return;
    
    char error_msg[256];
    
    switch (node->kind) {
        case NODE_PROGRAMA:
            /* Analisar todos os itens do programa */
            for (int i = 0; i < node->data.programa.num_items; i++) {
                analyze_node(node->data.programa.top_level_items[i], table, errors);
            }
            break;
            
        case NODE_RECEITA:
            /* Entrar em novo escopo para a receita */
            symtable_enter_scope(table);
            analyze_node(node->data.receita.bloco, table, errors);
            symtable_exit_scope(table);
            break;
            
        case NODE_PASSO:
            /* Entrar em novo escopo para o passo */
            symtable_enter_scope(table);
            analyze_node(node->data.passo.bloco, table, errors);
            symtable_exit_scope(table);
            break;
            
        case NODE_BLOCO:
            /* Analisar todos os statements do bloco */
            for (int i = 0; i < node->data.bloco.num_statements; i++) {
                analyze_node(node->data.bloco.statements[i], table, errors);
            }
            break;
            
        case NODE_DECLARACAO: {
            /* Verificar se a variavel ja foi declarada no escopo atual */
            if (!symtable_add(table, node->data.declaracao.nome, 
                            node->data.declaracao.tipo,
                            node->data.declaracao.init_expr != NULL)) {
                snprintf(error_msg, sizeof(error_msg),
                        "Variavel '%s' ja foi declarada neste escopo",
                        node->data.declaracao.nome);
                error_list_add(errors, error_msg, node->line);
            }
            
            /* Se tem inicializacao, analisar a expressao */
            if (node->data.declaracao.init_expr) {
                analyze_expr(node->data.declaracao.init_expr, table, errors);
                
                /* Verificar compatibilidade de tipos */
                DataType expr_type = node->data.declaracao.init_expr->data_type;
                if (!types_compatible(node->data.declaracao.tipo, expr_type)) {
                    snprintf(error_msg, sizeof(error_msg),
                            "Tipo incompativel na inicializacao: esperado '%s', obtido '%s'",
                            type_description(node->data.declaracao.tipo),
                            type_description(expr_type));
                    error_list_add(errors, error_msg, node->line);
                }
            }
            break;
        }
            
        case NODE_ATRIBUICAO: {
            /* Verificar se a variavel foi declarada */
            Symbol *sym = symtable_lookup(table, node->data.atribuicao.nome);
            if (!sym) {
                snprintf(error_msg, sizeof(error_msg),
                        "Variavel '%s' nao declarada", node->data.atribuicao.nome);
                error_list_add(errors, error_msg, node->line);
            } else {
                /* Analisar expressao */
                analyze_expr(node->data.atribuicao.expr, table, errors);
                
                /* Verificar compatibilidade de tipos */
                DataType expr_type = node->data.atribuicao.expr->data_type;
                if (!types_compatible(sym->type, expr_type)) {
                    snprintf(error_msg, sizeof(error_msg),
                            "Tipo incompativel na atribuicao: esperado '%s', obtido '%s'",
                            type_description(sym->type),
                            type_description(expr_type));
                    error_list_add(errors, error_msg, node->line);
                }
                
                /* Marcar variavel como inicializada */
                symtable_mark_initialized(table, node->data.atribuicao.nome);
            }
            break;
        }
            
        case NODE_PREAQUECER:
            analyze_expr(node->data.preaquecer.temperatura, table, errors);
            /* Temperatura deve ser inteiro */
            if (node->data.preaquecer.temperatura->data_type != TYPE_INTEIRO &&
                node->data.preaquecer.temperatura->data_type != TYPE_FRAC) {
                snprintf(error_msg, sizeof(error_msg),
                        "Temperatura deve ser do tipo inteiro ou frac");
                error_list_add(errors, error_msg, node->line);
            }
            break;
            
        case NODE_COZINHAR:
            analyze_expr(node->data.cozinhar.temperatura, table, errors);
            analyze_expr(node->data.cozinhar.tempo, table, errors);
            /* Verificar tipos */
            if (node->data.cozinhar.temperatura->data_type != TYPE_INTEIRO &&
                node->data.cozinhar.temperatura->data_type != TYPE_FRAC) {
                snprintf(error_msg, sizeof(error_msg),
                        "Temperatura deve ser do tipo inteiro ou frac");
                error_list_add(errors, error_msg, node->line);
            }
            if (node->data.cozinhar.tempo->data_type != TYPE_INTEIRO &&
                node->data.cozinhar.tempo->data_type != TYPE_FRAC) {
                snprintf(error_msg, sizeof(error_msg),
                        "Tempo deve ser do tipo inteiro ou frac");
                error_list_add(errors, error_msg, node->line);
            }
            break;
            
        case NODE_AQUECER:
            analyze_expr(node->data.aquecer.tempo, table, errors);
            if (node->data.aquecer.tempo->data_type != TYPE_INTEIRO &&
                node->data.aquecer.tempo->data_type != TYPE_FRAC) {
                snprintf(error_msg, sizeof(error_msg),
                        "Tempo deve ser do tipo inteiro ou frac");
                error_list_add(errors, error_msg, node->line);
            }
            break;
            
        case NODE_AGITAR:
            analyze_expr(node->data.agitar.tempo, table, errors);
            if (node->data.agitar.tempo->data_type != TYPE_INTEIRO &&
                node->data.agitar.tempo->data_type != TYPE_FRAC) {
                snprintf(error_msg, sizeof(error_msg),
                        "Tempo deve ser do tipo inteiro ou frac");
                error_list_add(errors, error_msg, node->line);
            }
            break;
            
        case NODE_SET_MODO:
        case NODE_PAUSAR:
        case NODE_CONTINUAR:
        case NODE_PARAR:
            /* Nada a verificar */
            break;
            
        case NODE_IMPRIMIR:
            /* Analisar todas as expressoes */
            for (int i = 0; i < node->data.imprimir.num_exprs; i++) {
                analyze_expr(node->data.imprimir.exprs[i], table, errors);
            }
            break;
            
        case NODE_SE:
            /* Analisar condicao */
            analyze_expr(node->data.se.condicao, table, errors);
            
            /* Condicao deve ser booleana */
            if (!type_is_boolean(node->data.se.condicao->data_type)) {
                snprintf(error_msg, sizeof(error_msg),
                        "Condicao do 'se' deve ser do tipo bool, obtido '%s'",
                        type_description(node->data.se.condicao->data_type));
                error_list_add(errors, error_msg, node->line);
            }
            
            /* Analisar blocos */
            symtable_enter_scope(table);
            analyze_node(node->data.se.bloco_then, table, errors);
            symtable_exit_scope(table);
            
            if (node->data.se.bloco_else) {
                symtable_enter_scope(table);
                analyze_node(node->data.se.bloco_else, table, errors);
                symtable_exit_scope(table);
            }
            break;
            
        case NODE_ENQUANTO:
            /* Analisar condicao */
            analyze_expr(node->data.enquanto.condicao, table, errors);
            
            /* Condicao deve ser booleana */
            if (!type_is_boolean(node->data.enquanto.condicao->data_type)) {
                snprintf(error_msg, sizeof(error_msg),
                        "Condicao do 'enquanto' deve ser do tipo bool, obtido '%s'",
                        type_description(node->data.enquanto.condicao->data_type));
                error_list_add(errors, error_msg, node->line);
            }
            
            /* Analisar bloco */
            symtable_enter_scope(table);
            analyze_node(node->data.enquanto.bloco, table, errors);
            symtable_exit_scope(table);
            break;
            
        default:
            /* Outros tipos de no (expressoes) */
            analyze_expr(node, table, errors);
            break;
    }
}

/* Realizar analise semantica completa */
int semantic_analyze(ASTNode *root, SemanticErrorList *errors) {
    if (!root || !errors) return 0;
    
    SymbolTable *table = symtable_create();
    
    /* Analisar a AST */
    analyze_node(root, table, errors);
    
    /* Liberar tabela de simbolos */
    symtable_free(table);
    
    /* Retornar 1 se nao houver erros, 0 caso contrario */
    return errors->num_errors == 0;
}
