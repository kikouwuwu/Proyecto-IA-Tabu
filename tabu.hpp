// tabu.hpp — Busqueda Tabu sobre los umbrales.
//
// MOVIMIENTOS (sobre un solo umbral ti):
//   M1: ti + 1, ti - 1    -> ajuste fino (intensificacion)
//   M2: ti + p, ti - p    -> salto largo (diversificacion); p = pasoLargo, 20 por defecto
//   Vecinos que rompen R1 o R2 se descartan con esFactible().
//
// LISTA TABU
//   Al mover ti desde un valor v, se prohibe que ti vuelva a v durante
//   'tenencia' iteraciones. Se guarda como pares (indice, valor).
//
// ASPIRACION
//   Un vecino tabu se acepta igual si supera a la mejor solucion vista.
//
// TERMINO
//   Se detiene al llegar a 'iteraciones' o tras 'estancamiento' iteraciones
//   seguidas sin mejorar la mejor solucion (0 = sin este criterio).
#pragma once

#include "instancia.hpp"
#include "solucion.hpp"

#include <ostream>

struct ParametrosTabu {
    int tenencia = 15;        // Iteraciones que dura una prohibicion.
    int pasoLargo = 20;       // Tamano del movimiento M2 (0 = solo M1).
    int iteraciones = 1000;   // Maximo de iteraciones.
    int estancamiento = 200;  // Iteraciones sin mejora para detenerse (0 = no se usa).
};

struct ResultadoTabu {
    Solucion mejor;              // Mejor solucion encontrada.
    double fMejor = 0.0;
    int iteracionMejor = 0;      // Iteracion en que se encontro (0 = la inicial).
    int iteracionesHechas = 0;   // Iteraciones realizadas antes de detenerse.
};

// Si 'avance' no es nulo, escribe una linea "iteracion,f_actual,f_mejor" por iteracion.
ResultadoTabu busquedaTabu(const Solucion& inicial, const Instancia& ins,
                           const ParametrosTabu& par, std::ostream* avance = nullptr);
