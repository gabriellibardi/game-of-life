#define _POSIX_C_SOURCE 199309L

#include "tempo.h"
#include <stdio.h>
#include <time.h>

int obter_tempo_monotonico(double *segundos) {
    struct timespec instante;
    if (clock_gettime(CLOCK_MONOTONIC, &instante) != 0) {
        perror("Erro ao consultar o relogio monotonicamente crescente");
        return 0;
    }

    *segundos = (double)instante.tv_sec + (double)instante.tv_nsec / 1000000000.0;
    return 1;
}
