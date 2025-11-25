 # Makefile para compilar o parser AirFryerScript

# Diretórios
SRC_DIR = src
BUILD_DIR = build

# Arquivos fonte
LEX_FILE = $(SRC_DIR)/airfryer.l
YACC_FILE = $(SRC_DIR)/airfryer.y
AST_SRC = $(SRC_DIR)/ast.c
SEMANTIC_SRC = $(SRC_DIR)/semantic.c
CODEGEN_SRC = $(SRC_DIR)/codegen.c

# Arquivos gerados
LEX_OUTPUT = $(BUILD_DIR)/lex.yy.c
YACC_OUTPUT = $(BUILD_DIR)/airfryer.tab.c
YACC_HEADER = $(BUILD_DIR)/airfryer.tab.h
AST_OBJ = $(BUILD_DIR)/ast.o
SEMANTIC_OBJ = $(BUILD_DIR)/semantic.o
CODEGEN_OBJ = $(BUILD_DIR)/codegen.o

# Executável final
TARGET = $(BUILD_DIR)/airfryer_parser

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(BUILD_DIR) -I$(SRC_DIR)
LDFLAGS = -lfl

# Regra principal
all: $(TARGET)

# Compilar o executável final
$(TARGET): $(LEX_OUTPUT) $(YACC_OUTPUT) $(AST_OBJ) $(SEMANTIC_OBJ) $(CODEGEN_OBJ)
	@echo "Compilando o parser..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Parser compilado com sucesso: $(TARGET)"

# Compilar módulos auxiliares
$(AST_OBJ): $(AST_SRC) $(SRC_DIR)/ast.h
	@echo "Compilando ast.c..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(SEMANTIC_OBJ): $(SEMANTIC_SRC) $(SRC_DIR)/semantic.h $(SRC_DIR)/ast.h
	@echo "Compilando semantic.c..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(CODEGEN_OBJ): $(CODEGEN_SRC) $(SRC_DIR)/codegen.h $(SRC_DIR)/ast.h $(SRC_DIR)/semantic.h
	@echo "Compilando codegen.c..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

# Gerar código C do Flex
$(LEX_OUTPUT): $(LEX_FILE) $(YACC_HEADER)
	@echo "Gerando código léxico com Flex..."
	@mkdir -p $(BUILD_DIR)
	flex -o $@ $<

# Gerar código C do Bison
$(YACC_OUTPUT) $(YACC_HEADER): $(YACC_FILE)
	@echo "Gerando código sintático com Bison..."
	@mkdir -p $(BUILD_DIR)
	bison -d -o $(YACC_OUTPUT) $<

# Testar com os exemplos
test: $(TARGET)
	@echo "\n=== Testando com batata.afs ==="
	$(TARGET) examples/batata.afs
	@echo "\n=== Testando with solto.afs ==="
	$(TARGET) examples/solto.afs

# Testar apenas análise léxica
test-lex: $(LEX_OUTPUT)
	@echo "Testando apenas análise léxica..."
	$(CC) $(CFLAGS) -DLEX_ONLY -o $(BUILD_DIR)/lexer $(LEX_OUTPUT) $(LDFLAGS)
	@echo "Teste léxico com batata.afs:"
	$(BUILD_DIR)/lexer < examples/batata.afs

# Limpar arquivos gerados
clean:
	@echo "Limpando arquivos gerados..."
	rm -rf $(BUILD_DIR)/*
	@echo "Limpeza concluída."

# Verificar dependências
check-deps:
	@echo "Verificando dependências..."
	@which flex > /dev/null || (echo "ERRO: flex não encontrado. Instale com: sudo apt-get install flex" && exit 1)
	@which bison > /dev/null || (echo "ERRO: bison não encontrado. Instale com: sudo apt-get install bison" && exit 1)
	@which gcc > /dev/null || (echo "ERRO: gcc não encontrado. Instale com: sudo apt-get install gcc" && exit 1)
	@echo "Todas as dependências estão disponíveis!"

# Mostrar ajuda
help:
	@echo "Comandos disponíveis:"
	@echo "  make         - Compila o parser completo"
	@echo "  make test    - Testa o parser com os exemplos"
	@echo "  make test-lex - Testa apenas o analisador léxico"
	@echo "  make clean   - Remove arquivos gerados"
	@echo "  make check-deps - Verifica se as dependências estão instaladas"
	@echo "  make help    - Mostra esta ajuda"

# Criar diretório de build se não existir
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY: all test test-lex clean check-deps help