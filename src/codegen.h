/*
 * codegen.h
 * Geracao de codigo assembly para AirFryerVM
 * 
 * Este modulo percorre a AST e gera codigo assembly compativel
 * com a AirFryerVM (extensao da MicrowaveVM).
 */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

/* Estrutura para gerenciar a geracao de codigo */
typedef struct CodeGenerator {
    FILE *output;              /* Arquivo de saida */
    int label_counter;         /* Contador para gerar labels unicos */
    int string_counter;        /* Contador para strings na string table */
    int temp_reg_counter;      /* Contador para registradores temporarios */
    
    /* Mapeamento de variaveis para registradores/memoria */
    struct {
        char *var_name;
        DataType type;
        int location;          /* 0-3 = R0-R3, negativo = stack offset */
    } *var_map;
    int num_vars;
    int capacity;
    
    /* String table (para literais de texto) */
    struct {
        char *text;
        int id;
    } *strings;
    int num_strings;
    int string_capacity;
} CodeGenerator;

/* Criar um novo gerador de codigo */
CodeGenerator* codegen_create(FILE *output);

/* Liberar memoria do gerador */
void codegen_free(CodeGenerator *gen);

/* Gerar codigo para a AST completa */
/* Retorna 1 se sucesso, 0 se erro */
int codegen_generate(CodeGenerator *gen, ASTNode *root);

/* Funcoes auxiliares para emitir codigo assembly */

/* Emitir um comentario */
void codegen_comment(CodeGenerator *gen, const char *comment);

/* Emitir uma instrucao sem argumentos */
void codegen_emit(CodeGenerator *gen, const char *instruction);

/* Emitir uma instrucao com um argumento */
void codegen_emit1(CodeGenerator *gen, const char *instruction, const char *arg1);

/* Emitir uma instrucao com dois argumentos */
void codegen_emit2(CodeGenerator *gen, const char *instruction, const char *arg1, const char *arg2);

/* Emitir uma instrucao com tres argumentos */
void codegen_emit3(CodeGenerator *gen, const char *instruction, 
                   const char *arg1, const char *arg2, const char *arg3);

/* Emitir um label */
void codegen_label(CodeGenerator *gen, const char *label);

/* Gerar um novo label unico */
char* codegen_new_label(CodeGenerator *gen, const char *prefix);

/* Adicionar uma string a string table e retornar seu ID */
int codegen_add_string(CodeGenerator *gen, const char *text);

/* Emitir a string table no inicio do arquivo */
void codegen_emit_string_table(CodeGenerator *gen);

/* Alocar um registrador para uma variavel */
/* Retorna o nome do registrador (R0-R3) ou NULL se nao houver disponivel */
char* codegen_alloc_register(CodeGenerator *gen, const char *var_name, DataType type);

/* Obter o local (registrador/offset) de uma variavel */
/* Retorna string com "R0", "R1", etc ou NULL se nao encontrado */
char* codegen_get_var_location(CodeGenerator *gen, const char *var_name);

/* Obter um registrador temporario livre */
char* codegen_temp_register(CodeGenerator *gen);

/* Liberar um registrador temporario */
void codegen_free_temp_register(CodeGenerator *gen, const char *reg);

#endif /* CODEGEN_H */
