#include "logica.h"
#include "matriz.h"

/*
    Conta quantos dos 8 vizinhos da posicao (linha, coluna) estao vivos (valor 1).
    Posicoes fora dos limites [0..n-1, 0..m-1] sao consideradas mortas.
*/
int contar_vizinhos_vivos(int **matriz, int n, int m, int linha, int coluna) {
    int vivos = 0;

    for (int di = -1; di <= 1; di++) {
        for (int dj = -1; dj <= 1; dj++) {
            if (di == 0 && dj == 0) {
                continue; // Ignora a propria celula
            }

            int ni = linha + di;
            int nj = coluna + dj;

            // Verifica se o vizinho esta dentro dos limites do tabuleiro
            if (ni >= 0 && ni < n && nj >= 0 && nj < m) {
                vivos += matriz[ni][nj];
            }
        }
    }

    return vivos;
}

void calcular_proxima_geracao(int **atual, int **proxima, int n, int m, int linha_inicio, int linha_fim) {
    for (int i = linha_inicio; i < linha_fim; i++) {
        for (int j = 0; j < m; j++) {
            int vizinhos = contar_vizinhos_vivos(atual, n, m, i, j);

            if (atual[i][j] == 1) {
                // Celula viva: permanece viva apenas se tiver 2 ou 3 vizinhos vivos
                proxima[i][j] = (vizinhos == 2 || vizinhos == 3) ? 1 : 0;
            } else {
                // Celula morta: torna-se viva por reproducao se tiver exatamente 3 vizinhos vivos
                proxima[i][j] = (vizinhos == 3) ? 1 : 0;
            }
        }
    }
}

void avancar_uma_geracao(int ***matriz_atual, int ***matriz_proxima, int n, int m) {
    calcular_proxima_geracao(*matriz_atual, *matriz_proxima, n, m, 0, n);
    trocar_matrizes(matriz_atual, matriz_proxima);
}

int contar_celulas_vivas(int **matriz, int n, int m) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            total += matriz[i][j];
        }
    }
    return total;
}

void simular_sequencial(int ***matriz_atual, int ***matriz_proxima, int n, int m, int geracoes) {
    for (int g = 0; g < geracoes; g++) {
        avancar_uma_geracao(matriz_atual, matriz_proxima, n, m);
    }
}
