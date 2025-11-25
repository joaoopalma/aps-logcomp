/*
 * codegen.c
 * Implementacao da geracao de codigo assembly
 */

#include "codegen.h"
#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 16
#define MAX_LABEL_LEN 64

/* Registradores disponiveis para variaveis: R0, R1, R2, R3 */
static const char* AVAILABLE_REGS[] = {"R0", "R1", "R2", "R3"};
static const int NUM_REGS = 4;

/* Funcoes auxiliares internas */
static void codegen_node(CodeGenerator *gen, ASTNode *node);
static void codegen_expr(CodeGenerator *gen, ASTNode *node, const char *dest_reg);
static void codegen_collect_strings(CodeGenerator *gen, ASTNode *node);

/* ===== CRIACAO E LIBERACAO ===== */

CodeGenerator* codegen_create(FILE *output) {
    CodeGenerator *gen = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    gen->output = output;
    gen->label_counter = 0;
    gen->string_counter = 0;
    gen->temp_reg_counter = 0;
    
    /* Inicializar mapeamento de variaveis */
    gen->var_map = malloc(INITIAL_CAPACITY * sizeof(*gen->var_map));
    gen->num_vars = 0;
    gen->capacity = INITIAL_CAPACITY;
    
    /* Inicializar string table */
    gen->strings = malloc(INITIAL_CAPACITY * sizeof(*gen->strings));
    gen->num_strings = 0;
    gen->string_capacity = INITIAL_CAPACITY;
    
    return gen;
}

void codegen_free(CodeGenerator *gen) {
    if (!gen) return;
    
    /* Liberar mapeamento de variaveis */
    for (int i = 0; i < gen->num_vars; i++) {
        free(gen->var_map[i].var_name);
    }
    free(gen->var_map);
    
    /* Liberar string table */
    for (int i = 0; i < gen->num_strings; i++) {
        free(gen->strings[i].text);
    }
    free(gen->strings);
    
    free(gen);
}

/* ===== EMISSAO DE CODIGO ===== */

void codegen_comment(CodeGenerator *gen, const char *comment) {
    fprintf(gen->output, "; %s\n", comment);
}

void codegen_emit(CodeGenerator *gen, const char *instruction) {
    fprintf(gen->output, "    %s\n", instruction);
}

void codegen_emit1(CodeGenerator *gen, const char *instruction, const char *arg1) {
    fprintf(gen->output, "    %s %s\n", instruction, arg1);
}

void codegen_emit2(CodeGenerator *gen, const char *instruction, const char *arg1, const char *arg2) {
    fprintf(gen->output, "    %s %s %s\n", instruction, arg1, arg2);
}

void codegen_emit3(CodeGenerator *gen, const char *instruction, 
                   const char *arg1, const char *arg2, const char *arg3) {
    fprintf(gen->output, "    %s %s %s %s\n", instruction, arg1, arg2, arg3);
}

void codegen_label(CodeGenerator *gen, const char *label) {
    fprintf(gen->output, "%s:\n", label);
}

char* codegen_new_label(CodeGenerator *gen, const char *prefix) {
    char *label = (char*)malloc(MAX_LABEL_LEN);
    snprintf(label, MAX_LABEL_LEN, "%s_%d", prefix, gen->label_counter++);
    return label;
}

/* ===== STRING TABLE ===== */

int codegen_add_string(CodeGenerator *gen, const char *text) {
    /* Verificar se a string ja existe */
    for (int i = 0; i < gen->num_strings; i++) {
        if (strcmp(gen->strings[i].text, text) == 0) {
            return gen->strings[i].id;
        }
    }
    
    /* Expandir se necessario */
    if (gen->num_strings >= gen->string_capacity) {
        gen->string_capacity *= 2;
        gen->strings = realloc(gen->strings, gen->string_capacity * sizeof(*gen->strings));
    }
    
    /* Adicionar nova string */
    int id = gen->string_counter++;
    gen->strings[gen->num_strings].text = strdup(text);
    gen->strings[gen->num_strings].id = id;
    gen->num_strings++;
    
    return id;
}

