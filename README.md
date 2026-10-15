# Jogo da Vida de Conway

Implementacao sequencial do Jogo da Vida de Conway em C. O projeto simula a
evolucao de um tabuleiro finito, permite acompanhar as geracoes no terminal,
exporta o resultado como imagem preto e branco e mede o tempo gasto no calculo.

Este repositorio faz parte do trabalho da disciplina de Programacao Paralela da
Universidade Estadual de Maringa (UEM). A versao com pthreads sera desenvolvida
posteriormente pelos integrantes do grupo.

## Integrantes

- Gabriel Libardi Lulu — RA 134728
- Matheus Cenerini Jacomini — RA 134700
- Vitor da Rocha Machado — RA 132769

## Regras da simulacao

Cada posicao do tabuleiro representa uma celula morta (`0`) ou viva (`1`). Em
cada geracao, todas as celulas sao atualizadas simultaneamente:

- Uma celula viva sobrevive quando possui dois ou tres vizinhos vivos.
- Uma celula viva morre por solidao ou superpopulacao nos demais casos.
- Uma celula morta nasce quando possui exatamente tres vizinhos vivos.

O tabuleiro e finito. Posicoes fora de seus limites sao consideradas mortas;
portanto, as bordas nao se conectam ao lado oposto.

## Requisitos

- GCC com suporte a C11
- GNU Make
- Ambiente POSIX para o relogio monotonicamente crescente e a animacao ANSI

Nenhuma biblioteca grafica externa e necessaria.

## Compilacao

Para gerar o executavel otimizado:

```bash
make
```

O executavel criado se chama `jogo_da_vida`.

Para gerar uma versao separada com simbolos de depuracao, AddressSanitizer e
UndefinedBehaviorSanitizer:

```bash
make debug
./jogo_da_vida_debug entradas/glider.txt --sem-impressao
```

Para remover objetos, dependencias e executaveis gerados:

```bash
make clean
```

## Formato da entrada

A primeira linha contem o numero de linhas, o numero de colunas e a quantidade
de geracoes:

```text
linhas colunas geracoes
```

As linhas seguintes informam as coordenadas das celulas inicialmente vivas:

```text
linha coluna
```

Exemplo de um oscilador blinker em um tabuleiro `5 x 5`:

```text
5 5 4

2 1
2 2
2 3
```

As coordenadas comecam em zero. Linhas vazias sao opcionais e podem aparecer
entre as coordenadas. Coordenadas repetidas nao alteram o resultado.
Coordenadas fora do tabuleiro geram um aviso e sao ignoradas. Conteudo
malformado faz o programa encerrar com erro e informar a linha correspondente.

## Execucao

Uso geral:

```bash
./jogo_da_vida <arquivo_de_entrada> [opcoes]
```

Execucao convencional:

```bash
./jogo_da_vida entradas/glider.txt
```

As opcoes disponiveis sao:

- `--sem-impressao`: omite as matrizes inicial e final.
- `--animar`: mostra todas as geracoes no terminal.
- `--atraso MS`: define o intervalo da animacao entre 0 e 60000 ms.
- `--saida ARQUIVO`: salva o estado final em uma imagem PBM.
- `--ajuda` ou `-h`: mostra a ajuda do programa.

Tambem e possivel executar por meio do Makefile:

```bash
make run
make run ARQUIVO=entradas/mistos.txt ARGS="--sem-impressao"
```

## Animacao no terminal

Para acompanhar as geracoes com um intervalo de 150 ms:

```bash
./jogo_da_vida entradas/glider.txt --animar --atraso 150
```

Na animacao, celulas vivas aparecem em branco e celulas mortas em preto. Cada
celula ocupa dois caracteres para compensar a proporcao do terminal. A opcao
requer uma saida interativa e uma janela suficientemente grande para o
tabuleiro. Matrizes grandes devem ser executadas com `--sem-impressao`.

A medicao de desempenho nao e feita durante a animacao, pois atrasos e desenho
dos quadros distorceriam o resultado.

## Exportacao PBM

O estado final pode ser salvo no formato PBM textual (`P1`):

```bash
./jogo_da_vida entradas/blinker.txt --sem-impressao --saida resultado.pbm
```

No arquivo PBM, celulas vivas sao pixels pretos e celulas mortas sao pixels
brancos. O formato pode ser aberto por visualizadores de imagem compatíveis ou
convertido posteriormente por ferramentas externas.

## Medicao de tempo

Nas execucoes sem animacao, o programa informa automaticamente o tempo da
simulacao sequencial:

```text
Tempo da simulacao sequencial: 0.012345678 s
```

O intervalo medido inclui apenas a evolucao das geracoes. Ficam fora da medicao
a leitura da entrada, a alocacao, as contagens, a impressao e a exportacao PBM.
Para experimentos, use matrizes maiores e desative a impressao:

```bash
./jogo_da_vida entradas/mistos.txt --sem-impressao
```

## Testes automatizados

Execute a suite com:

```bash
make test
```

Os testes cobrem tabuleiro vazio, celula isolada, bloco estavel, blinker,
glider, zero geracoes, tabuleiro `1 x 1`, bordas, matrizes retangulares,
comparacao de matrizes e validacao dos arquivos de entrada.

Durante os casos de entrada invalida, um aviso e uma mensagem de erro aparecem
propositalmente. Ao final, o resultado esperado e:

```text
14 testes executados: 14 aprovados e 0 falharam.
```

## Organizacao do codigo

- `main.c`: argumentos, fluxo da aplicacao e integracao dos modulos.
- `entrada.c` e `entrada.h`: leitura e validacao do arquivo de entrada.
- `matriz.c` e `matriz.h`: alocacao, liberacao, troca, comparacao e impressao.
- `logica.c` e `logica.h`: regras e simulacao sequencial.
- `renderizacao.c` e `renderizacao.h`: animacao ANSI e exportacao PBM.
- `tempo.c` e `tempo.h`: consulta ao relogio monotonicamente crescente.
- `testes/`: suite automatizada e entradas usadas nos testes.
- `entradas/`: exemplos prontos para execucao.

## Limitacoes atuais

- Somente a versao sequencial esta implementada.
- A animacao depende de um terminal com suporte a sequencias ANSI.
- O tabuleiro usa bordas mortas, sem comportamento toroidal.
- A imagem PBM representa apenas a geracao final.
- A impressao e a animacao nao sao adequadas para tabuleiros muito grandes.
