#include "matriz.h"
#include <stdio.h>
#include <stdlib.h>

int **alocar_matriz(int n, int m) {
    int **matriz = (int **)malloc(n * sizeof(int *));
    if (matriz == NULL) {
        perror("Erro ao alocar vetor de linhas da matriz");
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        matriz[i] = (int *)calloc(m, sizeof(int));
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

void imprimir_matriz(int **matriz, int n, int m) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d ", matriz[i][j]);
        }
        printf("\n");
    }
}
