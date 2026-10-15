#include <stdio.h>
#include <stdlib.h>
#include "entrada.h"
#include "matriz.h"
#include "logica.h"

/*
Trabalho 1 - Programacao Paralela

Alunos:
    - Gabriel Libardi Lulu (RA: 134728)
    - Matheus Cenerini Jacomini (RA: 134700)
    - Vitor da Rocha Machado (RA: 132769)

Modo de execucao:
    make
    ./jogo_da_vida entrada.txt

    ou

    make run ARQUIVO=entradas/arquivo.txt

Limpeza do diretorio:
    make clean
*/

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <caminho_do_arquivo_de_entrada>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = 0, m = 0, geracoes = 0;

    // Le o arquivo e ja obtem a matriz atual (t0) preenchida e os parametros n, m e geracoes
    int **matriz_atual = ler_arquivo_entrada(argv[1], &n, &m, &geracoes);
    if (matriz_atual == NULL) {
        return EXIT_FAILURE;
    }

    // Cria a matriz da proxima geracao com as mesmas dimensoes
    int **matriz_proxima = alocar_matriz(n, m);
    if (matriz_proxima == NULL) {
        liberar_matriz(matriz_atual, n);
        return EXIT_FAILURE;
    }

    printf("Dimensoes: %d x %d | Geracoes: %d\n", n, m, geracoes);
    printf("Estado inicial (t0) - Celulas vivas: %d\n", contar_celulas_vivas(matriz_atual, n, m));
    imprimir_matriz(matriz_atual, n, m);

    // Executa a simulacao sequencial
    simular_sequencial(&matriz_atual, &matriz_proxima, n, m, geracoes);

    printf("\nEstado final apos %d geracoes - Celulas vivas: %d\n", geracoes, contar_celulas_vivas(matriz_atual, n, m));
    imprimir_matriz(matriz_atual, n, m);


    liberar_matriz(matriz_atual, n);
    liberar_matriz(matriz_proxima, n);

    return EXIT_SUCCESS;
}
