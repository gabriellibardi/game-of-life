#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    ./jogo_da_vida entrada.txt [--sem-impressao]

    ou

    make run ARQUIVO=entradas/arquivo.txt

Limpeza do diretorio:
    make clean
*/

/*
    Exibe as formas de uso e as opcoes disponiveis no programa.
*/
static void exibir_ajuda(const char *nome_programa) {
    printf("Uso:\n");
    printf("  %s <arquivo_de_entrada> [opcoes]\n", nome_programa);
    printf("  %s --ajuda\n\n", nome_programa);
    printf("Opcoes:\n");
    printf("  --sem-impressao  Nao exibe as matrizes inicial e final.\n");
    printf("  --ajuda, -h      Exibe esta mensagem.\n");
}

/*
    Processa os argumentos da linha de comando. Retorna 1 quando os argumentos
    sao validos e 0 quando encontra uma opcao desconhecida ou entrada ausente.
*/
static int processar_argumentos(
    int argc,
    char *argv[],
    const char **caminho_arquivo,
    int *deve_imprimir_matrizes
) {
    *caminho_arquivo = NULL;
    *deve_imprimir_matrizes = 1;

    for (int indice = 1; indice < argc; indice++) {
        if (strcmp(argv[indice], "--sem-impressao") == 0) {
            *deve_imprimir_matrizes = 0;
        } else if (argv[indice][0] == '-') {
            fprintf(stderr, "Erro: opcao desconhecida: %s\n", argv[indice]);
            return 0;
        } else if (*caminho_arquivo == NULL) {
            *caminho_arquivo = argv[indice];
        } else {
            fprintf(stderr, "Erro: informe apenas um arquivo de entrada.\n");
            return 0;
        }
    }

    if (*caminho_arquivo == NULL) {
        fprintf(stderr, "Erro: nenhum arquivo de entrada foi informado.\n");
        return 0;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    for (int indice = 1; indice < argc; indice++) {
        if (strcmp(argv[indice], "--ajuda") == 0 || strcmp(argv[indice], "-h") == 0) {
            exibir_ajuda(argv[0]);
            return EXIT_SUCCESS;
        }
    }

    const char *caminho_arquivo;
    int deve_imprimir_matrizes;
    if (!processar_argumentos(argc, argv, &caminho_arquivo, &deve_imprimir_matrizes)) {
        fprintf(stderr, "Use '%s --ajuda' para consultar as opcoes.\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = 0, m = 0, geracoes = 0;

    // Le o arquivo e ja obtem a matriz atual (t0) preenchida e os parametros n, m e geracoes
    int **matriz_atual = ler_arquivo_entrada(caminho_arquivo, &n, &m, &geracoes);
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
    if (deve_imprimir_matrizes) {
        imprimir_matriz(matriz_atual, n, m);
    }

    // Executa a simulacao sequencial
    simular_sequencial(&matriz_atual, &matriz_proxima, n, m, geracoes);

    printf("\nEstado final apos %d geracoes - Celulas vivas: %d\n", geracoes, contar_celulas_vivas(matriz_atual, n, m));
    if (deve_imprimir_matrizes) {
        imprimir_matriz(matriz_atual, n, m);
    }


    liberar_matriz(matriz_atual, n);
    liberar_matriz(matriz_proxima, n);

    return EXIT_SUCCESS;
}
