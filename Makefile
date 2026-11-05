CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pedantic
OPENCV = $(shell pkg-config --cflags --libs opencv4)

.PHONY: all base clean probar experimentos

# Programas del proyecto: no usan OpenCV, solo leen las instancias .txt.
all: tabu optimo

tabu: main.cpp instancia.cpp instancia.hpp solucion.cpp solucion.hpp tabu.cpp tabu.hpp
	$(CXX) $(CXXFLAGS) main.cpp instancia.cpp solucion.cpp tabu.cpp -o tabu

# Optimo exacto (programacion dinamica), solo para medir la calidad de Tabu.
optimo: optimo.cpp instancia.cpp instancia.hpp solucion.cpp solucion.hpp
	$(CXX) $(CXXFLAGS) optimo.cpp instancia.cpp solucion.cpp -o optimo

# Corre Tabu (parametros por defecto, semilla 1) en las 30 instancias y guarda las salidas en resultados/.
probar: tabu
	mkdir -p resultados
	for f in imagenes_preprocesadas/instancias/*.txt; do \
	  n=$$(basename $$f .txt); \
	  ./tabu $$f -o resultados/$${n}.txt > /dev/null || exit 1; \
	done
	@echo "OK: $$(ls resultados | wc -l) archivos en resultados/"

# Experimentos del informe (30 semillas por instancia); escribe los CSV en experimentos/.
experimentos: tabu optimo
	sh experimentos.sh

# Programas del codigo base (necesitan OpenCV 4).
base: preprocesar segmentar

segmentar: segmentar.cpp imagenes.cpp imagenes.hpp
	$(CXX) $(CXXFLAGS) segmentar.cpp imagenes.cpp -o segmentar $(OPENCV)

preprocesar: preprocesar.cpp
	$(CXX) $(CXXFLAGS) preprocesar.cpp -o preprocesar $(OPENCV)

clean:
	$(RM) preprocesar segmentar tabu optimo *.o *.d
