"""Gera entradas reproduziveis para o Jogo da Vida."""
import argparse
from pathlib import Path
import random


def positivo(texto):
    valor = int(texto)
    if not 1 <= valor <= 2147483647:
        raise argparse.ArgumentTypeError("informe um inteiro entre 1 e 2147483647")
    return valor


def nao_negativo(texto):
    valor = int(texto)
    if not 0 <= valor <= 2147483647:
        raise argparse.ArgumentTypeError("informe um inteiro entre 0 e 2147483647")
    return valor


def densidade_valida(texto):
    valor = float(texto)
    if not 0 <= valor <= 1:
        raise argparse.ArgumentTypeError("densidade deve estar entre 0 e 1")
    return valor


def gerar(caminho, linhas, colunas, geracoes, densidade, semente):
    """Sorteia cada celula com gerador local e semente fixa."""
    gerador = random.Random(semente)
    caminho = Path(caminho)
    caminho.parent.mkdir(parents=True, exist_ok=True)
    vivas = 0
    with caminho.open("w", encoding="utf-8", newline="\n") as arquivo:
        arquivo.write(f"{linhas} {colunas} {geracoes}\n")
        for linha in range(linhas):
            for coluna in range(colunas):
                if gerador.random() < densidade:
                    arquivo.write(f"{linha} {coluna}\n")
                    vivas += 1
    return vivas


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("saida", type=Path)
    parser.add_argument("--linhas", type=positivo, required=True)
    parser.add_argument("--colunas", type=positivo, required=True)
    parser.add_argument("--geracoes", type=nao_negativo, default=100)
    parser.add_argument("--densidade", type=densidade_valida, default=0.3)
    parser.add_argument("--semente", type=int, default=42)
    opcoes = parser.parse_args()
    vivas = gerar(opcoes.saida, opcoes.linhas, opcoes.colunas,
                  opcoes.geracoes, opcoes.densidade, opcoes.semente)
    print(f"Entrada salva em {opcoes.saida}: {vivas} celulas vivas.")
