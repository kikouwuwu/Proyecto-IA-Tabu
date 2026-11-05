"""graficos.py — Figuras del informe a partir de los CSV de experimentos.sh.

Requiere Python 3 con matplotlib y Pillow. Uso (desde la carpeta del proyecto):
    python3 experimentos/graficos.py <carpeta_salida>
"""
import csv
import os
import statistics as st
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from PIL import Image

SALIDA = sys.argv[1] if len(sys.argv) > 1 else "figuras"
EXP = "experimentos"
INST = "imagenes_preprocesadas/instancias"
IMG = "imagenes_preprocesadas/imagenes"
os.makedirs(SALIDA, exist_ok=True)
plt.rcParams.update({"font.size": 9, "font.family": "serif", "axes.spines.top": False,
                     "axes.spines.right": False})
AZUL, NARANJO, GRIS = "#1f4e79", "#d9822b", "#7f7f7f"


def leer(nombre):
    with open(os.path.join(EXP, nombre)) as f:
        return list(csv.DictReader(f))


def histograma(n):
    texto = open(os.path.join(INST, f"imagen_{n:02d}_k2.txt")).read()
    return list(map(int, texto.split("histograma")[1].split()))


OPT = {(r["imagen"], r["k"]): (float(r["f_optimo"]), list(map(int, r["umbrales"].split())))
       for r in leer("optimos.csv")}

# 1) Representacion: histograma de la imagen 1 con los umbrales optimos para k = 3.
h = histograma(1)
t = [28, 72, 119]
fig, ax = plt.subplots(figsize=(5.6, 2.3))
colores = ["#dbe7f3", "#f6e3cf", "#e1efd9", "#efdcea"]
limites = [0] + [x + 1 for x in t]
for j, (a, b) in enumerate(zip(limites, t + [255])):
    ax.axvspan(a - 0.5, b + 0.5, color=colores[j], zorder=0)
    ax.text((a + b) / 2, 2600, f"$C_{j}$\n[{a}, {b}]", ha="center", va="top", fontsize=8)
ax.bar(range(256), h, width=1.0, color=AZUL, zorder=1)
for x, nombre in zip(t, ["$t_1$", "$t_2$", "$t_3$"]):
    ax.axvline(x + 0.5, color="black", lw=0.8, ls="--", zorder=2)
    ax.text(x + 2, 1650, f"{nombre} = {x}", fontsize=8)
ax.set_ylim(0, 2700)
ax.set_xlim(-1, 256)
ax.set_xlabel("Intensidad $g$")
ax.set_ylabel("Píxeles $h(g)$")
ax.text(165, 1100, f"$h(0) = {h[0]}$ (fondo)\nqueda fuera de escala", fontsize=7.5)
fig.tight_layout()
fig.savefig(os.path.join(SALIDA, "representacion.pdf"))
plt.close(fig)

# 2) Segmentacion: imagen 1 original y con los umbrales optimos para k = 2, 4 y 6.
original = Image.open(os.path.join(IMG, "imagen_01.jpg")).convert("L")
px = list(original.getdata())
fig, ejes = plt.subplots(1, 4, figsize=(6.2, 1.8))
ejes[0].imshow(original, cmap="gray", vmin=0, vmax=255)
ejes[0].set_title("Original", fontsize=8)
for ax, k in zip(ejes[1:], ["2", "4", "6"]):
    _, um = OPT[("imagen_01.jpg", k)]
    # Igual que mri::segmentar: clase = cantidad de umbrales menores que g; tono = 255*clase/k.
    tono = []
    for g in range(256):
        clase = sum(1 for x in um if x < g)
        tono.append(round(255 * clase / len(um)))
    seg = Image.new("L", original.size)
    seg.putdata([tono[g] for g in px])
    ax.imshow(seg, cmap="gray", vmin=0, vmax=255)
    ax.set_title(f"k = {k}: " + ", ".join(map(str, um)), fontsize=6.5)
for ax in ejes:
    ax.axis("off")
fig.tight_layout()
fig.savefig(os.path.join(SALIDA, "segmentacion.pdf"), dpi=200)
plt.close(fig)

