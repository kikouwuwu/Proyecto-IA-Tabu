// solucion.hpp — Representacion, restricciones, evaluacion y solucion inicial.
//
// REPRESENTACION
//   Una solucion es un vector de k enteros: t = (t1, t2, ..., tk).
//   Clases (igual que mri::segmentar del codigo base):
//     C0 = [0, t1], C1 = [t1+1, t2], ..., Ck = [tk+1, 255]
//   Un pixel igual a un umbral queda en la clase inferior.
//
// RESTRICCIONES
//   (R1) 1 <= ti <= 254       -> cada umbral es una intensidad valida
//   (R2) t1 < t2 < ... < tk   -> ordenados y distintos (sin clases repetidas)
//   Se manejan por construccion: toda solucion se genera ya ordenada y en
//   rango, y esFactible() rechaza cualquier vecino que las rompa.
//
// FUNCION DE EVALUACION (Otsu, maximizar varianza entre clases)
//   f(t) = sum_j  w_j * (mu_j - muT)^2
//   w_j  = proporcion de pixeles en la clase j
//   mu_j = intensidad media de la clase j
#pragma once

#include "instancia.hpp"

#include <string>
#include <vector>

using Solucion = std::vector<int>;

bool esFactible(const Solucion& t, const Instancia& ins);

// Evalua en O(k) usando las sumas acumuladas P y S de la instancia.
double evaluar(const Solucion& t, const Instancia& ins);

// Greedy: agrega un umbral a la vez, eligiendo el valor que mas sube f.
Solucion solucionGreedy(const Instancia& ins);

// Aleatoria: k valores distintos en [1, 254], ordenados. Reproducible por semilla.
Solucion solucionAleatoria(const Instancia& ins, unsigned semilla);

// Escribe umbrales, f y una tabla por clase. Si ruta es "-", escribe en consola.
void escribirSolucion(const Solucion& t, const Instancia& ins,
                      const std::string& metodo, double tiempoMs,
                      const std::string& ruta);
