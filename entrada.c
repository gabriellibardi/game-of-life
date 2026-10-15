#include "entrada.h"
#include "matriz.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANHO_BUFFER_LINHA 256

/*
    Verifica se uma linha contem apenas caracteres de espaco em branco.
*/
static int linha_esta_vazia(const char *linha) {
    while (*linha != '\0') {
        if (!isspace((unsigned char)*linha)) {
            return 0;
        }
        linha++;
    }

    return 1;
}

/*
    Converte uma linha em uma quantidade exata de valores inteiros. A conversao
    falha se houver valores ausentes, fora do intervalo de int ou texto extra.
*/
static int ler_inteiros_da_linha(const char *linha, int *valores, size_t quantidade) {
    const char *cursor = linha;

    for (size_t indice = 0; indice < quantidade; indice++) {
        while (isspace((unsigned char)*cursor)) {
            cursor++;
        }

        errno = 0;
        char *fim_conversao;
        long valor = strtol(cursor, &fim_conversao, 10);

        if (cursor == fim_conversao || errno == ERANGE || valor < INT_MIN || valor > INT_MAX) {
            return 0;
        }

        valores[indice] = (int)valor;
        cursor = fim_conversao;
    }

    while (isspace((unsigned char)*cursor)) {
        cursor++;
    }

    return *cursor == '\0';
}

/*
    Informa se a linha lida coube por inteiro no buffer. A ultima linha do
    arquivo pode terminar sem uma quebra de linha.
*/
static int linha_foi_lida_por_inteiro(const char *linha, FILE *arquivo) {
    return strchr(linha, '\n') != NULL || feof(arquivo);
}

int **ler_arquivo_entrada(const char *caminho_arquivo, int *n, int *m, int *geracoes) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        return NULL;
    }

    char buffer_linha[TAMANHO_BUFFER_LINHA];
    size_t numero_linha = 1;
    int cabecalho[3];

    // Le a primeira linha: dimensoes n (linhas), m (colunas) e numero de geracoes
    if (fgets(buffer_linha, sizeof(buffer_linha), arquivo) == NULL) {
        fprintf(stderr, "Erro: o arquivo de entrada esta vazio.\n");
        fclose(arquivo);
        return NULL;
    }

    if (!linha_foi_lida_por_inteiro(buffer_linha, arquivo)) {
        fprintf(stderr, "Erro: linha 1 excede o limite de %d caracteres.\n", TAMANHO_BUFFER_LINHA - 1);
        fclose(arquivo);
        return NULL;
    }

    if (!ler_inteiros_da_linha(buffer_linha, cabecalho, 3)) {
        fprintf(stderr, "Erro: formato invalido na linha 1 (esperado: n m geracoes).\n");
        fclose(arquivo);
        return NULL;
    }

    *n = cabecalho[0];
    *m = cabecalho[1];
    *geracoes = cabecalho[2];

    if (*n <= 0 || *m <= 0 || *geracoes < 0) {
        fprintf(stderr, "Erro: dimensoes (n, m) devem ser positivas e geracoes >= 0.\n");
        fclose(arquivo);
        return NULL;
    }

    // Aloca a matriz da geracao atual inicializada com zeros
    int **matriz_t0 = alocar_matriz(*n, *m);
    if (matriz_t0 == NULL) {
        fclose(arquivo);
        return NULL;
    }

    // Le as demais linhas com as coordenadas (i, j) e preenche o estado inicial (t0)
    while (fgets(buffer_linha, sizeof(buffer_linha), arquivo) != NULL) {
        numero_linha++;

        if (!linha_foi_lida_por_inteiro(buffer_linha, arquivo)) {
            fprintf(stderr, "Erro: linha %zu excede o limite de %d caracteres.\n",
                    numero_linha, TAMANHO_BUFFER_LINHA - 1);
            liberar_matriz(matriz_t0, *n);
            fclose(arquivo);
            return NULL;
        }

        if (linha_esta_vazia(buffer_linha)) {
            continue;
        }

        int coordenadas[2];
        if (!ler_inteiros_da_linha(buffer_linha, coordenadas, 2)) {
            fprintf(stderr, "Erro: formato invalido na linha %zu (esperado: linha coluna).\n", numero_linha);
            liberar_matriz(matriz_t0, *n);
            fclose(arquivo);
            return NULL;
        }

        int linha = coordenadas[0];
        int coluna = coordenadas[1];

        if (linha >= 0 && linha < *n && coluna >= 0 && coluna < *m) {
            matriz_t0[linha][coluna] = 1;
        } else {
            fprintf(stderr, "Aviso: posicao (%d, %d) na linha %zu esta fora dos limites "
                    "[0..%d, 0..%d] e foi ignorada.\n",
                    linha, coluna, numero_linha, *n - 1, *m - 1);
        }
    }

    fclose(arquivo);
    return matriz_t0;
}
