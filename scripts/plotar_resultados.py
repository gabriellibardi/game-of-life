"""Gera graficos SVG do benchmark com ReportLab (dependencia opcional)."""
import argparse
import csv
from pathlib import Path

from reportlab.graphics import renderSVG
from reportlab.graphics.charts.barcharts import VerticalBarChart
from reportlab.graphics.charts.legends import Legend
from reportlab.graphics.charts.lineplots import LinePlot
from reportlab.graphics.shapes import Drawing, Rect, String
from reportlab.lib.colors import HexColor

CORES = [HexColor(c) for c in ("#235789", "#e07a35", "#32936f", "#8157a5", "#c34f64")]


def base(titulo, subtitulo, eixo_y):
    desenho = Drawing(760, 410)
    desenho.add(Rect(0, 0, 760, 410, fillColor=HexColor("#ffffff"), strokeColor=None))
    desenho.add(String(65, 383, titulo, fontName="Helvetica-Bold", fontSize=17))
    desenho.add(String(65, 361, subtitulo, fontSize=10, fillColor=HexColor("#555555")))
    desenho.add(String(14, 220, eixo_y, angle=90, fontSize=11))
    desenho.add(String(65, 12, "Fonte: resumo.csv; mediana das repeticoes. E/S e verificacao fora do tempo.",
                       fontSize=9, fillColor=HexColor("#555555")))
    return desenho


def plotar(entrada, saida):
    """Compara tempos por tamanho e speedup por quantidade de threads."""
    with entrada.open(encoding="utf-8", newline="") as arquivo:
        registros = list(csv.DictReader(arquivo))
    saida.mkdir(parents=True, exist_ok=True)
    tamanhos = sorted({(int(r["linhas"]), int(r["colunas"])) for r in registros})
    for linhas, colunas in tamanhos:
        grupo = [r for r in registros if (int(r["linhas"]), int(r["colunas"])) == (linhas, colunas)]
        grupo.sort(key=lambda r: (r["modo"] != "sequencial", int(r["threads"])))
        desenho = base(f"Tempo de execucao: {linhas} x {colunas}",
                       f"{grupo[0]['geracoes']} geracoes; {grupo[0]['amostras']} repeticoes por configuracao",
                       "Tempo (segundos)")
        grafico = VerticalBarChart()
        grafico.x, grafico.y, grafico.width, grafico.height = 75, 65, 610, 265
        grafico.data = [[float(r["mediana_s"]) for r in grupo]]
        grafico.categoryAxis.categoryNames = ["Sequencial" if r["modo"] == "sequencial"
                                              else f"{r['threads']} thread(s)" for r in grupo]
        grafico.categoryAxis.labels.fontSize = 10
        grafico.valueAxis.valueMin = 0
        grafico.valueAxis.valueMax = max(grafico.data[0]) * 1.2
        grafico.valueAxis.valueStep = grafico.valueAxis.valueMax / 5
        grafico.valueAxis.labelTextFormat = "%.3f"
        grafico.bars[0].fillColor = CORES[0]
        grafico.bars[0].strokeColor = None
        desenho.add(grafico)
        renderSVG.drawToFile(desenho, str(saida / f"tempos_{linhas}x{colunas}.svg"))

    desenho = base("Speedup da execucao paralela", "Speedup = tempo sequencial / tempo paralelo",
                   "Speedup")
    grafico = LinePlot()
    grafico.x, grafico.y, grafico.width, grafico.height = 75, 70, 500, 260
    grafico.data = []
    legenda = Legend()
    legenda.x, legenda.y = 600, 325
    legenda.fontSize = 10
    legenda.colorNamePairs = []
    for indice, (linhas, colunas) in enumerate(tamanhos):
        grupo = sorted((r for r in registros if r["modo"] == "paralelo"
                        and (int(r["linhas"]), int(r["colunas"])) == (linhas, colunas)),
                       key=lambda r: int(r["threads"]))
        grafico.data.append([(int(r["threads"]), float(r["speedup"])) for r in grupo])
        grafico.lines[indice].strokeColor = CORES[indice % len(CORES)]
        grafico.lines[indice].strokeWidth = 2
        legenda.colorNamePairs.append((CORES[indice % len(CORES)], f"{linhas} x {colunas}"))
    threads = sorted({int(r["threads"]) for r in registros if r["modo"] == "paralelo"})
    grafico.xValueAxis.valueMin = 0
    grafico.xValueAxis.valueMax = max(threads) + 0.5
    grafico.xValueAxis.valueSteps = threads
    grafico.yValueAxis.valueMin = 0
    desenho.add(grafico)
    desenho.add(legenda)
    desenho.add(String(260, 38, "Numero de threads", fontSize=11))
    renderSVG.drawToFile(desenho, str(saida / "speedup.svg"))
    print(f"Graficos SVG salvos em {saida}.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--entrada", type=Path, default=Path("resultados/resumo.csv"))
    parser.add_argument("--saida", type=Path, default=Path("resultados/graficos"))
    opcoes = parser.parse_args()
    plotar(opcoes.entrada, opcoes.saida)