void codegen_emit_string_table(CodeGenerator *gen) {
    if (gen->num_strings == 0) return;
    
    codegen_comment(gen, "String Table");
    for (int i = 0; i < gen->num_strings; i++) {
        fprintf(gen->output, "    SDEF %d \"%s\"\n", 
                gen->strings[i].id, gen->strings[i].text);
    }
    fprintf(gen->output, "\n");
}

/* ===== GERENCIAMENTO DE REGISTRADORES ===== */

char* codegen_alloc_register(CodeGenerator *gen, const char *var_name, DataType type) {
    /* Verificar se ja tem registrador alocado */
    for (int i = 0; i < gen->num_vars; i++) {
        if (strcmp(gen->var_map[i].var_name, var_name) == 0) {
            char *reg = (char*)malloc(8);
            snprintf(reg, 8, "R%d", gen->var_map[i].location);
            return reg;
        }
    }
    
    /* Alocar novo registrador */
    if (gen->num_vars >= NUM_REGS) {
        /* Todos os registradores em uso, usar pilha (simplificado) */
        /* Por enquanto vamos limitar a 4 variaveis */
        return NULL;
    }
    
    /* Expandir array se necessario */
    if (gen->num_vars >= gen->capacity) {
        gen->capacity *= 2;
        gen->var_map = realloc(gen->var_map, gen->capacity * sizeof(*gen->var_map));
    }
    
    /* Adicionar mapeamento */
    gen->var_map[gen->num_vars].var_name = strdup(var_name);
    gen->var_map[gen->num_vars].type = type;
    gen->var_map[gen->num_vars].location = gen->num_vars;
    gen->num_vars++;
    
    char *reg = (char*)malloc(8);
    snprintf(reg, 8, "R%d", gen->num_vars - 1);
    return reg;
}

char* codegen_get_var_location(CodeGenerator *gen, const char *var_name) {
    for (int i = 0; i < gen->num_vars; i++) {
        if (strcmp(gen->var_map[i].var_name, var_name) == 0) {
            char *reg = (char*)malloc(8);
            snprintf(reg, 8, "R%d", gen->var_map[i].location);
            return reg;
        }
    }
    return NULL;
}

char* codegen_temp_register(CodeGenerator *gen) {
    /* Por simplicidade, usar TIME ou POWER como temporarios */
    /* Em uma implementacao real, seria mais sofisticado */
    char *reg = (char*)malloc(8);
    snprintf(reg, 8, "TIME");
    return reg;
}

void codegen_free_temp_register(CodeGenerator *gen, const char *reg) {
    /* Nao faz nada por enquanto */
    (void)gen;
    (void)reg;
}

/* ===== GERACAO DE EXPRESSOES ===== */

