#ifndef LOGICA_H
#define LOGICA_H

/*
    Conta quantos dos 8 vizinhos da posicao (linha, coluna) estao vivos (valor 1).
    Posicoes fora dos limites [0..n-1, 0..m-1] sao consideradas mortas.
*/
int contar_vizinhos_vivos(int **matriz, int n, int m, int linha, int coluna);

/*
    Calcula o estado da proxima geracao para uma faixa de linhas.
*/
void calcular_proxima_geracao(int **atual, int **proxima, int n, int m, int linha_inicio, int linha_fim);

/*
    Conta o total de celulas vivas no tabuleiro.
*/
int contar_celulas_vivas(int **matriz, int n, int m);

/*
    Executa a simulacao sequencial por 'geracoes' passos.
*/
void simular_sequencial(int ***matriz_atual, int ***matriz_proxima, int n, int m, int geracoes);

#endif
