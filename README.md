# Funciones auxiliares para imágenes MRI

Este proyecto entrega la base para trabajar con imágenes MRI en C++: cargar imágenes, prepararlas, consultar píxeles, construir histogramas y guardar segmentaciones.

Las imágenes del proyecto son de **256 × 256 píxeles, en gris de 8 bits**, con intensidades entre **0 y 255**. 

## Archivos principales

| Archivo | Para qué sirve |
| --- | --- |
| `imagenes.hpp` | Declara las funciones auxiliares y los tipos de datos. |
| `imagenes.cpp` | Implementa esas funciones mediante OpenCV. |
| `preprocesar.cpp` | Prepara 10 imágenes y genera las 30 instancias del proyecto. |
| `segmentar.cpp` | Aplica a una imagen los umbrales recibidos por consola y guarda el resultado. |
| `Makefile` | Compila y limpia los ejecutables. |

## Funciones disponibles

Todas las funciones de `imagenes.hpp` pertenecen al namespace `mri`. Las imágenes se entregan como `cv::Mat`: una matriz con una intensidad por píxel. El tipo `CV_8UC1` significa que cada píxel tiene un único canal de 8 bits.

| Necesidad | Función | Qué hace |
| --- | --- | --- |
| Leer JPG | `cargarImagen(ruta)` | Recibe la ruta, carga el archivo en memoria y devuelve la matriz de intensidades. También permite leer PNG. |
| Convertir a gris | Incluida en `cargarImagen(ruta)` | Si la imagen tiene color, la convierte automáticamente a un canal de gris. |
| Adaptar la resolución | `redimensionar(imagen)` | Devuelve una nueva matriz de 256 × 256. También permite indicar ancho y alto. |
| Consultar un píxel | `intensidad(imagen, fila, columna)` | Devuelve un entero de 0 a 255 y comprueba que las coordenadas sean válidas. |
| Construir el histograma | `calcularHistograma(imagen)` | Devuelve un arreglo de 256 contadores: la posición g indica cuántos píxeles tienen intensidad g. |
| Guardar una imagen | `guardarImagen(imagen, ruta)` | Recibe una matriz final de intensidades y la guarda como PNG sin pérdida. |
| Generar la segmentación | `segmentar(imagen, umbrales)` | Asigna una clase a cada píxel utilizando los umbrales recibidos. |
| Exportar el histograma | `guardarHistograma(histograma, ruta)` | Guarda un CSV con las columnas `intensidad,frecuencia`. |
| Validar entradas | `validarImagen(imagen)` y `validarUmbrales(umbrales)` | Comprueban el formato de la matriz y los límites y el orden de los umbrales. |

La carga conserva el tamaño original y acepta imágenes de 8 bits; rechaza otras profundidades. Para adaptar una imagen original al proyecto se carga y luego se redimensiona.

La tabla describe funciones que puedes llamar desde tu código. Que exista
`guardarHistograma` no significa que todos los ejecutables generen un CSV:
`preprocesar` incluye el histograma dentro de las instancias `.txt`, mientras que
`segmentar` lo exporta a `histograma.csv`.

Las coordenadas comienzan en cero. Para un píxel **(x, y)**, x es la columna e y es la fila: se consulta con **`mri::intensidad(imagen, y, x)`**.

El guardado auxiliar requiere extensión `.png`, crea las carpetas necesarias y conserva los valores exactos de la matriz. El preprocesador guarda sus imágenes en JPG. Los archivos de destino se reemplazan si ya existen. Las funciones informan entradas inválidas o fallos de lectura y escritura mediante excepciones.

## Cómo se representa la segmentación

Los umbrales se reciben como `vector<int>` y deben cumplir `1 <= t1 < ... < tk <= 254`.
El proyecto utiliza **k = 2, 4, 6** para sus instancias. La función y el ejecutable
`segmentar` aceptan cualquier cantidad de umbrales entre 1 y 254.

Se forman **k + 1 clases** con intervalos `[0,t1]`, `[t1+1,t2]`, …, `[tk+1,255]`. Un píxel igual a un umbral queda en la clase inferior. Se incluye el fondo.

`segmentar` devuelve una estructura `Segmentacion` con dos matrices:

- `etiquetas`: contiene el número de clase de cada píxel, de 0 a k.
- `visualizacion`: representa las clases con tonos distribuidos entre 0 y 255
  para distinguirlas al abrir la imagen.

Ambas se pueden guardar con `guardarImagen`. Los tonos de visualización no son las intensidades medias de las clases. Esta función aplica umbrales ya definidos.

## Compilación y limpieza

Se necesitan C++17, Make, pkg-config y OpenCV 4. En Ubuntu/Debian:

```bash
sudo apt update
sudo apt install g++ make pkg-config libopencv-dev
```

`make` compila los ejecutables `preprocesar` y `segmentar`. También se pueden
compilar individualmente con `make preprocesar` y `make segmentar`.

`make clean` elimina los ejecutables y los archivos `.o` y `.d` de la raíz.
Conserva el código, las imágenes originales, los datos procesados y las salidas.

## Preparación de las 30 instancias

Sintaxis general:

```text
./preprocesar <carpeta_dataset> <carpeta_salida> [semilla]
```

Ejemplo

```text
./preprocesar dataset_original imagenes_preprocesadas
```

Los argumentos entre `< >` son obligatorios y los que aparecen entre `[ ]` son
opcionales; estos signos no se escriben en el comando. La semilla vale 42 por
defecto y permite repetir la selección con el mismo dataset y entorno.

El programa busca JPG/JPEG en la carpeta y sus subcarpetas, selecciona 10 imágenes,
las convierte a gris y las redimensiona a 256 × 256. Requiere al menos 10 imágenes.
Genera tres instancias por imagen, para k = 2, 4 y 6.

