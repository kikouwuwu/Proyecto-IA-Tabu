// main.cpp — Lee una instancia, genera una solucion inicial aleatoria, la mejora
// con Busqueda Tabu y escribe el resultado.
//
// Uso:
//   ./tabu <instancia.txt> [-k K] [-s semilla] [-t tenencia] [-p paso_largo]
//          [-i iteraciones] [-e estancamiento] [-o salida.txt] [-a avance.csv] [-c]
//
//   -k  cambia la cantidad de umbrales (por defecto, la k de la instancia)
//   -s  semilla de la solucion inicial aleatoria (por defecto 1)
//   -t  tenencia: iteraciones que dura una prohibicion (por defecto 15)
//   -p  tamano del salto largo M2 (por defecto 20; 0 = solo movimientos +-1)
//   -i  maximo de iteraciones (por defecto 1000; 0 = solo solucion inicial)
//   -e  iteraciones seguidas sin mejora para detenerse (por defecto 200; 0 = no se usa)
//   -o  archivo de salida (por defecto, solo consola)
//   -a  guarda el avance de la busqueda: iteracion, f actual y f mejor
//   -c  modo experimento: imprime una sola linea CSV con el resumen de la ejecucion
//
// Ejemplos:
//   ./tabu imagenes_preprocesadas/instancias/imagen_01_k4.txt
//   ./tabu imagenes_preprocesadas/instancias/imagen_01_k2.txt -k 3 -s 5 -o salida.txt
#include "instancia.hpp"
#include "solucion.hpp"
#include "tabu.hpp"

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

using namespace std;

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0]
             << " <instancia.txt> [-k K] [-s semilla] [-t tenencia] [-p paso_largo]"
                " [-i iteraciones] [-e estancamiento] [-o salida.txt] [-a avance.csv] [-c]\n";
        return 1;
    }
    try {
        // 1) Parametros por consola.
        string ruta = argv[1], salida = "-", rutaAvance = "";
        int k = -1;
        unsigned semilla = 1;
        bool modoCsv = false;
        ParametrosTabu par;
        for (int i = 2; i < argc; ++i) {
            string op = argv[i];
            if (op == "-c") { modoCsv = true; continue; }
            if (i + 1 >= argc) throw invalid_argument("Falta el valor de " + op);
            string val = argv[++i];
            if (op == "-k") k = stoi(val);
            else if (op == "-s") semilla = stoul(val);
            else if (op == "-t") par.tenencia = stoi(val);
            else if (op == "-p") par.pasoLargo = stoi(val);
            else if (op == "-i") par.iteraciones = stoi(val);
            else if (op == "-e") par.estancamiento = stoi(val);
            else if (op == "-o") salida = val;
            else if (op == "-a") rutaAvance = val;
            else throw invalid_argument("Opcion desconocida: " + op);
        }
        if (par.tenencia < 0 || par.pasoLargo < 0 || par.iteraciones < 0 || par.estancamiento < 0)
            throw invalid_argument("Los parametros de Tabu deben ser >= 0");
        if (par.pasoLargo == 1)
            throw invalid_argument("El paso largo debe ser 0 o mayor que 1");

        // 2) Lectura de la instancia.
        Instancia ins = leerInstancia(ruta);
        if (k != -1) {
            if (k < 1 || k > ins.L - 2) throw invalid_argument("k fuera de rango");
            ins.k = k;
        }
        if (!modoCsv) mostrarInstancia(ins);

        ofstream archivoAvance;
        if (!rutaAvance.empty()) {
            archivoAvance.open(rutaAvance);
            if (!archivoAvance) throw runtime_error("No se pudo escribir: " + rutaAvance);
            archivoAvance << "iteracion,f_actual,f_mejor\n" << fixed << setprecision(6);
        }

        // 3) Solucion inicial + Busqueda Tabu (se mide el tiempo total).
        auto inicio = chrono::steady_clock::now();
        Solucion inicial = solucionAleatoria(ins, semilla);
        ResultadoTabu res = busquedaTabu(inicial, ins, par,
                                         rutaAvance.empty() ? nullptr : &archivoAvance);
        double ms = chrono::duration<double, milli>(chrono::steady_clock::now() - inicio).count();
        double fInicial = evaluar(inicial, ins);

        // 4a) Modo experimento: una linea CSV (el encabezado lo pone experimentos.sh).
        if (modoCsv) {
            cout << ins.imagen << ',' << ins.k << ',' << semilla << ',' << par.tenencia << ','
                 << par.pasoLargo << ',' << par.iteraciones << ',' << par.estancamiento << ','
                 << fixed << fInicial << ',' << res.fMejor << ',' << res.iteracionMejor << ','
                 << res.iteracionesHechas << ',' << ms << ',';
            for (int i = 0; i < ins.k; ++i) cout << (i ? " " : "") << res.mejor[i];
            cout << "\n";
            return 0;
        }

        // 4b) Escritura normal: primero la inicial (como referencia) y luego la final.
        cout << "\nSolucion inicial (aleatoria, semilla " << semilla << "): umbrales";
        for (int ti : inicial) cout << ' ' << ti;
        cout << ", f = " << fInicial << "\n";
        cout << "Tabu: " << res.iteracionesHechas << " iteraciones, mejor en la iteracion "
             << res.iteracionMejor << "\n\n";

        string descripcion = "tabu desde aleatoria (semilla " + to_string(semilla) +
                             ", tenencia " + to_string(par.tenencia) +
                             ", paso largo " + to_string(par.pasoLargo) +
                             ", iteraciones " + to_string(res.iteracionesHechas) +
                             " de " + to_string(par.iteraciones) +
                             ", estancamiento " + to_string(par.estancamiento) +
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
