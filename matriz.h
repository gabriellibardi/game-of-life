#ifndef MATRIZ_H
#define MATRIZ_H

/*
    Aloca dinamicamente uma matriz de dimensoes n x m inicializada com zeros (0 = celula morta).
    Utiliza ponteiros de linhas (int**) para permitir a troca rapida de referencias (swap) entre
    a matriz atual e a matriz da proxima geracao ao final de cada geracao.
*/
int **alocar_matriz(int n, int m);

/*
    Libera a memoria alocada para uma matriz de n linhas.
*/
void liberar_matriz(int **matriz, int n);

/*
    Troca as referencias (ponteiros) entre a matriz atual e a matriz da proxima geracao.
*/
void trocar_matrizes(int ***atual, int ***proxima);

/*
    Compara duas matrizes de mesmas dimensoes, celula por celula. Retorna 1 se
    todos os valores forem iguais e 0 ao encontrar a primeira diferenca.
*/
int matrizes_iguais(int **primeira, int **segunda, int n, int m);

/*
    Funcao auxiliar para visualizar o estado do tabuleiro no terminal.
*/
void imprimir_matriz(int **matriz, int n, int m);

#endif