| Salida | Contenido |
| --- | --- |
| `imagenes/` | Diez JPG procesados, identificados de `imagen_01` a `imagen_10`. |
| `instancias/` | Treinta archivos `.txt` con los datos del problema y su histograma. |

Cada instancia contiene la cabecera `SEGMENTACION_MULTIUMBRAL_V1`, el nombre de
imagen, `ancho` y `alto` (256), `L` (256 niveles), `k` (cantidad de umbrales),
`N` (65536 píxeles) y la marca `histograma`, seguida de 256 frecuencias.
Su suma debe ser N. Las tres instancias de una imagen comparten el histograma;
solo cambia k.

Las 256 frecuencias están en una línea, separadas por espacios y ordenadas por
intensidad: la primera cuenta los píxeles de intensidad 0, la segunda los de
intensidad 1 y así hasta la intensidad 255. No es una lista de la intensidad de
cada píxel; es un recuento de cuántos píxeles tienen cada intensidad.

El histograma se calcula después de releer el JPG guardado para que coincida con
sus píxeles, ya que la compresión JPEG puede modificar intensidades. Las instancias
contienen datos de entrada, no soluciones.

## Guardado de una segmentación por consola

Sintaxis general:

```text
./segmentar <imagen> <carpeta_salida> <t1> [t2 ... tk]
```

Ejemplo

```text
./segmentar imagenes_preprocesadas/imagenes/imagen_01.jpg ejemplo 50 80
```

Los umbrales se escriben como enteros separados por espacios. El programa conserva
el tamaño de entrada: para trabajar con 256 × 256 se utilizan las imágenes
preprocesadas. En la carpeta de salida guarda:

- `segmentada.png`: visualización de las clases en gris.
- `etiquetas.png`: identificador de clase de cada píxel. (No es muy útil de ver, es más útil para que el programa pueda leer las clases)
- `histograma.csv`: frecuencias de la imagen de entrada.


Y eso!! Estas son las funciones básicas de lectura de imágenes, intensidades, etc. Este es el punto de partida, de acá en adelante te toca implementar la parte algorítmica :D.
---

# Proyecto: Segmentación multiumbral con Búsqueda Tabú (estado de avance)

Programa `tabu`: lee una instancia, genera una **solución inicial** (greedy o aleatoria),
la mejora con **Búsqueda Tabú**, la **evalúa** con el criterio de Otsu y **escribe** el resultado. No usa OpenCV.

## Archivos del proyecto

| Archivo | Qué hace |
| --- | --- |
| `instancia.hpp/.cpp` | Lee y valida el `.txt` (cabecera, campos, suma del histograma = N) y precalcula sumas acumuladas. |
| `solucion.hpp/.cpp` | Representación, restricciones, función de evaluación, greedy, aleatoria y escritura de la salida. |
| `tabu.hpp/.cpp` | Búsqueda Tabú: movimientos ±1 y ±10, lista tabú y criterio de aspiración. |
| `main.cpp` | Une todo: parámetros por consola → lectura → solución inicial → Tabú → salida. |

## En 6 líneas

1. **Solución** = vector ordenado de k umbrales `t1 < ... < tk`, cada uno en `[1, 254]`.
2. **Clases** = `[0,t1]`, `[t1+1,t2]`, …, `[tk+1,255]` (igual que `mri::segmentar`).
3. **Evaluación (Otsu)** = `f = Σ w_j (mu_j − muT)²`; con sumas acumuladas cuesta O(k).
4. **Greedy** = agrega un umbral a la vez, el que más sube f. **Aleatoria** = k valores distintos al azar, ordenados.
5. **Restricciones** = se cumplen por construcción; `esFactible()` rechaza lo que no las cumpla.
6. **Tabú** = en cada iteración mueve un umbral ±1 o ±10 al mejor vecino no prohibido (aunque empeore),
   prohíbe volver al valor anterior por `tenencia` iteraciones y guarda la mejor solución vista.

## Uso

```bash
make tabu
./tabu imagenes_preprocesadas/instancias/imagen_01_k2.txt -k 3          # Tabú desde greedy, 3 umbrales
./tabu imagenes_preprocesadas/instancias/imagen_01_k4.txt -m aleatoria -s 7 -o salida.txt
./tabu imagenes_preprocesadas/instancias/imagen_01_k6.txt -t 10 -i 500  # otra tenencia e iteraciones
./tabu imagenes_preprocesadas/instancias/imagen_01_k2.txt -i 0          # solo la solución inicial
make probar                                                            # las 30 instancias → resultados/
```

Salida (ejemplo `imagen_01`, k = 3): greedy parte en (18, 49, 110) con f = 2039,81 y Tabú llega a

```text
metodo tabu desde greedy (tenencia 7, iteraciones 1000, mejor en iteracion 8)
umbrales 28 72 119
f 2062.638907
# clase desde hasta pixeles proporcion media
0 0 28 42198 0.643890 2.663
...
```

## Resultados (30 instancias, tenencia 7, 1000 iteraciones)

- Desde greedy, Tabú alcanza el óptimo exacto en las 30 instancias (comparado con fuerza bruta en k = 2 y programación dinámica en k = 4 y 6).
- Desde aleatoria llega al mismo valor en 29 de 30; la excepción es una instancia con k = 6.
- Cada instancia tarda menos de 3 ms.

## Pendiente

- Criterio de término por estancamiento (hoy siempre hace todas las iteraciones).
- Calibrar tenencia e iteraciones y reportar varias semillas desde aleatoria.
- Generar las imágenes segmentadas finales con `segmentar`.
