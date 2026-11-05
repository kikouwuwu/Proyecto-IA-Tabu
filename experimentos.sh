#!/bin/sh
# experimentos.sh — Experimentos del informe final. Se ejecuta con `make experimentos`.
#
# Cada ejecucion de ./tabu con -c entrega una linea CSV; aqui solo se repite
# sobre las 30 instancias y 30 semillas (1..30) y se guarda todo en experimentos/.
#
#   optimos.csv        optimo exacto de cada instancia (./optimo), para medir la brecha
#   grilla.csv         E1: tenencia x paso largo, 1000 iteraciones sin estancamiento
#   estancamiento.csv  E2: limite de iteraciones sin mejora, con la configuracion elegida
#   final.csv          E3: configuracion final (la de por defecto en ./tabu)
#   avance/            E4: f mejor por iteracion en dos instancias, con y sin salto largo
#
# Tiempo aproximado: 2 a 3 minutos.

INST=imagenes_preprocesadas/instancias
OUT=experimentos
SEMILLAS=$(seq 1 30)
CAB="imagen,k,semilla,tenencia,paso,iteraciones,estancamiento,f_inicial,f_final,iter_mejor,iter_hechas,tiempo_ms,umbrales"

# Configuracion final (elegida con E1 y E2; son los valores por defecto de ./tabu).
T_FINAL=15
P_FINAL=20
E_FINAL=200

mkdir -p $OUT/avance

echo "imagen,k,f_optimo,umbrales" > $OUT/optimos.csv
for f in $INST/*.txt; do ./optimo $f -c >> $OUT/optimos.csv || exit 1; done
echo "optimos listos"

# E1: efecto de la tenencia (t = 0 es una busqueda local sin memoria) y del salto largo (p = 0 es solo +-1).
echo $CAB > $OUT/grilla.csv
for p in 0 5 10 20 30; do
  for t in 0 5 10 15 20 30 40; do
    for f in $INST/*.txt; do
      for s in $SEMILLAS; do
        ./tabu $f -c -s $s -t $t -p $p -i 1000 -e 0 >> $OUT/grilla.csv || exit 1
      done
    done
  done
  echo "E1: paso largo $p listo"
done

# E2: criterio de estancamiento (0 = siempre 1000 iteraciones).
echo $CAB > $OUT/estancamiento.csv
for e in 25 50 100 200 300 0; do
  for f in $INST/*.txt; do
    for s in $SEMILLAS; do
      ./tabu $f -c -s $s -t $T_FINAL -p $P_FINAL -i 1000 -e $e >> $OUT/estancamiento.csv || exit 1
    done
  done
done
echo "E2 listo"

# E3: configuracion final.
echo $CAB > $OUT/final.csv
for f in $INST/*.txt; do
  for s in $SEMILLAS; do
    ./tabu $f -c -s $s -t $T_FINAL -p $P_FINAL -i 1000 -e $E_FINAL >> $OUT/final.csv || exit 1
  done
done
echo "E3 listo"

# E4: avance de la busqueda (300 iteraciones, 10 semillas), con y sin el salto largo.
for n in imagen_01_k6 imagen_10_k6; do
  for p in 0 $P_FINAL; do
    for s in $(seq 1 10); do
      ./tabu $INST/$n.txt -c -s $s -t $T_FINAL -p $p -i 300 -e 0 \
        -a $OUT/avance/${n}_p${p}_s${s}.csv > /dev/null || exit 1
    done
  done
done
echo "E4 listo. Resultados en $OUT/"
