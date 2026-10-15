#ifndef ENTRADA_H
#define ENTRADA_H

/*
    Le o arquivo de entrada, preenche as dimensoes (n, m) e o numero de geracoes por referencia,
    e retorna a matriz inicial (t0) ja alocada e preenchida (ou NULL em caso de erro).
*/
int **ler_arquivo_entrada(const char *caminho_arquivo, int *n, int *m, int *geracoes);

#endif