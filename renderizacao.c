#define _POSIX_C_SOURCE 199309L

#include "renderizacao.h"
#include <errno.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

int iniciar_animacao_terminal(int n, int m) {
    if (!isatty(STDOUT_FILENO)) {
        fprintf(stderr, "Erro: a animacao requer um terminal interativo.\n");
        return 0;
    }

    struct winsize tamanho_terminal;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &tamanho_terminal) == 0) {
        unsigned long altura_necessaria = (unsigned long)n + 2;
        unsigned long largura_necessaria = (unsigned long)m * 2;

        if ((tamanho_terminal.ws_row > 0 && altura_necessaria > tamanho_terminal.ws_row)
            || (tamanho_terminal.ws_col > 0 && largura_necessaria > tamanho_terminal.ws_col)) {
            fprintf(stderr,
                    "Erro: a matriz %d x %d nao cabe no terminal atual (%u x %u).\n",
                    n, m, (unsigned int)tamanho_terminal.ws_row,
                    (unsigned int)tamanho_terminal.ws_col);
            return 0;
        }
    }

    printf("\033[2J\033[H\033[?25l");
    fflush(stdout);
    return 1;
}

void renderizar_geracao_terminal(
    int **matriz,
    int n,
    int m,
    int geracao_atual,
    int total_geracoes,
    int celulas_vivas
) {
    printf("\033[H\033[2KGeracao %d/%d | Celulas vivas: %d | Vivas em branco\n\n",
           geracao_atual, total_geracoes, celulas_vivas);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            fputs(matriz[i][j] == 1 ? "\033[47m  " : "\033[40m  ", stdout);
        }
        printf("\033[0m\n");
    }

    fflush(stdout);
}

void aguardar_proximo_quadro(unsigned int atraso_milissegundos) {
    if (atraso_milissegundos == 0) {
        return;
    }

    struct timespec tempo_restante = {
        .tv_sec = atraso_milissegundos / 1000,
        .tv_nsec = (long)(atraso_milissegundos % 1000) * 1000000L
    };

    while (nanosleep(&tempo_restante, &tempo_restante) == -1 && errno == EINTR) {
    }
}

void finalizar_animacao_terminal(void) {
    printf("\033[0m\033[?25h\n");
    fflush(stdout);
}
