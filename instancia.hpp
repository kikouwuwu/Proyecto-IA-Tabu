// instancia.hpp — Lectura de una instancia del problema.
//
// Una instancia es un .txt generado por `preprocesar` con este formato:
//
//   SEGMENTACION_MULTIUMBRAL_V1
//   imagen imagen_01.jpg
//   ancho 256
//   alto 256
//   L 256          (niveles de gris)
//   k 2            (cantidad de umbrales)
//   N 65536        (cantidad de pixeles)
//   histograma
//   h0 h1 ... h255 (cuantos pixeles tienen cada intensidad)
#pragma once

#include <string>
#include <vector>

struct Instancia {
    std::string imagen;              // Nombre de la imagen de origen.
    int ancho = 0, alto = 0;
    int L = 0;                       // Niveles de gris (256).
    int k = 0;                       // Umbrales a buscar.
    long long N = 0;                 // Total de pixeles.
    std::vector<long long> h;        // Histograma: h[g] = pixeles con intensidad g.

    // Datos derivados, calculados una sola vez al leer.
    // Sirven para evaluar cualquier solucion en O(k) (ver solucion.cpp).
    std::vector<double> P;           // P[g] = suma de p_x para x <= g   (proporcion acumulada)
    std::vector<double> S;           // S[g] = suma de x*p_x para x <= g (media acumulada)
    double muT = 0.0;                // Intensidad media de toda la imagen.
};

// Lee el archivo y valida cabecera, campos y que la suma del histograma sea N.
// Lanza std::runtime_error si algo no cuadra.
Instancia leerInstancia(const std::string& ruta);

// Muestra por consola un resumen de lo leido (para verificar la lectura).
void mostrarInstancia(const Instancia& ins);
