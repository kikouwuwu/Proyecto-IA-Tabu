// tabu.hpp — Busqueda Tabu sobre los umbrales.
//
// MOVIMIENTOS (sobre un solo umbral ti):
//   M1: ti + 1, ti - 1    -> ajuste fino
//   M2: ti + 10, ti - 10  -> salto, para no quedar atrapado cerca del inicio
//   Vecinos que rompen R1 o R2 se descartan con esFactible().
//
// LISTA TABU
//   Al mover ti desde un valor v, se prohibe que ti vuelva a v durante
//   'tenencia' iteraciones. Se guarda como pares (indice, valor).
//
// ASPIRACION
//   Un vecino tabu se acepta igual si supera a la mejor solucion vista.
#pragma once

#include "instancia.hpp"
#include "solucion.hpp"

struct ResultadoTabu {
    Solucion mejor;          // Mejor solucion encontrada.
    double fMejor = 0.0;
    int iteracionMejor = 0;  // Iteracion en que se encontro (0 = la inicial).
};

ResultadoTabu busquedaTabu(const Solucion& inicial, const Instancia& ins,
                           int tenencia, int iteraciones);
