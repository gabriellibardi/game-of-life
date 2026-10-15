#include <stdio.h>
#include <stdlib.h>
#include "entrada.h"
#include "logica.h"
#include "matriz.h"

typedef struct {
    int linha;
    int coluna;
} Coordenada;

static int total_testes = 0;
static int total_falhas = 0;

/*
    Registra e exibe o resultado de um caso de teste.
*/
static void registrar_resultado(const char *nome, int passou) {
    total_testes++;

    if (passou) {
        printf("[OK] %s\n", nome);
    } else {
        printf("[FALHOU] %s\n", nome);
        total_falhas++;
    }
}

/*
    Cria uma matriz e marca como vivas as coordenadas recebidas.
*/
static int **criar_matriz_com_celulas(int n, int m, const Coordenada *celulas, size_t quantidade) {
    int **matriz = alocar_matriz(n, m);
    if (matriz == NULL) {
        return NULL;
    }

    for (size_t indice = 0; indice < quantidade; indice++) {
        matriz[celulas[indice].linha][celulas[indice].coluna] = 1;
    }

    return matriz;
}

/*
    Executa uma quantidade de geracoes e compara o resultado completo com a
    matriz esperada para o caso de teste.
*/
static int evolucao_corresponde_ao_esperado(
    int n,
    int m,
    int geracoes,
    const Coordenada *iniciais,
    size_t quantidade_iniciais,
    const Coordenada *esperadas,
    size_t quantidade_esperadas
) {
    int **matriz_atual = criar_matriz_com_celulas(n, m, iniciais, quantidade_iniciais);
    int **matriz_proxima = alocar_matriz(n, m);
    int **matriz_esperada = criar_matriz_com_celulas(n, m, esperadas, quantidade_esperadas);

    if (matriz_atual == NULL || matriz_proxima == NULL || matriz_esperada == NULL) {
        liberar_matriz(matriz_atual, n);
        liberar_matriz(matriz_proxima, n);
        liberar_matriz(matriz_esperada, n);
        return 0;
    }

    simular_sequencial(&matriz_atual, &matriz_proxima, n, m, geracoes);
    int resultado_correto = matrizes_iguais(matriz_atual, matriz_esperada, n, m);

    liberar_matriz(matriz_atual, n);
    liberar_matriz(matriz_proxima, n);
    liberar_matriz(matriz_esperada, n);

    return resultado_correto;
}

/*
    Testa configuracoes conhecidas do Jogo da Vida e casos de borda.
*/
static void testar_regras_do_jogo(void) {
    const Coordenada celula_unica[] = {{1, 1}};
    const Coordenada celula_1x1[] = {{0, 0}};
    const Coordenada bloco[] = {{1, 1}, {1, 2}, {2, 1}, {2, 2}};
    const Coordenada blinker_horizontal[] = {{2, 1}, {2, 2}, {2, 3}};
    const Coordenada blinker_vertical[] = {{1, 2}, {2, 2}, {3, 2}};
    const Coordenada glider_inicial[] = {{1, 2}, {2, 3}, {3, 1}, {3, 2}, {3, 3}};
    const Coordenada glider_apos_quatro[] = {{2, 3}, {3, 4}, {4, 2}, {4, 3}, {4, 4}};
    const Coordenada canto_inicial[] = {{0, 0}, {0, 1}, {1, 0}};
    const Coordenada canto_esperado[] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};
    const Coordenada retangular_inicial[] = {{1, 1}, {1, 2}, {1, 3}};
    const Coordenada retangular_esperado[] = {{0, 2}, {1, 2}, {2, 2}};

    registrar_resultado(
        "tabuleiro vazio permanece vazio",
        evolucao_corresponde_ao_esperado(4, 4, 3, NULL, 0, NULL, 0)
    );
    registrar_resultado(
        "celula isolada morre",
        evolucao_corresponde_ao_esperado(3, 3, 1, celula_unica, 1, NULL, 0)
    );
    registrar_resultado(
        "bloco permanece estavel",
        evolucao_corresponde_ao_esperado(4, 4, 5, bloco, 4, bloco, 4)
    );
    registrar_resultado(
        "blinker gira apos uma geracao",
        evolucao_corresponde_ao_esperado(5, 5, 1, blinker_horizontal, 3, blinker_vertical, 3)
    );
    registrar_resultado(
        "blinker retorna apos duas geracoes",
        evolucao_corresponde_ao_esperado(5, 5, 2, blinker_horizontal, 3, blinker_horizontal, 3)
    );
    registrar_resultado(
        "glider se desloca apos quatro geracoes",
        evolucao_corresponde_ao_esperado(6, 6, 4, glider_inicial, 5, glider_apos_quatro, 5)
    );
    registrar_resultado(
        "zero geracoes preserva o estado inicial",
        evolucao_corresponde_ao_esperado(3, 3, 0, celula_unica, 1, celula_unica, 1)
    );
    registrar_resultado(
        "tabuleiro 1 x 1 aplica corretamente as bordas",
        evolucao_corresponde_ao_esperado(1, 1, 1, celula_1x1, 1, NULL, 0)
    );
    registrar_resultado(
        "celulas no canto consideram o exterior morto",
        evolucao_corresponde_ao_esperado(3, 3, 1, canto_inicial, 3, canto_esperado, 4)
    );
    registrar_resultado(
        "tabuleiro retangular evolui corretamente",
        evolucao_corresponde_ao_esperado(3, 5, 1, retangular_inicial, 3, retangular_esperado, 3)
    );
}

