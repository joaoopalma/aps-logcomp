# Makefile para compilar o parser AirFryerScript

# Diretórios
SRC_DIR = src
BUILD_DIR = build

# Arquivos fonte
LEX_FILE = $(SRC_DIR)/airfryer.l
YACC_FILE = $(SRC_DIR)/airfryer.y

# Arquivos gerados
LEX_OUTPUT = $(BUILD_DIR)/lex.yy.c
YACC_OUTPUT = $(BUILD_DIR)/airfryer.tab.c
YACC_HEADER = $(BUILD_DIR)/airfryer.tab.h

# Executável final
TARGET = $(BUILD_DIR)/airfryer_parser

# Compilador e flags
CC = gcc
CFLAGS = -Wall -Wextra -g -I$(BUILD_DIR)
LDFLAGS = -lfl

# Regra principal
all: $(TARGET)

# Compilar o executável final
$(TARGET): $(LEX_OUTPUT) $(YACC_OUTPUT)
	@echo "Compilando o parser..."
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Parser compilado com sucesso: $(TARGET)"

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