# Compilador e flags da versao sequencial
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS =

# Nome do executavel final
TARGET = jogo_da_vida
TEST_TARGET = testes/teste_logica

# Arquivos-fonte e objetos
SRCS = main.c entrada.c matriz.c logica.c renderizacao.c tempo.c
OBJS = $(SRCS:.c=.o)
TEST_SRCS = testes/teste_logica.c entrada.c matriz.c logica.c

# Arquivo de entrada padrao para o alvo 'make run' (pode ser sobrescrito na linha de comando)
ARQUIVO ?= entradas/blinker.txt

.PHONY: all clean run test

# Regra padrao: compila o executavel
all: $(TARGET)

# Ligacao (linking) dos arquivos objeto para gerar o executavel
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Dependencias explicitas dos cabecalhos (.h) para recompilar caso sejam alterados
main.o: main.c entrada.h matriz.h logica.h renderizacao.h tempo.h
	$(CC) $(CFLAGS) -c main.c -o main.o

entrada.o: entrada.c entrada.h matriz.h
	$(CC) $(CFLAGS) -c entrada.c -o entrada.o

matriz.o: matriz.c matriz.h
	$(CC) $(CFLAGS) -c matriz.c -o matriz.o

logica.o: logica.c logica.h matriz.h
	$(CC) $(CFLAGS) -c logica.c -o logica.o

renderizacao.o: renderizacao.c renderizacao.h
	$(CC) $(CFLAGS) -c renderizacao.c -o renderizacao.o

tempo.o: tempo.c tempo.h
	$(CC) $(CFLAGS) -c tempo.c -o tempo.o

# Executa o programa passando um arquivo da pasta 'entradas/'
# Uso: make run ARQUIVO=entradas/meu_teste.txt
run: $(TARGET)
	./$(TARGET) $(ARQUIVO)

# Compila e executa os testes automatizados da versao sequencial
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS) entrada.h matriz.h logica.h
	$(CC) $(CFLAGS) -I. -o $(TEST_TARGET) $(TEST_SRCS) $(LDFLAGS)

# Remove os arquivos objeto (.o) e o executavel
clean:
	rm -f $(OBJS) $(TARGET) $(TEST_TARGET)
