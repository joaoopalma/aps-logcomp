/*
 * semantic.h
 * Analise semantica para AirFryerScript
 * 
 * Este modulo implementa:
 * - Tabela de simbolos para rastreamento de variaveis
 * - Checagem de tipos
 * - Verificacao de declaracao antes do uso
 * - Deteccao de redeclaracao de variaveis
 * - Inferencia de tipos em expressoes
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

/* Estrutura para uma entrada na tabela de simbolos */
typedef struct Symbol {
    char *name;           /* Nome da variavel */
    DataType type;        /* Tipo da variavel */
    int is_initialized;   /* 1 se foi inicializada, 0 caso contrario */
    int scope_level;      /* Nivel de escopo (0 = global, 1+ = local) */
} Symbol;

/* Estrutura para a tabela de simbolos */
typedef struct SymbolTable {
    Symbol *symbols;      /* Array de simbolos */
    int num_symbols;      /* Numero de simbolos */
    int capacity;         /* Capacidade do array */
    int current_scope;    /* Nivel de escopo atual */
} SymbolTable;

/* Estrutura para armazenar erros semanticos */
typedef struct SemanticError {
    char *message;        /* Mensagem de erro */
    int line;             /* Linha onde ocorreu o erro */
} SemanticError;

/* Lista de erros semanticos */
typedef struct SemanticErrorList {
    SemanticError *errors;
    int num_errors;
    int capacity;
} SemanticErrorList;

/* Criar uma nova tabela de simbolos */
SymbolTable* symtable_create(void);

/* Liberar memoria da tabela de simbolos */
void symtable_free(SymbolTable *table);

/* Entrar em um novo escopo */
void symtable_enter_scope(SymbolTable *table);

/* Sair do escopo atual (remove simbolos do escopo) */
void symtable_exit_scope(SymbolTable *table);

/* Adicionar um simbolo na tabela */
/* Retorna 1 se sucesso, 0 se ja existe no escopo atual */
int symtable_add(SymbolTable *table, const char *name, DataType type, int is_initialized);

/* Buscar um simbolo na tabela (procura em todos os escopos, de dentro para fora) */
/* Retorna o simbolo se encontrado, NULL caso contrario */
Symbol* symtable_lookup(SymbolTable *table, const char *name);

/* Marcar uma variavel como inicializada */
void symtable_mark_initialized(SymbolTable *table, const char *name);

/* Criar uma nova lista de erros */
SemanticErrorList* error_list_create(void);

/* Liberar memoria da lista de erros */
void error_list_free(SemanticErrorList *list);

/* Adicionar um erro a lista */
void error_list_add(SemanticErrorList *list, const char *message, int line);

/* Imprimir todos os erros */
void error_list_print(SemanticErrorList *list);

/* Realizar analise semantica completa na AST */
/* Retorna 1 se sucesso (sem erros), 0 se houver erros */
int semantic_analyze(ASTNode *root, SemanticErrorList *errors);

/* Funcoes auxiliares internas (podem ser usadas por codegen tambem) */

/* Verificar se dois tipos sao compativeis para operacao */
int types_compatible(DataType type1, DataType type2);

/* Obter o tipo resultante de uma operacao binaria */
DataType binop_result_type(BinOpKind op, DataType left, DataType right);

/* Obter o tipo resultante de uma operacao unaria */
DataType unop_result_type(UnOpKind op, DataType operand);

/* Verificar se um tipo pode ser usado em contexto booleano (condicoes) */
int type_is_boolean(DataType type);

/* Converter tipo para string descritivo (para mensagens de erro) */
const char* type_description(DataType type);

#endif /* SEMANTIC_H */
