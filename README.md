# Jogo da Vida de Conway

Implementações sequencial e paralela (pthreads) do Jogo da Vida de Conway em C.
O projeto simula a evolução de um tabuleiro finito, permite acompanhar as
gerações no terminal, exporta o resultado como imagem em preto e branco e mede
o tempo gasto no cálculo.

Este repositório faz parte do trabalho da disciplina de Programação Paralela da
Universidade Estadual de Maringá (UEM). Ambas as versões estão integradas ao
executável, com verificação de corretude opcional.

## Integrantes

- Gabriel Libardi Lulu — RA 134728
- Matheus Cenerini Jacomini — RA 134700
- Vitor da Rocha Machado — RA 132769

## Regras da simulação

Cada posição do tabuleiro representa uma célula morta (`0`) ou viva (`1`). Em
cada geração, todas as células são atualizadas simultaneamente:

- Uma célula viva sobrevive quando possui dois ou três vizinhos vivos.
- Uma célula viva morre por solidão ou superpopulação nos demais casos.
- Uma célula morta nasce quando possui exatamente três vizinhos vivos.

O tabuleiro é finito. Posições fora de seus limites são consideradas mortas;
portanto, as bordas não se conectam ao lado oposto.

## Requisitos

- GCC com suporte a C11
- GNU Make
- Linux/WSL com pthreads, barreiras POSIX e relógio monotônico
- Python 3 para os scripts de experimentos

A aplicação C dispensa bibliotecas gráficas externas. A geração dos gráficos
pelos scripts utiliza ReportLab, conforme explicado ao final.

## Compilação

Para gerar o executável otimizado:

```bash
make
```

O executável criado se chama `jogo_da_vida`.

Para gerar uma versão separada com símbolos de depuração, AddressSanitizer e
UndefinedBehaviorSanitizer:

```bash
make debug
./jogo_da_vida_debug entradas/glider.txt --sem-impressao
```

Para remover objetos, dependências e executáveis gerados:

```bash
make clean
```

## Formato da entrada

A primeira linha contém o número de linhas, o número de colunas e a quantidade
de gerações:

```text
linhas colunas geracoes
```

As linhas seguintes informam as coordenadas das células inicialmente vivas:

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

As coordenadas começam em zero. Linhas vazias são opcionais e podem aparecer
entre as coordenadas. Coordenadas repetidas não alteram o resultado.
Coordenadas fora do tabuleiro geram um aviso e são ignoradas. Conteúdo
malformado faz o programa encerrar com erro e informar a linha correspondente.

## Execução

Uso geral:

```bash
./jogo_da_vida <arquivo_de_entrada> [opcoes]
```

Execução convencional:

```bash
./jogo_da_vida entradas/glider.txt
```

As opções disponíveis são:

- `--sequencial`: seleciona a versão sequencial (padrão).
- `--threads N`: seleciona a execução paralela com N threads. Sem essa opção,
  a execução é sequencial. Para usar pthreads com uma única thread, informe
  `--threads 1`.
- `--verificar`: executa a outra versão sobre uma cópia do estado inicial e compara
  todas as células finais, fora do intervalo cronometrado.
- `--sem-impressao`: omite as matrizes inicial e final.
- `--animar`: mostra todas as gerações no terminal.
- `--atraso MS`: define o intervalo da animação entre 0 e 60000 ms; sem
  `--animar`, o valor é ignorado.
- `--saida ARQUIVO`: salva o estado final em uma imagem PBM.
- `--ajuda` ou `-h`: mostra a ajuda do programa.

Também é possível executar por meio do Makefile:

```bash
make run
make run ARQUIVO=entradas/mistos.txt ARGS="--sem-impressao"
```

Para executar com 4 threads e conferir o resultado:

```bash
./jogo_da_vida entradas/mistos.txt --threads 4 --verificar --sem-impressao
```

Sem `--verificar`, somente o modo escolhido é executado. A verificação mostra
`Verificacao: OK` quando os resultados são idênticos. Uma divergência gera a
mensagem `Verificacao: FALHOU` e encerra com código de erro. Também é possível
usar `--sequencial --verificar`; nesse caso a referência paralela usa 1 thread.
`--sequencial` não pode acompanhar `--threads`.

## Animação no terminal

Para acompanhar as gerações com um intervalo de 150 ms:

```bash
./jogo_da_vida entradas/glider.txt --animar --atraso 150
```

Na animação, células vivas aparecem em branco e células mortas em preto. Cada
célula ocupa dois caracteres para compensar a proporção do terminal. A opção
requer uma saída interativa e uma janela suficientemente grande para o
tabuleiro. Matrizes grandes devem ser executadas com `--sem-impressao`.

A medição de desempenho não é feita durante a animação, pois atrasos e desenho
dos quadros distorceriam o resultado.
`--animar` funciona apenas no modo sequencial e sem `--verificar`.

## Exportação PBM

O estado final pode ser salvo no formato PBM textual (`P1`):

```bash
./jogo_da_vida entradas/blinker.txt --sem-impressao --saida resultado.pbm
```

No arquivo PBM, células vivas são pixels pretos e células mortas são pixels
brancos. O formato pode ser aberto por visualizadores de imagem compatíveis ou
convertido posteriormente por ferramentas externas.

## Medição de tempo

Nas execuções sem animação, o programa informa o tempo do modo escolhido:

```text
Tempo da simulacao sequencial: 0.012345678 s
```

