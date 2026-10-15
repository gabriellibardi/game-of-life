#ifndef RENDERIZACAO_H
#define RENDERIZACAO_H

/*
    Verifica se a saida e o tamanho do terminal permitem a animacao. Em caso
    positivo, limpa a tela e oculta o cursor. Retorna 1 em caso de sucesso.
*/
int iniciar_animacao_terminal(int n, int m);

/*
    Desenha uma geracao usando fundos branco e preto para representar as
    celulas. O cursor deve ter sido preparado por iniciar_animacao_terminal.
*/
void renderizar_geracao_terminal(
    int **matriz,
    int n,
    int m,
    int geracao_atual,
    int total_geracoes,
    int celulas_vivas
);

/*
    Suspende a execucao pelo intervalo indicado entre dois quadros.
*/
void aguardar_proximo_quadro(unsigned int atraso_milissegundos);

/*
    Restaura as cores e torna o cursor do terminal visivel novamente.
*/
void finalizar_animacao_terminal(void);

#endif
