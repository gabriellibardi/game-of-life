# Compilador e flags da versao sequencial
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2
LDFLAGS =

# Nome do executavel final
TARGET = jogo_da_vida

# Arquivos-fonte e objetos
SRCS = main.c entrada.c matriz.c logica.c
OBJS = $(SRCS:.c=.o)

# Arquivo de entrada padrao para o alvo 'make run' (pode ser sobrescrito na linha de comando)
ARQUIVO ?= entradas/blinker.txt

.PHONY: all clean run

# Regra padrao: compila o executavel
all: $(TARGET)

# Ligacao (linking) dos arquivos objeto para gerar o executavel
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)

# Dependencias explicitas dos cabecalhos (.h) para recompilar caso sejam alterados
main.o: main.c entrada.h matriz.h logica.h
	$(CC) $(CFLAGS) -c main.c -o main.o

entrada.o: entrada.c entrada.h matriz.h
	$(CC) $(CFLAGS) -c entrada.c -o entrada.o

matriz.o: matriz.c matriz.h
	$(CC) $(CFLAGS) -c matriz.c -o matriz.o

logica.o: logica.c logica.h matriz.h
	$(CC) $(CFLAGS) -c logica.c -o logica.o

# Executa o programa passando um arquivo da pasta 'entradas/'
# Uso: make run ARQUIVO=entradas/meu_teste.txt
run: $(TARGET)
	./$(TARGET) $(ARQUIVO)

# Remove os arquivos objeto (.o) e o executavel
clean:
	rm -f $(OBJS) $(TARGET)