# 3) Parametros: (a) iteracion de la mejor solucion segun el paso largo (t = 15),
#    (b) criterio de estancamiento: exito e iteraciones realizadas.
grilla = leer("grilla.csv")
pasos = [0, 5, 10, 20, 30]
datos = [[int(r["iter_mejor"]) for r in grilla if r["tenencia"] == "15" and int(r["paso"]) == p]
         for p in pasos]
estanc = leer("estancamiento.csv")
valores_e = [25, 50, 100, 200, 300, 0]


def brecha(r):
    o, _ = OPT[(r["imagen"], r["k"])]
    return 100 * (o - float(r["f_final"])) / o


exito_e = [100 * sum(1 for r in estanc if int(r["estancamiento"]) == e and brecha(r) < 1e-6)
           / sum(1 for r in estanc if int(r["estancamiento"]) == e) for e in valores_e]
iters_e = [st.mean(int(r["iter_hechas"]) for r in estanc if int(r["estancamiento"]) == e)
           for e in valores_e]
fig, (a1, a2) = plt.subplots(1, 2, figsize=(6.2, 2.4))
a1.boxplot(datos, labels=["0\n(solo ±1)", "5", "10", "20", "30"], showfliers=True,
           flierprops={"markersize": 2, "markerfacecolor": GRIS, "markeredgecolor": GRIS},
           medianprops={"color": NARANJO})
a1.set_xlabel("Paso largo $p$ (tenencia 15)")
a1.set_ylabel("Iteración de la mejor solución")
a1.set_title("(a) Rapidez según el paso largo", fontsize=9)
etiquetas = [str(e) if e else "sin\nlímite" for e in valores_e]
x = range(len(valores_e))
a2.bar(x, iters_e, color="#c9d9ea")
a2.set_ylabel("Iteraciones realizadas (prom.)")
a2.set_xticks(list(x))
a2.set_xticklabels(etiquetas)
a2.set_xlabel("Límite de estancamiento $e$")
a2b = a2.twinx()
a2b.plot(list(x), exito_e, color=NARANJO, marker="o", ms=3)
a2b.set_ylabel("Éxito (%)", color=NARANJO)
a2b.set_ylim(94, 100.5)
a2b.spines["top"].set_visible(False)
a2.set_title("(b) Criterio de estancamiento", fontsize=9)
fig.tight_layout()
fig.savefig(os.path.join(SALIDA, "parametros.pdf"))
plt.close(fig)

# 4) Avance: brecha de la mejor solucion por iteracion (mediana y rango de 10 semillas).
fig, ejes = plt.subplots(1, 2, figsize=(6.2, 2.4), sharey=True)
for ax, n in zip(ejes, ["imagen_01_k6", "imagen_10_k6"]):
    imagen = n[:9] + ".jpg"
    o, _ = OPT[(imagen, "6")]
    for p, color, nombre in [(0, GRIS, "solo ±1"), (20, AZUL, "±1 y ±20")]:
        curvas = []
        for s in range(1, 11):
            with open(os.path.join(EXP, "avance", f"{n}_p{p}_s{s}.csv")) as f:
                curvas.append([100 * (o - float(r["f_mejor"])) / o for r in csv.DictReader(f)])
        largo = min(len(c) for c in curvas)
        its = list(range(largo))
        med = [st.median(c[i] for c in curvas) for i in its]
        ax.fill_between(its, [min(c[i] for c in curvas) for i in its],
                        [max(c[i] for c in curvas) for i in its], color=color, alpha=0.2, lw=0)
        ax.plot(its, med, color=color, lw=1.2, label=nombre)
    ax.set_yscale("symlog", linthresh=0.01)
    ax.set_title(f"{n[:9]}, k = 6", fontsize=9)
    ax.set_xlabel("Iteración")
ejes[0].set_ylabel("Brecha al óptimo (%)")
ejes[0].legend(frameon=False, fontsize=8)
fig.tight_layout()
fig.savefig(os.path.join(SALIDA, "avance.pdf"))
plt.close(fig)
print("Figuras en", SALIDA)
