#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entrada.h"
#include "logica.h"
#include "matriz.h"
#include "renderizacao.h"

#define ATRASO_PADRAO_MS 200U
#define ATRASO_MAXIMO_MS 60000UL

typedef struct {
    const char *caminho_arquivo;
    int deve_imprimir_matrizes;
    int deve_animar;
    unsigned int atraso_milissegundos;
} OpcoesExecucao;

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
    printf("  --sem-impressao  Nao exibe as matrizes inicial e final fora da animacao.\n");
    printf("  --animar         Exibe todas as geracoes no terminal.\n");
    printf("  --atraso MS      Define o intervalo da animacao (0 a 60000 ms).\n");
    printf("  --ajuda, -h      Exibe esta mensagem.\n");
}

/*
    Converte o texto do atraso para um valor valido em milissegundos.
*/
static int converter_atraso(const char *texto, unsigned int *atraso_milissegundos) {
    errno = 0;
    char *fim_conversao;
    unsigned long valor = strtoul(texto, &fim_conversao, 10);

    if (texto == fim_conversao || *fim_conversao != '\0'
        || errno == ERANGE || valor > ATRASO_MAXIMO_MS) {
        return 0;
    }

    *atraso_milissegundos = (unsigned int)valor;
    return 1;
}

/*
    Processa os argumentos da linha de comando e preenche as opcoes. Retorna 1
    quando os argumentos sao validos e 0 quando encontra algum erro.
*/
static int processar_argumentos(int argc, char *argv[], OpcoesExecucao *opcoes) {
    opcoes->caminho_arquivo = NULL;
    opcoes->deve_imprimir_matrizes = 1;
    opcoes->deve_animar = 0;
    opcoes->atraso_milissegundos = ATRASO_PADRAO_MS;
    int atraso_foi_informado = 0;

    for (int indice = 1; indice < argc; indice++) {
        if (strcmp(argv[indice], "--sem-impressao") == 0) {
            opcoes->deve_imprimir_matrizes = 0;
        } else if (strcmp(argv[indice], "--animar") == 0) {
            opcoes->deve_animar = 1;
        } else if (strcmp(argv[indice], "--atraso") == 0) {
            if (indice + 1 >= argc
                || !converter_atraso(argv[indice + 1], &opcoes->atraso_milissegundos)) {
                fprintf(stderr, "Erro: --atraso exige um valor inteiro entre 0 e 60000.\n");
                return 0;
            }
            atraso_foi_informado = 1;
            indice++;
        } else if (argv[indice][0] == '-') {
            fprintf(stderr, "Erro: opcao desconhecida: %s\n", argv[indice]);
            return 0;
        } else if (opcoes->caminho_arquivo == NULL) {
            opcoes->caminho_arquivo = argv[indice];
        } else {
            fprintf(stderr, "Erro: informe apenas um arquivo de entrada.\n");
            return 0;
        }
    }

    if (opcoes->caminho_arquivo == NULL) {
        fprintf(stderr, "Erro: nenhum arquivo de entrada foi informado.\n");
        return 0;
    }

    if (atraso_foi_informado && !opcoes->deve_animar) {
        fprintf(stderr, "Erro: --atraso deve ser usado junto com --animar.\n");
        return 0;
    }

    return 1;
}

/*
    Executa a simulacao exibindo o estado inicial e cada geracao calculada.
*/
static int simular_com_animacao(
    int ***matriz_atual,
    int ***matriz_proxima,
    int n,
    int m,
    int geracoes,
    unsigned int atraso_milissegundos
) {
    if (!iniciar_animacao_terminal(n, m)) {
        return 0;
    }

    renderizar_geracao_terminal(
        *matriz_atual, n, m, 0, geracoes, contar_celulas_vivas(*matriz_atual, n, m)
    );

    for (int geracao = 1; geracao <= geracoes; geracao++) {
        aguardar_proximo_quadro(atraso_milissegundos);
        avancar_uma_geracao(matriz_atual, matriz_proxima, n, m);
        renderizar_geracao_terminal(
            *matriz_atual, n, m, geracao, geracoes,
            contar_celulas_vivas(*matriz_atual, n, m)
        );
    }

    finalizar_animacao_terminal();
    return 1;
}

int main(int argc, char *argv[]) {
    for (int indice = 1; indice < argc; indice++) {
        if (strcmp(argv[indice], "--ajuda") == 0 || strcmp(argv[indice], "-h") == 0) {
            exibir_ajuda(argv[0]);
            return EXIT_SUCCESS;
        }
    }

    OpcoesExecucao opcoes;
    if (!processar_argumentos(argc, argv, &opcoes)) {
        fprintf(stderr, "Use '%s --ajuda' para consultar as opcoes.\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = 0, m = 0, geracoes = 0;

    // Le o arquivo e ja obtem a matriz atual (t0) preenchida e os parametros n, m e geracoes
    int **matriz_atual = ler_arquivo_entrada(opcoes.caminho_arquivo, &n, &m, &geracoes);
    if (matriz_atual == NULL) {
        return EXIT_FAILURE;
    }

    // Cria a matriz da proxima geracao com as mesmas dimensoes
    int **matriz_proxima = alocar_matriz(n, m);
    if (matriz_proxima == NULL) {
        liberar_matriz(matriz_atual, n);
        return EXIT_FAILURE;
    }

    if (opcoes.deve_animar) {
        if (!simular_com_animacao(
                &matriz_atual, &matriz_proxima, n, m, geracoes,
                opcoes.atraso_milissegundos)) {
            liberar_matriz(matriz_atual, n);
            liberar_matriz(matriz_proxima, n);
            return EXIT_FAILURE;
        }
    } else {
        printf("Dimensoes: %d x %d | Geracoes: %d\n", n, m, geracoes);
        printf("Estado inicial (t0) - Celulas vivas: %d\n",
               contar_celulas_vivas(matriz_atual, n, m));
        if (opcoes.deve_imprimir_matrizes) {
            imprimir_matriz(matriz_atual, n, m);
        }

        simular_sequencial(&matriz_atual, &matriz_proxima, n, m, geracoes);
    }

    printf("\nEstado final apos %d geracoes - Celulas vivas: %d\n", geracoes, contar_celulas_vivas(matriz_atual, n, m));
    if (!opcoes.deve_animar && opcoes.deve_imprimir_matrizes) {
        imprimir_matriz(matriz_atual, n, m);
    }


    liberar_matriz(matriz_atual, n);
    liberar_matriz(matriz_proxima, n);

    return EXIT_SUCCESS;
}