/* Gerar codigo para avaliar uma expressao e colocar resultado em dest_reg */
static void codegen_expr(CodeGenerator *gen, ASTNode *node, const char *dest_reg) {
    if (!node) return;
    
    char temp_str[128];
    
    switch (node->kind) {
        case NODE_LITERAL_INT:
            /* Carregar literal inteiro */
            snprintf(temp_str, sizeof(temp_str), "%d", node->data.literal_int.value);
            codegen_emit2(gen, "SET", dest_reg, temp_str);
            break;
            
        case NODE_LITERAL_FRAC: {
            /* Converter frac para fixed-point (multiplicar por 100) */
            int fixed_value = (int)(node->data.literal_frac.value * 100);
            snprintf(temp_str, sizeof(temp_str), "%d", fixed_value);
            codegen_emit2(gen, "SET", dest_reg, temp_str);
            break;
        }
            
        case NODE_LITERAL_BOOL:
            /* Booleano: 0 ou 1 */
            snprintf(temp_str, sizeof(temp_str), "%d", node->data.literal_bool.value);
            codegen_emit2(gen, "SET", dest_reg, temp_str);
            break;
            
        case NODE_VARIAVEL: {
            /* Carregar valor da variavel */
            char *var_loc = codegen_get_var_location(gen, node->data.variavel.nome);
            if (var_loc) {
                if (strcmp(var_loc, dest_reg) != 0) {
                    /* Copiar de um registrador para outro */
                    codegen_emit2(gen, "PUSH", var_loc, "");
                    codegen_emit2(gen, "POP", dest_reg, "");
                }
                free(var_loc);
            }
            break;
        }
            
        case NODE_BINOP: {
            /* Avaliar operacao binaria */
            /* Estrategia: avaliar left em dest_reg, right em TIME (se dest_reg != TIME), operar */
            /* Escolher registrador auxiliar diferente do destino */
            const char *aux_reg = (strcmp(dest_reg, "TIME") == 0) ? "POWER" : "TIME";
            
            codegen_expr(gen, node->data.binop.left, dest_reg);
            codegen_emit1(gen, "PUSH", dest_reg);
            
            codegen_expr(gen, node->data.binop.right, aux_reg);
            codegen_emit1(gen, "POP", dest_reg);
            
            /* Determinar se e operacao com frac ou int */
            int is_frac = (node->data.binop.left->data_type == TYPE_FRAC ||
                          node->data.binop.right->data_type == TYPE_FRAC);
            
            switch (node->data.binop.op) {
                case OP_ADD:
                    if (is_frac) {
                        codegen_emit2(gen, "ADDF", dest_reg, aux_reg);
                    } else {
                        codegen_emit2(gen, "ADD", dest_reg, aux_reg);
                    }
                    break;
                case OP_SUB:
                    if (is_frac) {
                        codegen_emit2(gen, "SUBF", dest_reg, aux_reg);
                    } else {
                        codegen_emit2(gen, "SUB", dest_reg, aux_reg);
                    }
                    break;
                case OP_MUL:
                    if (is_frac) {
                        codegen_emit2(gen, "MULF", dest_reg, aux_reg);
                    } else {
                        codegen_emit2(gen, "MUL", dest_reg, aux_reg);
                    }
                    break;
                case OP_DIV:
                    if (is_frac) {
                        codegen_emit2(gen, "DIVF", dest_reg, aux_reg);
                    } else {
                        codegen_emit2(gen, "DIV", dest_reg, aux_reg);
                    }
                    break;
                case OP_MOD:
                    codegen_emit2(gen, "MOD", dest_reg, aux_reg);
                    break;
                case OP_EQ:
                    codegen_emit2(gen, "EQ", dest_reg, aux_reg);
                    break;
                case OP_NE:
                    codegen_emit2(gen, "NE", dest_reg, aux_reg);
                    break;
                case OP_LT:
                    codegen_emit2(gen, "LT", dest_reg, aux_reg);
                    break;
                case OP_LE:
                    codegen_emit2(gen, "LE", dest_reg, aux_reg);
                    break;
                case OP_GT:
                    codegen_emit2(gen, "GT", dest_reg, aux_reg);
                    break;
                case OP_GE:
                    codegen_emit2(gen, "GE", dest_reg, aux_reg);
                    break;
                case OP_AND:
                    codegen_emit2(gen, "AND", dest_reg, aux_reg);
                    break;
                case OP_OR:
                    codegen_emit2(gen, "OR", dest_reg, aux_reg);
                    break;
            }
            break;
        }
            
        case NODE_UNOP: {
            /* Avaliar operacao unaria */
            codegen_expr(gen, node->data.unop.operand, dest_reg);
            
            switch (node->data.unop.op) {
                case OP_NEG:
                    /* Negar: 0 - valor */
                    codegen_emit1(gen, "PUSH", dest_reg);
                    codegen_emit2(gen, "SET", dest_reg, "0");
                    codegen_emit1(gen, "POP", "POWER");
                    codegen_emit2(gen, "SUB", dest_reg, "POWER");
                    break;
                case OP_NOT:
                    /* NOT logico */
                    codegen_emit1(gen, "NOT", dest_reg);
                    break;
            }
            break;
        }
            
        default:
            break;
    }
}

