// solucion.cpp — Implementacion de lo declarado en solucion.hpp.
#include "solucion.hpp"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>

using namespace std;

bool esFactible(const Solucion& t, const Instancia& ins) {
    if ((int)t.size() != ins.k) return false;
    for (int i = 0; i < ins.k; ++i) {
        if (t[i] < 1 || t[i] > ins.L - 2) return false;      // R1
        if (i > 0 && t[i] <= t[i - 1]) return false;         // R2
    }
    return true;
}

// Aporte de la clase [a, b]: w * mu^2 = (suma x*p_x)^2 / (suma p_x).
// Las sumas del intervalo salen de restar acumulados: O(1).
static double aporteClase(int a, int b, const Instancia& ins) {
    double w = ins.P[b] - (a > 0 ? ins.P[a - 1] : 0.0);
    double s = ins.S[b] - (a > 0 ? ins.S[a - 1] : 0.0);
    return w > 0 ? s * s / w : 0.0;   // clase vacia: aporte 0
}

// Usa la forma equivalente f = sum_j w_j*mu_j^2 - muT^2 (mismo valor, menos calculos).
// Recibe umbrales ordenados; no exige que sean exactamente k (lo usa el greedy).
static double evaluarParcial(const Solucion& t, const Instancia& ins) {
    double suma = 0.0;
    int desde = 0;
    for (int ti : t) {
        suma += aporteClase(desde, ti, ins);
        desde = ti + 1;
    }
    suma += aporteClase(desde, ins.L - 1, ins);
    return suma - ins.muT * ins.muT;
}

double evaluar(const Solucion& t, const Instancia& ins) {
    if (!esFactible(t, ins)) throw invalid_argument("Solucion no factible");
    return evaluarParcial(t, ins);
}

Solucion solucionGreedy(const Instancia& ins) {
    Solucion t;
    for (int paso = 0; paso < ins.k; ++paso) {
        int mejorValor = -1;
        double mejorF = -1.0;
        // Prueba cada intensidad libre como nuevo umbral y se queda con la mejor.
        for (int g = 1; g <= ins.L - 2; ++g) {
            if (find(t.begin(), t.end(), g) != t.end()) continue;
            Solucion candidata = t;
            candidata.insert(upper_bound(candidata.begin(), candidata.end(), g), g);
            double f = evaluarParcial(candidata, ins);
            if (f > mejorF) { mejorF = f; mejorValor = g; }
        }
        t.insert(upper_bound(t.begin(), t.end(), mejorValor), mejorValor);
    }
    return t;   // ya ordenada y en rango: factible por construccion
}

Solucion solucionAleatoria(const Instancia& ins, unsigned semilla) {
    vector<int> valores;
    for (int g = 1; g <= ins.L - 2; ++g) valores.push_back(g);
    mt19937 rng(semilla);
    shuffle(valores.begin(), valores.end(), rng);
    Solucion t(valores.begin(), valores.begin() + ins.k);  // k distintos
    sort(t.begin(), t.end());                              // ordenados
    return t;
}

void escribirSolucion(const Solucion& t, const Instancia& ins,
                      const string& metodo, double tiempoMs, const string& ruta) {
    ofstream archivo;
    if (ruta != "-") {
        archivo.open(ruta);
        if (!archivo) throw runtime_error("No se pudo escribir: " + ruta);
    }
    ostream& out = (ruta == "-") ? cout : archivo;

    out << fixed;
    out << "instancia " << ins.imagen << " k=" << ins.k << "\n";
    out << "metodo " << metodo << "\n";
    out << "umbrales";
    for (int ti : t) out << ' ' << ti;
    out << "\nf " << setprecision(6) << evaluar(t, ins) << "\n";
    out << "tiempo_ms " << setprecision(4) << tiempoMs << "\n";
    out << "# clase desde hasta pixeles proporcion media\n";

    int desde = 0;
    for (int j = 0; j <= ins.k; ++j) {
        int hasta = (j < ins.k) ? t[j] : ins.L - 1;
        long long px = 0;
        double sumaG = 0;
        for (int g = desde; g <= hasta; ++g) { px += ins.h[g]; sumaG += double(g) * ins.h[g]; }
        out << j << ' ' << desde << ' ' << hasta << ' ' << px << ' '
            << setprecision(6) << double(px) / ins.N << ' '
            << setprecision(3) << (px ? sumaG / px : 0.0) << "\n";
        desde = hasta + 1;
    }
}
