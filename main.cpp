// main.cpp — Lee una instancia, genera una solucion inicial, la mejora con
// Busqueda Tabu y escribe el resultado.
//
// Uso:
//   ./tabu <instancia.txt> [-k K] [-m greedy|aleatoria] [-s semilla]
//                      [-t tenencia] [-i iteraciones] [-o salida.txt]
//
//   -k  cambia la cantidad de umbrales (por defecto, la k de la instancia)
//   -m  metodo de solucion inicial (por defecto greedy)
//   -s  semilla para el metodo aleatorio (por defecto 42)
//   -t  tenencia: iteraciones que dura una prohibicion (por defecto 7)
//   -i  iteraciones de la Busqueda Tabu (por defecto 1000; 0 = solo solucion inicial)
//   -o  archivo de salida (por defecto, solo consola)
//
// Ejemplo (3 umbrales sobre la imagen 1):
//   ./tabu imagenes_preprocesadas/instancias/imagen_01_k2.txt -k 3
#include "instancia.hpp"
#include "solucion.hpp"
#include "tabu.hpp"

#include <chrono>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0]
             << " <instancia.txt> [-k K] [-m greedy|aleatoria] [-s semilla] [-t tenencia] [-i iteraciones] [-o salida.txt]\n";
        return 1;
    }
    try {
        // 1) Parametros por consola.
        string ruta = argv[1], metodo = "greedy", salida = "-";
        int k = -1, tenencia = 7, iteraciones = 1000;
        unsigned semilla = 42;
        for (int i = 2; i + 1 < argc; i += 2) {
            string op = argv[i], val = argv[i + 1];
            if (op == "-k") k = stoi(val);
            else if (op == "-m") metodo = val;
            else if (op == "-s") semilla = stoul(val);
            else if (op == "-t") tenencia = stoi(val);
            else if (op == "-i") iteraciones = stoi(val);
            else if (op == "-o") salida = val;
            else throw invalid_argument("Opcion desconocida: " + op);
        }
        if (metodo != "greedy" && metodo != "aleatoria")
            throw invalid_argument("Metodo desconocido: " + metodo);
        if (tenencia < 0 || iteraciones < 0)
            throw invalid_argument("Tenencia e iteraciones deben ser >= 0");

        // 2) Lectura de la instancia.
        Instancia ins = leerInstancia(ruta);
        if (k != -1) {
            if (k < 1 || k > ins.L - 2) throw invalid_argument("k fuera de rango");
            ins.k = k;
        }
        mostrarInstancia(ins);

        // 3) Solucion inicial + Busqueda Tabu (se mide el tiempo total).
        auto inicio = chrono::steady_clock::now();
        Solucion inicial;
        if (metodo == "greedy") inicial = solucionGreedy(ins);
        else inicial = solucionAleatoria(ins, semilla);
        ResultadoTabu res = busquedaTabu(inicial, ins, tenencia, iteraciones);
        double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - inicio).count();

        // 4) Escritura: primero la inicial (como referencia) y luego la final.
        cout << "\nSolucion inicial (" << metodo << "): umbrales";
        for (int ti : inicial) cout << ' ' << ti;
        cout << ", f = " << evaluar(inicial, ins) << "\n";
        cout << "Tabu: mejor en iteracion " << res.iteracionMejor << " de " << iteraciones << "\n\n";

        string descripcion = "tabu desde " + metodo + " (tenencia " + to_string(tenencia) +
                             ", iteraciones " + to_string(iteraciones) +
                             ", mejor en iteracion " + to_string(res.iteracionMejor) + ")";
        escribirSolucion(res.mejor, ins, descripcion, ms, "-");
        if (salida != "-") {
            escribirSolucion(res.mejor, ins, descripcion, ms, salida);
            cout << "Guardado en " << salida << "\n";
        }
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
