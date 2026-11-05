"""analisis.py — Resume los CSV de experimentos.sh (solo biblioteca estandar de Python 3).

Uso (desde la carpeta del proyecto):  python3 experimentos/analisis.py

Indicadores por configuracion, sobre 30 instancias x 30 semillas = 900 ejecuciones:
  exito  = % de ejecuciones que terminan en el optimo exacto (f igual al de ./optimo)
  brecha = 100 * (f_optimo - f_final) / f_optimo, en %
"""
import csv
import os
import statistics as st

CARPETA = "experimentos"


def leer(nombre):
    with open(os.path.join(CARPETA, nombre)) as f:
        return list(csv.DictReader(f))


OPT = {(r["imagen"], r["k"]): float(r["f_optimo"]) for r in leer("optimos.csv")}


def brecha(r):
    o = OPT[(r["imagen"], r["k"])]
    return 100 * (o - float(r["f_final"])) / o


def exito(filas):
    return 100 * sum(1 for r in filas if brecha(r) < 1e-6) / len(filas)


def agrupar(filas, *claves):
    g = {}
    for r in filas:
        g.setdefault(tuple(int(r[c]) for c in claves), []).append(r)
    return dict(sorted(g.items()))


def resumen(filas):
    b = [brecha(r) for r in filas]
    return (exito(filas), st.mean(b), max(b), st.mean(int(r["iter_mejor"]) for r in filas),
            st.mean(int(r["iter_hechas"]) for r in filas), st.mean(float(r["tiempo_ms"]) for r in filas))


print("E1 - % de exito por tenencia (filas) y paso largo (columnas)")
grilla = agrupar(leer("grilla.csv"), "tenencia", "paso")
pasos = sorted({p for _, p in grilla})
print("t \\ p " + "".join(f"{p:>8}" for p in pasos))
for t in sorted({t for t, _ in grilla}):
    print(f"{t:>5} " + "".join(f"{exito(grilla[(t, p)]):8.1f}" for p in pasos))

print("\nE1 - iteracion promedio en que aparece la mejor solucion")
for t in sorted({t for t, _ in grilla}):
    print(f"{t:>5} " + "".join(f"{st.mean(int(r['iter_mejor']) for r in grilla[(t, p)]):8.1f}" for p in pasos))

print("\nE2 - criterio de estancamiento")
print("estanc.  exito%  brecha_prom%  brecha_max%  iter_mejor  iter_hechas  ms")
for (e,), filas in agrupar(leer("estancamiento.csv"), "estancamiento").items():
    ex, bp, bm, im, ih, ms = resumen(filas)
    print(f"{e:>7} {ex:7.2f} {bp:13.5f} {bm:12.4f} {im:11.1f} {ih:12.1f} {ms:6.3f}")

print("\nE3 - configuracion final por k")
final = leer("final.csv")
print("k  exito%  brecha_prom%  brecha_max%  f_inicial_prom  f_final_prom  ms_prom  ms_max")
for (k,), filas in agrupar(final, "k").items():
    ex, bp, bm, im, ih, ms = resumen(filas)
    print(f"{k}  {ex:6.2f} {bp:13.5f} {bm:12.4f} {st.mean(float(r['f_inicial']) for r in filas):15.2f} "
          f"{st.mean(float(r['f_final']) for r in filas):13.2f} {ms:8.3f} {max(float(r['tiempo_ms']) for r in filas):7.3f}")

print("\nE3 - por instancia: optimo, f final (promedio y desviacion), exito")
for (im, k), filas in sorted({(r["imagen"], r["k"]): None for r in final}.items()):
    fs = [float(r["f_final"]) for r in final if r["imagen"] == im and r["k"] == k]
    sub = [r for r in final if r["imagen"] == im and r["k"] == k]
    print(f"{im} k={k}  opt={OPT[(im, k)]:10.4f}  prom={st.mean(fs):10.4f}  desv={st.pstdev(fs):7.4f}  "
          f"min={min(fs):10.4f}  exito={exito(sub):5.1f}%")
