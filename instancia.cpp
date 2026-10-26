// instancia.cpp — Lectura y validacion de instancias.
#include "instancia.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace std;

// Lee "clave valor" y comprueba que la clave sea la esperada.
template <typename T>
static T leerCampo(ifstream& in, const string& esperada) {
    string clave;
    T valor;
    if (!(in >> clave >> valor) || clave != esperada)
        throw runtime_error("Se esperaba el campo '" + esperada + "'");
    return valor;
}

Instancia leerInstancia(const string& ruta) {
    ifstream in(ruta);
    if (!in) throw runtime_error("No se pudo abrir: " + ruta);

    string cabecera;
    in >> cabecera;
    if (cabecera != "SEGMENTACION_MULTIUMBRAL_V1")
        throw runtime_error("Cabecera invalida en " + ruta);

    // 1) Campos de la cabecera, en el orden del formato.
    Instancia ins;
    ins.imagen = leerCampo<string>(in, "imagen");
    ins.ancho  = leerCampo<int>(in, "ancho");
    ins.alto   = leerCampo<int>(in, "alto");
    ins.L      = leerCampo<int>(in, "L");
    ins.k      = leerCampo<int>(in, "k");
    ins.N      = leerCampo<long long>(in, "N");

    string marca;
    if (!(in >> marca) || marca != "histograma")
        throw runtime_error("Falta la marca 'histograma'");

    // 2) Las L frecuencias del histograma.
    ins.h.assign(ins.L, 0);
    long long suma = 0;
    for (int g = 0; g < ins.L; ++g) {
        if (!(in >> ins.h[g]) || ins.h[g] < 0)
            throw runtime_error("Histograma incompleto o negativo en g=" + to_string(g));
        suma += ins.h[g];
    }

    // 3) Validaciones de consistencia.
    if (suma != ins.N) throw runtime_error("La suma del histograma no es N");
    if (ins.N != 1LL * ins.ancho * ins.alto) throw runtime_error("N != ancho*alto");
    if (ins.k < 1 || ins.k > ins.L - 2) throw runtime_error("k fuera de rango");

    // 4) Sumas acumuladas: p_g = h[g]/N, P[g] = sum p_x, S[g] = sum x*p_x.
    ins.P.assign(ins.L, 0.0);
    ins.S.assign(ins.L, 0.0);
    double accP = 0.0, accS = 0.0;
    for (int g = 0; g < ins.L; ++g) {
        double p = double(ins.h[g]) / ins.N;
        accP += p;
        accS += g * p;
        ins.P[g] = accP;
        ins.S[g] = accS;
    }
    ins.muT = ins.S[ins.L - 1];
    return ins;
}

void mostrarInstancia(const Instancia& ins) {
    int gMin = 0, gMax = ins.L - 1;
    while (ins.h[gMin] == 0) ++gMin;
    while (ins.h[gMax] == 0) --gMax;
    cout << "Instancia: " << ins.imagen << "  (" << ins.ancho << "x" << ins.alto
         << ", L=" << ins.L << ", k=" << ins.k << ", N=" << ins.N << ")\n"
         << "  Histograma OK: suma = N\n"
         << "  Intensidades presentes: " << gMin << " a " << gMax << "\n"
         << "  Pixeles en intensidad 0 (fondo): " << ins.h[0]
         << " (" << 100.0 * ins.h[0] / ins.N << " %)\n"
         << "  Media global muT = " << ins.muT << "\n";
}
