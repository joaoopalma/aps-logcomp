%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ast.h"
#include "airfryer.tab.h"

extern int yyparse();
extern void yyerror(const char *s);

int line_num = 1;
%}

%option noyywrap

/* Definições de padrões */
DIGIT       [0-9]
LETTER      [a-zA-Z]
ID_CHAR     [a-zA-Z0-9_]

%%

    /* Comentários */
"//".*                  { /* ignorar comentários de linha */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* ignorar comentários de bloco */ }

    /* Whitespace */
[ \t]+                  { /* ignorar espaços e tabs */ }
\n                      { line_num++; }

    /* Palavras-chave da linguagem */
"programa"              { return PROGRAMA; }
"receita"               { return RECEITA; }
"passo"                 { return PASSO; }

    /* Tipos */
"var"                   { return VAR; }
"inteiro"               { return INTEIRO; }
"frac"                  { return FRAC; }
"bool"                  { return BOOL; }
"texto"                 { return TEXTO; }

    /* Comandos da air fryer */
"preaquecer"            { return PREAQUECER; }
"cozinhar"              { return COZINHAR; }
"aquecer"               { return AQUECER; }
"agitar"                { return AGITAR; }
"modo"                  { return MODO; }
"temperatura"           { return TEMPERATURA; }
"graus"                 { return GRAUS; }
"celsius"               { return CELSIUS; }
"tempo"                 { return TEMPO; }
"minutos"               { return MINUTOS; }
"segundos"              { return SEGUNDOS; }
"aos"                   { return AOS; }

    /* Modos de operação */
"batata"                { return BATATA; }
"legumes"               { return LEGUMES; }
"nuggets"               { return NUGGETS; }
"esfihas"               { return ESFIHAS; }

    /* Controle de execução */
"pausar"                { return PAUSAR; }
"continuar"             { return CONTINUAR; }
"parar"                 { return PARAR; }
"imprimir"              { return IMPRIMIR; }

    /* Estruturas de controle */
"se"                    { return SE; }
"senao"                 { return SENAO; }
"enquanto"              { return ENQUANTO; }

    /* Valores booleanos */
"verdadeiro"            { return VERDADEIRO; }
"falso"                 { return FALSO; }

    /* Operadores lógicos */
"e"                     { return E; }
"ou"                    { return OU; }
"nao"                   { return NAO; }

    /* Operadores de comparação */
"=="                    { return EQ; }
"!="                    { return NE; }
"<="                    { return LE; }
">="                    { return GE; }
"<"                     { return LT; }
">"                     { return GT; }

    /* Operadores aritméticos */
"+"                     { return PLUS; }
"-"                     { return MINUS; }
"*"                     { return MULT; }
"/"                     { return DIV; }
"%"                     { return MOD; }

    /* Operador de atribuição */
"="                     { return ASSIGN; }

    /* Delimitadores */
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
";"                     { return SEMICOLON; }
":"                     { return COLON; }
","                     { return COMMA; }

    /* Literais */
{DIGIT}+                { 
                          yylval.int_val = atoi(yytext); 
                          return INT_LITERAL; 
                        }

{DIGIT}+\.{DIGIT}+      { 
                          yylval.double_val = atof(yytext); 
                          return DEC_LITERAL; 
                        }

\"([^"\\]|\\.)*\"       { 
                          /* Remove as aspas e salva a string */
                          yylval.str_val = strdup(yytext + 1);
                          yylval.str_val[strlen(yylval.str_val) - 1] = '\0';
                          return STR_LITERAL; 
                        }

{LETTER}{ID_CHAR}*      { 
                          yylval.str_val = strdup(yytext);
                          return ID; 
                        }

    /* Caracteres inválidos */
.                       { 
                          printf("Erro léxico: caractere inválido '%c' na linha %d\n", 
                                 yytext[0], line_num); 
                          return -1;
                        }

%%

/* Função para reportar erro léxico */
void lex_error(const char* msg) {
    printf("Erro léxico na linha %d: %s\n", line_num, msg);
}