/* ===== COLETA DE STRINGS (PRE-PROCESSAMENTO) ===== */

/* Percorrer a AST e coletar todos os literais de string */
static void codegen_collect_strings(CodeGenerator *gen, ASTNode *node) {
    if (!node) return;
    
    switch (node->kind) {
        case NODE_LITERAL_STR:
            /* Adicionar string a tabela */
            codegen_add_string(gen, node->data.literal_str.value);
            break;
            
        case NODE_PROGRAMA:
            for (int i = 0; i < node->data.programa.num_items; i++) {
                codegen_collect_strings(gen, node->data.programa.top_level_items[i]);
            }
            break;
            
        case NODE_RECEITA:
            codegen_collect_strings(gen, node->data.receita.bloco);
            break;
            
        case NODE_PASSO:
            codegen_collect_strings(gen, node->data.passo.bloco);
            break;
            
        case NODE_BLOCO:
            for (int i = 0; i < node->data.bloco.num_statements; i++) {
                codegen_collect_strings(gen, node->data.bloco.statements[i]);
            }
            break;
            
        case NODE_DECLARACAO:
            codegen_collect_strings(gen, node->data.declaracao.init_expr);
            break;
            
        case NODE_ATRIBUICAO:
            codegen_collect_strings(gen, node->data.atribuicao.expr);
            break;
            
        case NODE_PREAQUECER:
            codegen_collect_strings(gen, node->data.preaquecer.temperatura);
            break;
            
        case NODE_COZINHAR:
            codegen_collect_strings(gen, node->data.cozinhar.temperatura);
            codegen_collect_strings(gen, node->data.cozinhar.tempo);
            break;
            
        case NODE_AQUECER:
            codegen_collect_strings(gen, node->data.aquecer.tempo);
            break;
            
        case NODE_AGITAR:
            codegen_collect_strings(gen, node->data.agitar.tempo);
            break;
            
        case NODE_IMPRIMIR:
            for (int i = 0; i < node->data.imprimir.num_exprs; i++) {
                codegen_collect_strings(gen, node->data.imprimir.exprs[i]);
            }
            break;
            
        case NODE_SE:
            codegen_collect_strings(gen, node->data.se.condicao);
            codegen_collect_strings(gen, node->data.se.bloco_then);
            codegen_collect_strings(gen, node->data.se.bloco_else);
            break;
            
        case NODE_ENQUANTO:
            codegen_collect_strings(gen, node->data.enquanto.condicao);
            codegen_collect_strings(gen, node->data.enquanto.bloco);
            break;
            
        case NODE_BINOP:
            codegen_collect_strings(gen, node->data.binop.left);
            codegen_collect_strings(gen, node->data.binop.right);
            break;
            
        case NODE_UNOP:
            codegen_collect_strings(gen, node->data.unop.operand);
            break;
            
        default:
            /* Outros tipos nao tem strings */
            break;
    }
}

/* ===== GERACAO DE COMANDOS ===== */

