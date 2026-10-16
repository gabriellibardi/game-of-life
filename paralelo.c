#define _POSIX_C_SOURCE 200809L

#include "paralelo.h"
#include "logica.h"
#include "matriz.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    int **matriz_atual;              
    int **matriz_proxima;            
    int linhas;
    int colunas;
    int geracoes;                   
    int cancelar_inicio;           
    pthread_mutex_t controle_inicio; 
    pthread_barrier_t barreira;     
} EstadoCompartilhado;

typedef struct {
    EstadoCompartilhado *estado;
    int linha_inicio;            
    int linha_fim;              
    int responsavel_pela_troca; 
} DadosThread;

/* Informa erros de pthreads e encerra o programa quando solicitado. */
static void tratar_erro(const char *operacao, int codigo_erro, int encerrar_programa) {
    if (codigo_erro != 0) {
        fprintf(stderr, "Erro em %s: %s\n", operacao, strerror(codigo_erro));
        if (encerrar_programa) {
            abort();
        }
    }
}

/* Sincroniza as threads em uma barreira. */
static void aguardar_barreira(pthread_barrier_t *barreira) {
    int resultado = pthread_barrier_wait(barreira); 
    if (resultado != PTHREAD_BARRIER_SERIAL_THREAD) {
        tratar_erro("pthread_barrier_wait", resultado, 1);
    }
}

/* Calcula as geracoes da faixa de linhas atribuida a uma thread. */
static void *executar_faixa(void *argumento) {
    DadosThread *dados = argumento;
    EstadoCompartilhado *estado = dados->estado;
    int inicio_cancelado;  

    tratar_erro("pthread_mutex_lock", pthread_mutex_lock(&estado->controle_inicio), 1);
    inicio_cancelado = estado->cancelar_inicio;
    tratar_erro("pthread_mutex_unlock", pthread_mutex_unlock(&estado->controle_inicio), 1);
    
    if (inicio_cancelado) {
        return NULL;
    }

    for (int geracao = 0; geracao < estado->geracoes; geracao++) { 
        calcular_proxima_geracao(estado->matriz_atual, estado->matriz_proxima, estado->linhas, estado->colunas, dados->linha_inicio, dados->linha_fim);
        aguardar_barreira(&estado->barreira);
        if (dados->responsavel_pela_troca) {
            trocar_matrizes(&estado->matriz_atual, &estado->matriz_proxima);
        }
        aguardar_barreira(&estado->barreira);
    }
    return NULL;
}

/* Coordena a simulacao paralela do Jogo da Vida com pthreads. */
int simular_paralelo(int ***matriz_atual, int ***matriz_proxima, int linhas, int colunas, int geracoes, int numero_threads) {
    pthread_t *identificadores;
    DadosThread *dados_threads;
    int resultado;
    int threads_criadas = 0;
    int proxima_linha = 0;
    EstadoCompartilhado estado = {0};

    if (matriz_atual == NULL || matriz_proxima == NULL
        || *matriz_atual == NULL
        || *matriz_proxima == NULL || *matriz_atual == *matriz_proxima
        || linhas <= 0 || colunas <= 0 || geracoes < 0
        || numero_threads <= 0 || numero_threads > linhas) {
        fprintf(stderr, "Erro: parametros invalidos para a simulacao paralela.\n");
        return 0;
    }

    if (geracoes == 0) {
        return 1;
    }

    identificadores = calloc((size_t)numero_threads, sizeof(*identificadores));
    dados_threads = calloc((size_t)numero_threads, sizeof(*dados_threads));

    if (identificadores == NULL || dados_threads == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para preparar as threads.\n");
        free(identificadores);
        free(dados_threads);
        return 0;
    }

    estado.matriz_atual = *matriz_atual;
    estado.matriz_proxima = *matriz_proxima;
    estado.linhas = linhas;
    estado.colunas = colunas;
    estado.geracoes = geracoes;

    resultado = pthread_mutex_init(&estado.controle_inicio, NULL);

    if (resultado != 0) {
        tratar_erro("pthread_mutex_init", resultado, 0);
        free(identificadores);
        free(dados_threads);
        return 0;
    }

    resultado = pthread_barrier_init(&estado.barreira, NULL, (unsigned int)numero_threads);

    if (resultado != 0) {
        tratar_erro("pthread_barrier_init", resultado, 0);
        pthread_mutex_destroy(&estado.controle_inicio);
        free(identificadores);
        free(dados_threads);
        return 0;
    }

    tratar_erro("pthread_mutex_lock", pthread_mutex_lock(&estado.controle_inicio), 1);

    for (int indice_thread = 0; indice_thread < numero_threads; indice_thread++) { 
        int quantidade_linhas = linhas / numero_threads + (indice_thread < linhas % numero_threads); 
        dados_threads[indice_thread].estado = &estado;
        dados_threads[indice_thread].linha_inicio = proxima_linha;
        proxima_linha += quantidade_linhas;
        dados_threads[indice_thread].linha_fim = proxima_linha;
        dados_threads[indice_thread].responsavel_pela_troca = indice_thread == 0;
        resultado = pthread_create(&identificadores[indice_thread], NULL, executar_faixa, &dados_threads[indice_thread]);
        
        if (resultado != 0) {
            tratar_erro("pthread_create", resultado, 0);
            estado.cancelar_inicio = 1;
            break;
        }
        threads_criadas++;
    }

    tratar_erro("pthread_mutex_unlock", pthread_mutex_unlock(&estado.controle_inicio), 1);

    for (int indice_thread = 0; indice_thread < threads_criadas; indice_thread++) {
        tratar_erro("pthread_join", pthread_join(identificadores[indice_thread], NULL), 1);
    }
    
    *matriz_atual = estado.matriz_atual;
    *matriz_proxima = estado.matriz_proxima;

    /* Todas as threads terminaram; estes recursos nao estao mais em uso. */
    pthread_barrier_destroy(&estado.barreira);
    pthread_mutex_destroy(&estado.controle_inicio);
    free(dados_threads);
    free(identificadores);
    return !estado.cancelar_inicio;
}
