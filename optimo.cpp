// optimo.cpp — Optimo exacto por programacion dinamica (solo como referencia).
//
// No forma parte de la Busqueda Tabu: sirve para medir que tan cerca llega
// Tabu en los experimentos. Como f es una suma de aportes independientes por
// clase, el mejor valor de las clases 0..j cuando la clase j termina en b es
//   F[j][b] = max_{a < b} ( F[j-1][a] + aporte(a+1, b) )
// Costo O(k * L^2), menos de un segundo para k <= 6.
//
// Uso:
//   ./optimo <instancia.txt> [-k K] [-c]
//   -c  imprime una sola linea CSV: imagen,k,f_optimo,umbrales
#include "instancia.hpp"
#include "solucion.hpp"

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <instancia.txt> [-k K] [-c]\n";
        return 1;
    }
    try {
        string ruta = argv[1];
        int k = -1;
        bool modoCsv = false;
        for (int i = 2; i < argc; ++i) {
            string op = argv[i];
            if (op == "-c") modoCsv = true;
            else if (op == "-k" && i + 1 < argc) k = stoi(argv[++i]);
            else throw invalid_argument("Opcion desconocida: " + op);
        }
        Instancia ins = leerInstancia(ruta);
        if (k != -1) {
            if (k < 1 || k > ins.L - 2) throw invalid_argument("k fuera de rango");
            ins.k = k;
        }
        const int L = ins.L;
        const double MENOS_INF = -1e300;

        // F[j][b]: mejor suma de aportes de las clases 0..j si la clase j termina en b.
        // previo[j][b]: donde termina la clase j-1 en esa mejor opcion.
        vector<vector<double>> F(ins.k, vector<double>(L, MENOS_INF));
        vector<vector<int>> previo(ins.k, vector<int>(L, -1));
        for (int b = 1; b <= L - 2; ++b) F[0][b] = aporteClase(0, b, ins);
        for (int j = 1; j < ins.k; ++j)
            for (int b = j + 1; b <= L - 2; ++b)
                for (int a = j; a < b; ++a) {
                    if (F[j - 1][a] == MENOS_INF) continue;
                    double v = F[j - 1][a] + aporteClase(a + 1, b, ins);
                    if (v > F[j][b]) { F[j][b] = v; previo[j][b] = a; }
                }

        // La ultima clase va desde el ultimo umbral + 1 hasta L-1.
        int ultimo = -1;
        double mejor = MENOS_INF;
        for (int a = ins.k; a <= L - 2; ++a) {
            if (F[ins.k - 1][a] == MENOS_INF) continue;
            double v = F[ins.k - 1][a] + aporteClase(a + 1, L - 1, ins);
            if (v > mejor) { mejor = v; ultimo = a; }
        }

        // Reconstruir los umbrales desde el ultimo hacia atras.
        Solucion t(ins.k);
        for (int j = ins.k - 1, b = ultimo; j >= 0; b = previo[j][b], --j) t[j] = b;
        double f = evaluar(t, ins);   // tambien verifica que sea factible

        if (modoCsv) {
            cout << ins.imagen << ',' << ins.k << ',' << fixed << setprecision(6) << f << ',';
            for (int i = 0; i < ins.k; ++i) cout << (i ? " " : "") << t[i];
            cout << "\n";
        } else {
            cout << "instancia " << ins.imagen << " k=" << ins.k << "\numbrales";
            for (int ti : t) cout << ' ' << ti;
            cout << "\nf " << fixed << setprecision(6) << f << "\n";
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
