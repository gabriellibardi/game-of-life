"""Mede o executavel C e salva amostras, estatisticas e ambiente em CSV/JSON."""
import argparse
import csv
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import platform
import random
import re
import statistics
import subprocess
from gerar_entrada import gerar, positivo, densidade_valida


def executar(comando, verificar=False):
    """Rejeita erros, ausencia de tempo e verificacao nao confirmada."""
    processo = subprocess.run(comando, capture_output=True, text=True, timeout=300)
    if processo.returncode:
        raise RuntimeError(f"Falha em {comando!r}:\n{processo.stdout}\n{processo.stderr}")
    if verificar and "Verificacao: OK" not in processo.stdout:
        raise RuntimeError("O executavel nao confirmou a igualdade dos resultados.")
    medicao = re.search(r"Tempo da simulacao (?:sequencial|paralela): ([0-9.]+) s", processo.stdout)
    if medicao is None or float(medicao.group(1)) <= 0:
        raise RuntimeError("Tempo ausente ou nulo: aumente o tamanho ou as geracoes.")
    return float(medicao.group(1))


def ler_opcional(caminho):
    try:
        return Path(caminho).read_text().strip()
    except OSError:
        return None


def salvar_csv(caminho, registros):
    with caminho.open("w", newline="", encoding="utf-8") as arquivo:
        escritor = csv.DictWriter(arquivo, fieldnames=list(registros[0]))
        escritor.writeheader()
        escritor.writerows(registros)


def medir(opcoes):
    """Verifica cada configuracao e intercala a ordem das medicoes."""
    executavel = opcoes.executavel.resolve()
    saida = opcoes.saida.resolve()
    saida.mkdir(parents=True, exist_ok=True)
    metadados = {
        "inicio_utc": datetime.now(timezone.utc).isoformat(),
        "sistema": platform.platform(), "python": platform.python_version(),
        "cpu": next((l.split(":", 1)[1].strip() for l in
                     (ler_opcional("/proc/cpuinfo") or "").splitlines()
                     if l.startswith("model name")), platform.processor()),
        "cpus_logicas": os.cpu_count(),
        "afinidade": sorted(os.sched_getaffinity(0)) if hasattr(os, "sched_getaffinity") else None,
        "cgroup_cpu_max": ler_opcional("/sys/fs/cgroup/cpu.max"),
        "executavel_sha256": hashlib.sha256(executavel.read_bytes()).hexdigest(),
        "parametros": {k: str(v) if isinstance(v, Path) else v for k, v in vars(opcoes).items()},
        "medicao": "CLOCK_MONOTONIC no C; inclui equipe pthreads; exclui E/S e verificacao",
        "entradas": [],
    }
    configuracoes = [("sequencial", 1)] + [("paralelo", n) for n in sorted(set(opcoes.threads))]
    amostras = []
    for tamanho in sorted(set(opcoes.tamanhos)):
        entrada = saida / "entradas" / f"tabuleiro_{tamanho}.txt"
        vivas = gerar(entrada, tamanho, tamanho, opcoes.geracoes, opcoes.densidade, opcoes.semente)
        metadados["entradas"].append({"arquivo": str(entrada.relative_to(saida)), "vivas": vivas,
            "sha256": hashlib.sha256(entrada.read_bytes()).hexdigest()})
        comandos = {}
        for modo, threads in configuracoes:
            comando = [str(executavel), str(entrada), "--sem-impressao"]
            comando += ["--sequencial"] if modo == "sequencial" else ["--threads", str(threads)]
            comandos[(modo, threads)] = comando
            executar(comando + ["--verificar"], verificar=True)
        for repeticao in range(1, opcoes.repeticoes + 1):
            ordem = configuracoes.copy()
            random.Random(opcoes.semente + tamanho + repeticao).shuffle(ordem)
            for modo, threads in ordem:
                tempo = executar(comandos[(modo, threads)])
                amostras.append(dict(linhas=tamanho, colunas=tamanho, geracoes=opcoes.geracoes,
                                     modo=modo, threads=threads, repeticao=repeticao, tempo_s=tempo))
                salvar_csv(saida / "tempos.csv", amostras)
        print(f"{tamanho} x {tamanho}: corretude confirmada e medicoes concluidas.", flush=True)
    resumo = []
    for tamanho in sorted(set(opcoes.tamanhos)):
        base = statistics.median(r["tempo_s"] for r in amostras
                                 if r["linhas"] == tamanho and r["modo"] == "sequencial")
        for modo, threads in configuracoes:
            tempos = [r["tempo_s"] for r in amostras
                      if r["linhas"] == tamanho and r["modo"] == modo and r["threads"] == threads]
            mediana = statistics.median(tempos)
            speedup = base / mediana
            resumo.append(dict(linhas=tamanho, colunas=tamanho, geracoes=opcoes.geracoes,
                               modo=modo, threads=threads, amostras=len(tempos), mediana_s=mediana,
                               media_s=statistics.mean(tempos),
                               desvio_s=statistics.stdev(tempos) if len(tempos) > 1 else 0,
                               minimo_s=min(tempos), maximo_s=max(tempos),
                               speedup=speedup, eficiencia=speedup / threads))
    salvar_csv(saida / "resumo.csv", resumo)
    metadados["fim_utc"] = datetime.now(timezone.utc).isoformat()
    (saida / "ambiente.json").write_text(json.dumps(metadados, indent=2, ensure_ascii=False) + "\n",
                                         encoding="utf-8")
    print(f"Resultados em {saida}.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executavel", type=Path, default=Path("jogo_da_vida"))
    parser.add_argument("--saida", type=Path, default=Path("resultados"))
    parser.add_argument("--tamanhos", nargs="+", type=positivo, default=[128, 512, 1024])
    parser.add_argument("--threads", nargs="+", type=positivo, default=[1, 2, 4, 8])
    parser.add_argument("--geracoes", type=positivo, default=100)
    parser.add_argument("--repeticoes", type=positivo, default=5)
    parser.add_argument("--densidade", type=densidade_valida, default=0.3)
    parser.add_argument("--semente", type=int, default=42)
    medir(parser.parse_args())