/*
    Testa diretamente a funcao usada para comparar resultados completos.
*/
static void testar_comparacao_de_matrizes(void) {
    int **primeira = alocar_matriz(2, 2);
    int **segunda = alocar_matriz(2, 2);

    if (primeira == NULL || segunda == NULL) {
        registrar_resultado("comparacao de matrizes", 0);
        liberar_matriz(primeira, 2);
        liberar_matriz(segunda, 2);
        return;
    }

    int iguais_inicialmente = matrizes_iguais(primeira, segunda, 2, 2);
    segunda[1][1] = 1;
    int diferentes_apos_alteracao = !matrizes_iguais(primeira, segunda, 2, 2);

    registrar_resultado(
        "comparacao detecta igualdade e diferenca",
        iguais_inicialmente && diferentes_apos_alteracao
    );

    liberar_matriz(primeira, 2);
    liberar_matriz(segunda, 2);
}

/*
    Testa o leitor com linha vazia opcional, coordenada fora dos limites e
    conteudo malformado.
*/
static void testar_arquivos_de_entrada(void) {
    int n = 0;
    int m = 0;
    int geracoes = 0;

    int **sem_linha_vazia = ler_arquivo_entrada(
        "testes/entradas/sem_linha_vazia.txt", &n, &m, &geracoes
    );
    int entrada_valida = sem_linha_vazia != NULL
        && n == 3
        && m == 4
        && geracoes == 2
        && contar_celulas_vivas(sem_linha_vazia, n, m) == 2
        && sem_linha_vazia[0][0] == 1
        && sem_linha_vazia[2][3] == 1;
    registrar_resultado("entrada sem linha vazia", entrada_valida);
    liberar_matriz(sem_linha_vazia, n);

    printf("Os dois diagnosticos a seguir sao esperados pelos testes:\n");
    fflush(stdout);

    int **fora_dos_limites = ler_arquivo_entrada(
        "testes/entradas/coordenada_fora_dos_limites.txt", &n, &m, &geracoes
    );
    int coordenada_ignorada = fora_dos_limites != NULL
        && contar_celulas_vivas(fora_dos_limites, n, m) == 1
        && fora_dos_limites[1][1] == 1;
    registrar_resultado("coordenada fora dos limites e ignorada", coordenada_ignorada);
    liberar_matriz(fora_dos_limites, n);

    int **malformada = ler_arquivo_entrada(
        "testes/entradas/malformada.txt", &n, &m, &geracoes
    );
    registrar_resultado("entrada malformada e rejeitada", malformada == NULL);
    liberar_matriz(malformada, n);
}

int main(void) {
    testar_regras_do_jogo();
    testar_comparacao_de_matrizes();
    testar_arquivos_de_entrada();

    printf("\n%d testes executados: %d aprovados e %d falharam.\n",
           total_testes, total_testes - total_falhas, total_falhas);

    return total_falhas == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
