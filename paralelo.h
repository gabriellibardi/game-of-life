#ifndef PARALELO_H
#define PARALELO_H

/*
    Executa o Jogo da Vida de Conway com pthreads, distribuindo faixas de linhas.
    As matrizes devem ser distintas, validas, sem linhas compartilhadas,
    e possuir linhas x colunas celulas.
    Aceita 1 <= numero_threads <= linhas e geracoes >= 0.
*/
int simular_paralelo(int ***matriz_atual, int ***matriz_proxima,
                    int linhas, int colunas, int geracoes, int numero_threads);

#endif
