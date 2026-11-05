// tabu.cpp — Implementacion de la Busqueda Tabu.
#include "tabu.hpp"

#include <utility>
#include <vector>

using namespace std;

// Revisa si el par (indice, valor) esta en la lista tabu.
static bool esTabu(const vector<pair<int, int>>& lista, int indice, int valor) {
    for (const auto& par : lista)
        if (par.first == indice && par.second == valor) return true;
    return false;
}

ResultadoTabu busquedaTabu(const Solucion& inicial, const Instancia& ins,
                           const ParametrosTabu& par, ostream* avance) {
    // M1 (+-1) siempre; M2 (+-pasoLargo) solo si pasoLargo > 0.
    vector<int> pasos = {-1, 1};
    if (par.pasoLargo > 0) pasos = {-par.pasoLargo, -1, 1, par.pasoLargo};

    Solucion actual = inicial;
    double fActual = evaluar(inicial, ins);
    ResultadoTabu res;
    res.mejor = inicial;
    res.fMejor = fActual;
    if (avance) *avance << 0 << ',' << fActual << ',' << res.fMejor << '\n';

    vector<pair<int, int>> listaTabu;        // (indice del umbral, valor prohibido)

    for (int it = 1; it <= par.iteraciones; ++it) {
        // 1) Recorrer el vecindario y quedarse con el mejor vecino permitido.
        Solucion mejorVecino;
        double fMejorVecino = -1.0;
        int indiceMovido = -1;

        for (int i = 0; i < ins.k; ++i) {
            for (int paso : pasos) {
                Solucion vecino = actual;
                vecino[i] += paso;
                if (!esFactible(vecino, ins)) continue;

                double f = evaluar(vecino, ins);
                bool tabu = esTabu(listaTabu, i, vecino[i]);
                // Aspiracion: si es tabu pero da un nuevo record, se permite.
                if (tabu && f <= res.fMejor) continue;

                if (f > fMejorVecino) {
                    fMejorVecino = f;
                    mejorVecino = vecino;
                    indiceMovido = i;
                }
            }
        }

        // Si todos los vecinos estan prohibidos, no hay movimiento posible.
        if (indiceMovido == -1) break;
        res.iteracionesHechas = it;

        // 2) Prohibir volver al valor anterior del umbral movido.
        listaTabu.push_back({indiceMovido, actual[indiceMovido]});
        if ((int)listaTabu.size() > par.tenencia) listaTabu.erase(listaTabu.begin());

        // 3) Moverse, aunque el vecino sea peor que la actual.
        actual = mejorVecino;
        fActual = fMejorVecino;

        // 4) Recordar la mejor solucion vista.
        if (fActual > res.fMejor) {
            res.fMejor = fActual;
            res.mejor = actual;
            res.iteracionMejor = it;
        }
        if (avance) *avance << it << ',' << fActual << ',' << res.fMejor << '\n';

        // 5) Termino por estancamiento: muchas iteraciones sin mejorar.
        if (par.estancamiento > 0 && it - res.iteracionMejor >= par.estancamiento) break;
    }
    return res;
}
