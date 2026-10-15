#include "matriz.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int **alocar_matriz(int n, int m) {
    if (n <= 0 || m <= 0) {
        fprintf(stderr, "Erro: as dimensoes da matriz devem ser positivas.\n");
        return NULL;
    }

    if ((size_t)n > SIZE_MAX / sizeof(int *) || (size_t)m > SIZE_MAX / sizeof(int)) {
        fprintf(stderr, "Erro: dimensao da matriz excede o limite de alocacao.\n");
        return NULL;
    }

    int **matriz = malloc((size_t)n * sizeof(int *));
    if (matriz == NULL) {
        perror("Erro ao alocar vetor de linhas da matriz");
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        matriz[i] = calloc((size_t)m, sizeof(int));
        if (matriz[i] == NULL) {
            perror("Erro ao alocar colunas da matriz");
            for (int k = 0; k < i; k++) {
                free(matriz[k]);
            }
            free(matriz);
            return NULL;
        }
    }

    return matriz;
}

void liberar_matriz(int **matriz, int n) {
    if (matriz == NULL) return;
    for (int i = 0; i < n; i++) {
        free(matriz[i]);
    }
    free(matriz);
}

void trocar_matrizes(int ***atual, int ***proxima) {
    int **temp = *atual;
    *atual = *proxima;
    *proxima = temp;
}

int matrizes_iguais(int **primeira, int **segunda, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (primeira[i][j] != segunda[i][j]) {
                return 0;
            }
        }
    }

    return 1;
}

void imprimir_matriz(int **matriz, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }
}
