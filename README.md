# 🚀 Programación Paralela y Concurrente

Este repositorio contiene todo el código fuente, proyectos, pruebas y trabajos grupales desarrollados durante el curso de **Programación Paralela y Concurrente**. El objetivo principal de la materia ha sido explorar, comprender e implementar distintos paradigmas de paralelismo para resolver problemas computacionalmente costosos de manera eficiente.

## 🛠️ Tecnologías y Herramientas
El repositorio hace uso de un stack moderno enfocado en el alto rendimiento:
*   **Lenguajes:** C++ (GCC >= 14, MSVC), Java, CUDA C++.
*   **Paralelismo de Memoria Compartida:** OpenMP, Threads (Java).
*   **Paralelismo de Memoria Distribuida:** MPI (Message Passing Interface - Intel oneAPI).
*   **Aceleración por Hardware (GPU):** NVIDIA CUDA (Toolkit >= 12).
*   **Vectorización:** SIMD (Instrucciones AVX2).
*   **Gráficos e Interfaz:** SFML (Simple and Fast Multimedia Library) y librerías stb_image.
*   **Construcción y Despliegue:** CMake, Ninja, Vcpkg.

## 📂 Estructura del Repositorio y Temáticas

A lo largo del semestre, el aprendizaje se estructuró de manera progresiva, abarcando los siguientes pilares:

### 1. Conceptos Base y Multithreading (CPU)
*   `01.fractal-julia` / `02.fractal-java`: Generación de conjuntos fractales de Julia. Introducción a la carga computacional intensiva y su resolución mediante hilos (threads).
*   `03.fractal-dll`: Modularidad e integración de bibliotecas de enlace dinámico (DLL) con C++.

### 2. Memoria Compartida (OpenMP)
*   `04.ejemplo-openmp`: Uso de directivas `#pragma omp parallel` y `#pragma omp for`. 
*   Paralelización a nivel de ciclo para optimizar bucles sin necesidad de gestionar la creación de hilos manualmente.

### 3. Memoria Distribuida (MPI)
*   `05.ejemplo-mpi` / `06.fractal-mpi`: Cambio de paradigma hacia arquitecturas de clúster.
*   Comunicación Punto a Punto (`Send`, `Recv`) y Comunicación Colectiva (`Bcast`, `Scatterv`, `Gatherv`).
*   División matemática de la carga de trabajo entre nodos que no comparten memoria física.

### 4. Computación en GPU (CUDA)
*   `07.cuda` / `08.fractal-cuda`: Traslado de la carga masiva a la Tarjeta Gráfica.
*   Uso de modificadores `__global__`, `__device__` y variables `__constant__`.
*   Diseño de mallas (Grids) y bloques 1D/2D para asignar hilos lógicos a miles de núcleos físicos CUDA.

### 5. 🏆 Proyectos Destacados (Integración)
*   **[Simulación de la Ecuación de Calor 2D (`trabajo-grupal_ecuacion-de-calor-2d`)]:** Un benchmark completo que simula la propagación térmica de una placa. Compara la eficiencia algorítmica entre código Serial, Vectorizado (SIMD AVX2), OpenMP y MPI, renderizando los resultados térmicos en tiempo real con SFML.
*   **[Procesamiento y Filtros de Imágenes (`repaso-ev-final` / Exámenes)]:** Implementación de convoluciones matemáticas (Filtro Gaussiano, Detección de Bordes) aplicadas a imágenes RGBA. Requiere la fusión arquitectónica de OpenMP, MPI y CUDA en un solo entorno, manejando memoria dinámica inter-dispositivo, división del trabajo por píxeles y validación estricta de coordenadas y bordes.

## ⚙️ Compilación y Ejecución

La mayoría de los proyectos en C++ están rigurosamente configurados para compilarse automáticamente usando **CMake**. 

**Requisitos Previos:**
*   CMake (>= 3.16)
*   Compilador C++ compatible (MinGW g++ o MSVC cl.exe)
*   NVIDIA CUDA Toolkit instalado y mapeado en el PATH.
*   Intel MPI (u otra implementación de MS-MPI) instalada.

```bash
# Ejemplo estándar de compilación para un sub-proyecto
cd <nombre_del_proyecto>
mkdir build && cd build
cmake -G Ninja ..
cmake --build .
```

---
*Desarrollado como evidencia de aprendizaje y dominio de la optimización algorítmica y arquitecturas de alto rendimiento.*
