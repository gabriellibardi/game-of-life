#include "entrada.h"
#include "matriz.h"
#include <stdio.h>

int **ler_arquivo_entrada(const char *caminho_arquivo, int *n, int *m, int *geracoes) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    if (arquivo == NULL) {
        perror("Erro ao abrir o arquivo de entrada");
        return NULL;
    }

    char buffer_linha[256];

    // 1. Le a primeira linha: dimensoes n (linhas), m (colunas) e numero de geracoes
    if (fgets(buffer_linha, sizeof(buffer_linha), arquivo) == NULL || sscanf(buffer_linha, "%d %d %d", n, m, geracoes) != 3) {
        fprintf(stderr, "Erro: formato invalido na primeira linha (esperado: n m geracoes).\n");
        fclose(arquivo);
        return NULL;
    }

    if (*n <= 0 || *m <= 0 || *geracoes < 0) {
        fprintf(stderr, "Erro: dimensoes (n, m) devem ser positivas e geracoes >= 0.\n");
        fclose(arquivo);
        return NULL;
    }

    // 2. Consome a segunda linha (linha vazia)
    if (fgets(buffer_linha, sizeof(buffer_linha), arquivo) == NULL) {
        fprintf(stderr, "Erro: fim inesperado do arquivo ao ler a segunda linha.\n");
        fclose(arquivo);
        return NULL;
    }

    // 3. Aloca a matriz da geracao atual inicializada com zeros
    int **matriz_t0 = alocar_matriz(*n, *m);
    if (matriz_t0 == NULL) {
        fclose(arquivo);
        return NULL;
    }

    // 4. Le as demais linhas com as coordenadas (i, j) e preenche o estado inicial (t0)
    int i, j;
    while (fgets(buffer_linha, sizeof(buffer_linha), arquivo) != NULL) {
        if (sscanf(buffer_linha, "%d %d", &i, &j) == 2) {
            if (i >= 0 && i < *n && j >= 0 && j < *m) {
                matriz_t0[i][j] = 1;
            } else {
                fprintf(stderr, "Aviso: posicao (%d, %d) fora dos limites [0..%d, 0..%d] ignorada.\n",
                        i, j, *n - 1, *m - 1);
            }
        }
    }

    fclose(arquivo);
    return matriz_t0;
}