O intervalo medido inclui a evolução das gerações. No modo paralelo, inclui
também preparação, criação e término das threads e sincronização. Ficam fora
a leitura, alocação das matrizes, cópias, contagens, impressão, exportação e
verificação. Portanto, o tempo paralelo considera o custo da equipe completa.
Para experimentos, use matrizes maiores e desative a impressão:

```bash
./jogo_da_vida entradas/mistos.txt --sem-impressao
```

## Testes automatizados

Execute a suíte sequencial com:

```bash
make test
```

Os 14 testes sequenciais cobrem tabuleiro vazio, célula isolada, bloco estável,
blinker, glider, zero gerações, `1 x 1`, bordas, matrizes retangulares,
comparação e leitura. O comando `make test-sequencial` executa a mesma suíte.
A comparação entre as versões sequencial e paralela é feita pelo próprio
programa quando a opção `--verificar` é informada.

Durante casos inválidos, avisos e erros aparecem propositalmente. A suíte
sequencial apresenta:

```text
14 testes executados: 14 aprovados e 0 falharam.
```

## Organização do código

- `main.c`: argumentos, fluxo da aplicação e integração dos módulos.
- `entrada.c` e `entrada.h`: leitura e validação do arquivo de entrada.
- `matriz.c` e `matriz.h`: alocação, liberação, troca, comparação e impressão.
- `logica.c` e `logica.h`: regras e simulação sequencial.
- `paralelo.c` e `paralelo.h`: simulação com pthreads por faixas de linhas.
- `renderizacao.c` e `renderizacao.h`: animação ANSI e exportação PBM.
- `tempo.c` e `tempo.h`: consulta ao relógio monotonicamente crescente.
- `testes/`: suíte automatizada e entradas usadas nos testes.
- `entradas/`: exemplos prontos para execução.
- `scripts/`: geração de entradas, benchmark e gráficos.
- `resultados/`: amostras de tempo, resumo estatístico, ambiente e gráficos.

## Limitações atuais

- O número de threads deve estar entre 1 e o número de linhas.
- A animação é exclusiva da versão sequencial.
- A animação depende de um terminal com suporte a sequências ANSI.
- O tabuleiro usa bordas mortas, sem comportamento toroidal.
- A imagem PBM representa apenas a geração final.
- A impressão e a animação não são adequadas para tabuleiros muito grandes.

## Módulo paralelo

`simular_paralelo` recebe as referências das duas matrizes, linhas, colunas,
gerações e número de threads, nessa ordem. Retorna 1 no sucesso e 0 em erro de
argumentos ou preparação. Aceita entre 1 thread e o número de linhas. As matrizes
devem ser distintas, ter as dimensões indicadas e não compartilhar suas linhas.
Ao terminar, as referências identificam a matriz final e a auxiliar, como na
simulação sequencial. Zero gerações preserva o estado sem criar threads.

Cada thread calcula uma faixa exclusiva de linhas durante todas as gerações.
Duas esperas em barreira por geração separam o cálculo da troca dos ponteiros.
Um mutex controla o início para permitir encerrar as threads já criadas se
alguma chamada a `pthread_create` falhar. Os nomes e RAs do grupo constam em
`main.c`; a divisão efetiva das atividades deve ser registrada pelo grupo.

Para verificar o resultado paralelo contra a versão sequencial:

```bash
./jogo_da_vida entradas/mistos.txt --threads 4 --verificar --sem-impressao
```

Os alvos normal e debug compilam e ligam com `-pthread`.

## Entradas maiores e experimentos

Para gerar uma entrada retangular reproduzível:

```bash
python3 scripts/gerar_entrada.py entradas/grande.txt --linhas 1000 --colunas 1200 --geracoes 100 --densidade 0.3 --semente 42
./jogo_da_vida entradas/grande.txt --threads 4 --verificar --sem-impressao
```

Para medir 3 tamanhos, cada um com 100 gerações, nas versões sequencial e
paralela com 1, 2, 4 e 8 threads, usando 5 repetições:

```bash
make benchmark
```

Os parâmetros podem ser personalizados:

```bash
make benchmark BENCH_ARGS="--tamanhos 256 768 --threads 1 2 4 --geracoes 80 --repeticoes 3 --saida resultados/outro_experimento"
```

Antes das medições, o script confirma a corretude de cada configuração.
Não há execuções extras de aquecimento. As repetições medidas não usam `--verificar`, e a
ordem dos modos é embaralhada de forma reproduzível a cada repetição. Cada
execução parte do mesmo arquivo. Rode sem outras cargas intensivas.

O diretório de saída recebe:

- `tempos.csv`: amostras individuais (segundos).
- `resumo.csv`: média, mediana, desvio amostral, extremos, speedup e eficiência.
- `ambiente.json`: sistema, processador, afinidade, parâmetros e hashes.
- `entradas/`: arquivos gerados; podem ser recriados pela semente e parâmetros.

Speedup = mediana sequencial / mediana paralela. Eficiência = speedup / número
de threads. O resultado com uma thread paralela permite observar o custo da
infraestrutura, mas pequenas diferenças também sofrem variação de medição.
Os resultados locais salvos não representam uma garantia para outras máquinas.

Os gráficos SVG podem ser regenerados com a dependência opcional ReportLab:

```bash
python3 -m pip install reportlab
python3 scripts/plotar_resultados.py
```

Python e ReportLab são ferramentas auxiliares: a simulação e a verificação do
executável são implementadas em C. O relatório técnico será preparado depois.
