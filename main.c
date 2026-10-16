#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "entrada.h"
#include "logica.h"
#include "matriz.h"
#include "paralelo.h"
#include "renderizacao.h"
#include "tempo.h"

#define ATRASO_PADRAO_MS 200U
#define ATRASO_MAXIMO_MS 60000UL

typedef struct {
    const char *caminho_arquivo;
    const char *caminho_saida;
    int deve_imprimir_matrizes;
    int deve_animar;
    unsigned int atraso_milissegundos;
    int deve_paralelizar; /* Seleciona o calculo com pthreads. */
    int deve_verificar;   /* Executa a outra versao e compara todas as celulas. */
    int numero_threads;   /* Quantidade configurada para a versao paralela. */
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
    printf("  --sequencial    Executa sem threads (padrao).\n");
    printf("  --threads N     Executa com pthreads; N deve ser de 1 ao numero de linhas.\n");
    printf("  --verificar     Compara as versoes fora do intervalo cronometrado.\n");
    printf("  --sem-impressao  Nao exibe as matrizes inicial e final fora da animacao.\n");
    printf("  --animar         Exibe todas as geracoes no terminal.\n");
    printf("  --atraso MS      Define o intervalo da animacao (0 a 60000 ms).\n");
    printf("  --saida ARQUIVO  Salva o estado final como uma imagem PBM.\n");
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

/* Converte uma quantidade positiva de threads sem aceitar texto extra. */
static int converter_threads(const char *texto, int *numero_threads) {
    char *fim_conversao; 
    long quantidade;  
    errno = 0;
    quantidade = strtol(texto, &fim_conversao, 10);
    if (*fim_conversao != '\0' || errno == ERANGE
        || quantidade < 1 || quantidade > INT_MAX) {
        return 0;
    }
    *numero_threads = (int)quantidade;
    return 1;
}

/*
    Processa os argumentos da linha de comando e preenche as opcoes. Retorna 1
    quando os argumentos sao validos e 0 quando encontra algum erro.
*/
static int processar_argumentos(int argc, char *argv[], OpcoesExecucao *opcoes) {
    opcoes->caminho_arquivo = NULL;
    opcoes->caminho_saida = NULL;
    opcoes->deve_imprimir_matrizes = 1;
    opcoes->deve_animar = 0;
    opcoes->atraso_milissegundos = ATRASO_PADRAO_MS;
    opcoes->deve_paralelizar = 0;
    opcoes->deve_verificar = 0;
    opcoes->numero_threads = 1;
    int sequencial_explicito = 0; 

    for (int indice = 1; indice < argc; indice++) {
        if (strcmp(argv[indice], "--sequencial") == 0) {
            sequencial_explicito = 1;
        } else if (strcmp(argv[indice], "--threads") == 0) {
            if (indice + 1 >= argc
                || !converter_threads(argv[indice + 1], &opcoes->numero_threads)) {
                fprintf(stderr, "Erro: --threads exige um inteiro positivo ate %d.\n", INT_MAX);
                return 0;
            }
            opcoes->deve_paralelizar = 1;
            indice++;
        } else if (strcmp(argv[indice], "--verificar") == 0) {
            opcoes->deve_verificar = 1;
        } else if (strcmp(argv[indice], "--sem-impressao") == 0) {
            opcoes->deve_imprimir_matrizes = 0;
        } else if (strcmp(argv[indice], "--animar") == 0) {
            opcoes->deve_animar = 1;
        } else if (strcmp(argv[indice], "--atraso") == 0) {
            if (indice + 1 >= argc
                || !converter_atraso(argv[indice + 1], &opcoes->atraso_milissegundos)) {
                fprintf(stderr, "Erro: --atraso exige um valor inteiro entre 0 e 60000.\n");
                return 0;
            }
            indice++;
        } else if (strcmp(argv[indice], "--saida") == 0) {
            if (indice + 1 >= argc || argv[indice + 1][0] == '-') {
                fprintf(stderr, "Erro: --saida exige o caminho do arquivo PBM.\n");
                return 0;
            }
            opcoes->caminho_saida = argv[indice + 1];
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

    if (sequencial_explicito && opcoes->deve_paralelizar) {
        fprintf(stderr, "Erro: --sequencial nao pode acompanhar --threads.\n");
        return 0;
    }
    if (opcoes->deve_animar && (opcoes->deve_paralelizar || opcoes->deve_verificar)) {
        fprintf(stderr, "Erro: --animar exige modo sequencial sem --verificar.\n");
        return 0;
    }
    return 1;
}

/*
    Executa apenas o modo solicitado e mede seu tempo. No paralelo, a medicao
    inclui criacao, sincronizacao e joins; leitura, copia e verificacao ficam fora.
    Uma falha do relogio impede a medicao, mas nao repete nem cancela o calculo.
*/
static int executar_simulacao(int ***atual, int ***proxima, int linhas, int colunas,
                            int geracoes, const OpcoesExecucao *opcoes,
                            double *tempo, int *tempo_medido) {
    double inicio; 
    double fim;   
    int relogio_disponivel = obter_tempo_monotonico(&inicio); 
    *tempo_medido = 0;
    if (opcoes->deve_paralelizar) {
        if (!simular_paralelo(atual, proxima, linhas, colunas,
                              geracoes, opcoes->numero_threads)) {
            return 0;
        }
    } else {
        simular_sequencial(atual, proxima, linhas, colunas, geracoes);
    }
    if (relogio_disponivel && obter_tempo_monotonico(&fim)) {
        *tempo = fim - inicio;
        *tempo_medido = 1;
    }
    return 1;
}

/*
    Calcula a versao de referencia sobre uma copia independente do estado inicial.
    Compara todas as celulas e informa se os resultados coincidem. Nao mede tempo.
*/
static int verificar_resultado(int **resultado, int ***referencia, int ***auxiliar,
                              int linhas, int colunas, int geracoes,
                              const OpcoesExecucao *opcoes) {
    if (opcoes->deve_paralelizar) {
        simular_sequencial(referencia, auxiliar, linhas, colunas, geracoes);
    } else if (!simular_paralelo(referencia, auxiliar, linhas, colunas,
                                 geracoes, opcoes->numero_threads)) {
        return 0;
    }
    if (matrizes_iguais(resultado, *referencia, linhas, colunas)) {
        printf("Verificacao: OK - resultados sequencial e paralelo identicos.\n");
        return 1;
    }
    fprintf(stderr, "Verificacao: FALHOU - resultados sequencial e paralelo diferentes.\n");
    return 0;
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

    for (int geracao = 0; geracao < geracoes; geracao++) {
        aguardar_proximo_quadro(atraso_milissegundos);
        avancar_uma_geracao(matriz_atual, matriz_proxima, n, m);
        renderizar_geracao_terminal(
            *matriz_atual, n, m, geracao + 1, geracoes,
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

    double tempo_simulacao = 0.0;
    int tempo_foi_medido = 0;
    int codigo_saida = EXIT_FAILURE; 
    int **referencia = NULL; 
    int **auxiliar_referencia = NULL; 
    if (opcoes.deve_verificar) {
        referencia = alocar_matriz(n, m);
        auxiliar_referencia = alocar_matriz(n, m);
        if (referencia == NULL || auxiliar_referencia == NULL) {
            goto liberar_recursos;
        }
        for (int linha = 0; linha < n; linha++) { 
            for (int coluna = 0; coluna < m; coluna++) { 
                referencia[linha][coluna] = matriz_atual[linha][coluna];
            }
        }
    }

    if (opcoes.deve_animar) {
        if (!simular_com_animacao(
                &matriz_atual, &matriz_proxima, n, m, geracoes,
                opcoes.atraso_milissegundos)) {
            goto liberar_recursos;
        }
    } else {
        printf("Dimensoes: %d x %d | Geracoes: %d\n", n, m, geracoes);
        printf("Modo: %s | Threads: %d\n", opcoes.deve_paralelizar ? "paralelo" : "sequencial",
               opcoes.deve_paralelizar ? opcoes.numero_threads : 1);
        printf("Estado inicial (t0) - Celulas vivas: %d\n",
               contar_celulas_vivas(matriz_atual, n, m));
        if (opcoes.deve_imprimir_matrizes) {
            imprimir_matriz(matriz_atual, n, m);
        }

        if (!executar_simulacao(&matriz_atual, &matriz_proxima, n, m, geracoes,
                                &opcoes, &tempo_simulacao, &tempo_foi_medido)) {
            goto liberar_recursos;
        }
    }

    printf("\nEstado final apos %d geracoes - Celulas vivas: %d\n", geracoes, contar_celulas_vivas(matriz_atual, n, m));
    if (tempo_foi_medido) {
        printf("Tempo da simulacao %s: %.9f s\n",
               opcoes.deve_paralelizar ? "paralela" : "sequencial", tempo_simulacao);
    } else if (opcoes.deve_animar) {
        printf("Tempo nao medido: desative --animar para avaliar o desempenho.\n");
    } else {
        printf("Tempo nao medido: falha ao consultar o relogio.\n");
    }
    if (!opcoes.deve_animar && opcoes.deve_imprimir_matrizes) {
        imprimir_matriz(matriz_atual, n, m);
    }

    if (opcoes.deve_verificar
        && !verificar_resultado(matriz_atual, &referencia, &auxiliar_referencia,
                                n, m, geracoes, &opcoes)) {
        goto liberar_recursos;
    }
    if (opcoes.caminho_saida != NULL) {
        if (!salvar_matriz_pbm(opcoes.caminho_saida, matriz_atual, n, m)) {
            goto liberar_recursos;
        }
        printf("Imagem final salva em: %s\n", opcoes.caminho_saida);
    }

    codigo_saida = EXIT_SUCCESS;
liberar_recursos:
    liberar_matriz(referencia, n);
    liberar_matriz(auxiliar_referencia, n);
    liberar_matriz(matriz_atual, n);
    liberar_matriz(matriz_proxima, n);

    return codigo_saida;
}
