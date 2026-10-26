CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pedantic
OPENCV = $(shell pkg-config --cflags --libs opencv4)

.PHONY: all clean probar

all: preprocesar segmentar tabu

# Programa del proyecto: no usa OpenCV, solo lee las instancias .txt.
tabu: main.cpp instancia.cpp instancia.hpp solucion.cpp solucion.hpp tabu.cpp tabu.hpp
	$(CXX) $(CXXFLAGS) main.cpp instancia.cpp solucion.cpp tabu.cpp -o tabu

# Corre Tabu desde greedy y desde aleatoria en las 30 instancias y guarda las salidas en resultados/.
probar: tabu
	mkdir -p resultados
	for f in imagenes_preprocesadas/instancias/*.txt; do \
	  n=$$(basename $$f .txt); \
	  ./tabu $$f -m greedy -o resultados/$${n}_greedy.txt > /dev/null || exit 1; \
	  ./tabu $$f -m aleatoria -o resultados/$${n}_aleatoria.txt > /dev/null || exit 1; \
	done
	@echo "OK: $$(ls resultados | wc -l) archivos en resultados/"

segmentar: segmentar.cpp imagenes.cpp imagenes.hpp
	$(CXX) $(CXXFLAGS) segmentar.cpp imagenes.cpp -o segmentar $(OPENCV)

preprocesar: preprocesar.cpp
	$(CXX) $(CXXFLAGS) preprocesar.cpp -o preprocesar $(OPENCV)

clean:
	$(RM) preprocesar segmentar tabu *.o *.d