static void codegen_node(CodeGenerator *gen, ASTNode *node) {
    if (!node) return;
    
    char temp_str[128];
    
    switch (node->kind) {
        case NODE_PROGRAMA:
            codegen_comment(gen, "===========================================");
            snprintf(temp_str, sizeof(temp_str), "Programa: %s", node->data.programa.nome);
            codegen_comment(gen, temp_str);
            codegen_comment(gen, "Compilado por AirFryerScript Compiler");
            codegen_comment(gen, "===========================================");
            fprintf(gen->output, "\n");
            
            /* Pre-processar para coletar strings (primeira passagem) */
            /* Fazemos isso percorrendo a arvore apenas para encontrar literais de string */
            codegen_collect_strings(gen, node);
            
            /* Emitir string table */
            codegen_emit_string_table(gen);
            
            /* Gerar codigo para todos os itens */
            for (int i = 0; i < node->data.programa.num_items; i++) {
                codegen_node(gen, node->data.programa.top_level_items[i]);
            }
            
            /* Adicionar HALT no final */
            codegen_emit(gen, "HALT");
            break;
            
        case NODE_RECEITA:
            codegen_comment(gen, "===== RECEITA =====");
            snprintf(temp_str, sizeof(temp_str), "Receita: %s", node->data.receita.nome);
            codegen_comment(gen, temp_str);
            codegen_node(gen, node->data.receita.bloco);
            fprintf(gen->output, "\n");
            break;
            
        case NODE_PASSO:
            snprintf(temp_str, sizeof(temp_str), "Passo: %s", node->data.passo.nome);
            codegen_comment(gen, temp_str);
            codegen_node(gen, node->data.passo.bloco);
            break;
            
        case NODE_BLOCO:
            for (int i = 0; i < node->data.bloco.num_statements; i++) {
                codegen_node(gen, node->data.bloco.statements[i]);
            }
            break;
            
        case NODE_DECLARACAO: {
            /* Alocar registrador para a variavel */
            char *reg = codegen_alloc_register(gen, node->data.declaracao.nome, 
                                              node->data.declaracao.tipo);
            if (!reg) {
                fprintf(stderr, "Erro: nao ha registradores disponiveis para '%s'\n",
                       node->data.declaracao.nome);
                return;
            }
            
            snprintf(temp_str, sizeof(temp_str), "var %s : %s", 
                    node->data.declaracao.nome,
                    ast_type_name(node->data.declaracao.tipo));
            codegen_comment(gen, temp_str);
            
            /* Se tem inicializacao, gerar codigo */
            if (node->data.declaracao.init_expr) {
                codegen_expr(gen, node->data.declaracao.init_expr, reg);
            } else {
                /* Inicializar com 0 */
                codegen_emit2(gen, "SET", reg, "0");
            }
            
            free(reg);
            break;
        }
            
        case NODE_ATRIBUICAO: {
            snprintf(temp_str, sizeof(temp_str), "%s = ...", node->data.atribuicao.nome);
            codegen_comment(gen, temp_str);
            
            char *reg = codegen_get_var_location(gen, node->data.atribuicao.nome);
            if (reg) {
                codegen_expr(gen, node->data.atribuicao.expr, reg);
                free(reg);
            }
            break;
        }
            
        case NODE_PREAQUECER:
            codegen_comment(gen, "preaquecer");
            codegen_expr(gen, node->data.preaquecer.temperatura, "POWER");
            codegen_emit2(gen, "SETMODE", "0", "");  /* Modo preaquecer */
            break;
            
        case NODE_COZINHAR: {
            codegen_comment(gen, "cozinhar");
            codegen_expr(gen, node->data.cozinhar.temperatura, "POWER");
            codegen_expr(gen, node->data.cozinhar.tempo, "TIME");
            
            /* Converter tempo se for segundos */
            if (node->data.cozinhar.unidade == TIME_SEGUNDOS) {
                codegen_comment(gen, "converter segundos para minutos");
                /* Simplificacao: assumir que TIME ja esta em unidade adequada */
            }
            
            /* Loop de cooking (simulado) */
            char *loop_label = codegen_new_label(gen, "cook_loop");
            char *end_label = codegen_new_label(gen, "cook_end");
            codegen_label(gen, loop_label);
            codegen_emit2(gen, "DECJZ", "TIME", end_label);
            codegen_emit1(gen, "GOTO", loop_label);
            codegen_label(gen, end_label);
            free(loop_label);
            free(end_label);
            break;
        }
            
        case NODE_AQUECER:
            codegen_comment(gen, "aquecer");
            codegen_expr(gen, node->data.aquecer.tempo, "TIME");
            /* Similar ao cozinhar, mas sem mudar POWER */
            break;
            
        case NODE_AGITAR:
            codegen_comment(gen, "agitar");
            /* Simplificado: apenas um marcador */
            codegen_emit(gen, "PRINT");
            break;
            
        case NODE_SET_MODO: {
            codegen_comment(gen, "modo");
            int mode_val = node->data.set_modo.modo + 1;  /* 1=batata, 2=legumes, etc */
            snprintf(temp_str, sizeof(temp_str), "%d", mode_val);
            codegen_emit2(gen, "SETMODE", temp_str, "");
            break;
        }
            
        case NODE_PAUSAR:
            codegen_comment(gen, "pausar");
            codegen_emit(gen, "PAUSE");
            break;
            
        case NODE_CONTINUAR:
            codegen_comment(gen, "continuar");
            codegen_emit(gen, "RESUME");
            break;
            
        case NODE_PARAR:
            codegen_comment(gen, "parar");
            codegen_emit(gen, "STOP");
            break;
            
        case NODE_IMPRIMIR:
            codegen_comment(gen, "imprimir");
            for (int i = 0; i < node->data.imprimir.num_exprs; i++) {
                ASTNode *expr = node->data.imprimir.exprs[i];
                
                if (expr->kind == NODE_LITERAL_STR) {
                    /* String literal: adicionar a string table e emitir SPRINT */
                    int str_id = codegen_add_string(gen, expr->data.literal_str.value);
                    snprintf(temp_str, sizeof(temp_str), "%d", str_id);
                    codegen_emit1(gen, "SPRINT", temp_str);
                } else {
                    /* Avaliar expressao e imprimir */
                    codegen_expr(gen, expr, "TIME");
                    
                    /* Escolher instrucao de print baseada no tipo */
                    if (expr->data_type == TYPE_FRAC) {
                        codegen_emit1(gen, "PRINTF", "TIME");
                    } else if (expr->data_type == TYPE_BOOL) {
                        codegen_emit1(gen, "PRINTB", "TIME");
                    } else {
                        codegen_emit1(gen, "PRINTI", "TIME");
                    }
                }
            }
            break;
            
        case NODE_SE: {
            char *else_label = codegen_new_label(gen, "else");
            char *end_label = codegen_new_label(gen, "endif");
            
            codegen_comment(gen, "se");
            
            /* Avaliar condicao em POWER (usamos como temporario) */
            codegen_expr(gen, node->data.se.condicao, "POWER");
            
            /* Se POWER == 0, pular para else/end */
            if (node->data.se.bloco_else) {
                codegen_emit2(gen, "JZ", "POWER", else_label);
            } else {
                codegen_emit2(gen, "JZ", "POWER", end_label);
            }
            
            /* Bloco then */
            codegen_node(gen, node->data.se.bloco_then);
            
            if (node->data.se.bloco_else) {
                codegen_emit1(gen, "GOTO", end_label);
                codegen_label(gen, else_label);
                codegen_comment(gen, "senao");
                codegen_node(gen, node->data.se.bloco_else);
            }
            
            codegen_label(gen, end_label);
            free(else_label);
            free(end_label);
            break;
        }
            
        case NODE_ENQUANTO: {
            char *loop_label = codegen_new_label(gen, "while");
            char *end_label = codegen_new_label(gen, "endwhile");
            
            codegen_comment(gen, "enquanto");
            codegen_label(gen, loop_label);
            
            /* Avaliar condicao em POWER (usamos como temporario) */
            codegen_expr(gen, node->data.enquanto.condicao, "POWER");
            
            /* Se POWER == 0, sair do loop */
            codegen_emit2(gen, "JZ", "POWER", end_label);
            
            /* Corpo do loop */
            codegen_node(gen, node->data.enquanto.bloco);
            
            /* Voltar ao inicio */
            codegen_emit1(gen, "GOTO", loop_label);
            
            codegen_label(gen, end_label);
            free(loop_label);
            free(end_label);
            break;
        }
            
        default:
            break;
    }
}

/* ===== FUNCAO PRINCIPAL ===== */

int codegen_generate(CodeGenerator *gen, ASTNode *root) {
    if (!gen || !root) return 0;
    
    /* Gerar codigo para a AST */
    codegen_node(gen, root);
    
    return 1;
}
