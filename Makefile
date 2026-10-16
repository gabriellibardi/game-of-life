# Compilador e flags comuns as duas versoes
CC = gcc
CPPFLAGS = -I.
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
DEPFLAGS = -MMD -MP
THREAD_FLAGS = -pthread
PYTHON ?= python3
LDFLAGS =
DEBUG_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O0 -g3 -fsanitize=address,undefined
DEBUG_LDFLAGS = -fsanitize=address,undefined

# Nomes dos executaveis
TARGET = jogo_da_vida
DEBUG_TARGET = jogo_da_vida_debug
TEST_TARGET = testes/teste_logica

# Arquivos-fonte, cabecalhos, objetos e dependencias geradas automaticamente
SRCS = main.c entrada.c matriz.c logica.c paralelo.c renderizacao.c tempo.c
HEADERS = entrada.h matriz.h logica.h paralelo.h renderizacao.h tempo.h
OBJS = $(SRCS:.c=.o)
DEPS = $(OBJS:.o=.d)
TEST_SRCS = testes/teste_logica.c entrada.c matriz.c logica.c

# Arquivo de entrada padrao para o alvo 'make run' (pode ser sobrescrito na linha de comando)
ARQUIVO ?= entradas/blinker.txt
ARGS ?=

.PHONY: all clean debug run test test-sequencial benchmark

# Regra padrao: compila o executavel
all: $(TARGET)

# Ligacao (linking) dos arquivos objeto para gerar o executavel
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $(THREAD_FLAGS) -o $(TARGET) $(OBJS)

# Regra generica de compilacao, incluindo arquivos .d para dependencias de cabecalhos
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THREAD_FLAGS) $(DEPFLAGS) -c $< -o $@

# Gera um executavel separado com simbolos de depuracao e sanitizadores
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(SRCS) $(HEADERS)
	$(CC) $(CPPFLAGS) $(DEBUG_CFLAGS) $(THREAD_FLAGS) -o $(DEBUG_TARGET) $(SRCS) $(DEBUG_LDFLAGS)

# Executa o programa passando um arquivo da pasta 'entradas/'
# Uso: make run ARQUIVO=entradas/meu_teste.txt ARGS="--sem-impressao"
run: $(TARGET)
	./$(TARGET) $(ARQUIVO) $(ARGS)

# Executa a suite sequencial.
test: test-sequencial

test-sequencial: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS) entrada.h matriz.h logica.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $(TEST_TARGET) $(TEST_SRCS) $(LDFLAGS)

# Parametros podem ser passados em BENCH_ARGS; veja --help do script.
benchmark: $(TARGET)
	$(PYTHON) scripts/benchmark.py --executavel ./$(TARGET) $(BENCH_ARGS)

# Remove os objetos, dependencias e executaveis gerados
clean:
	rm -f $(OBJS) $(DEPS) $(TARGET) $(DEBUG_TARGET) $(TEST_TARGET)

-include $(DEPS)
