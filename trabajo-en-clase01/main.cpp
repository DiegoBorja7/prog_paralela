#include <iostream>
#include <vector>
#include <immintrin.h>
#include <fmt/core.h>
#include <omp.h>
#include <chrono>
#include <cmath>

float operaciones_vectoriales(const std::vector<float> &x, const std::vector<float> &y)
{
    int n = (int)x.size();
    int i = 0;
    __m256 acc = _mm256_setzero_ps();

    for (; i + 7 < n; i += 8)
    {
        __m256 a = _mm256_loadu_ps(&x[i]);
        __m256 b = _mm256_loadu_ps(&y[i]);
        acc = _mm256_add_ps(acc, _mm256_mul_ps(a, b));
    }

    float temp[8];
    _mm256_storeu_ps(temp, acc);

    float sum = 0.0f;
    for (int k = 0; k < 8; ++k)
        sum += temp[k];

    for (; i < n; ++i)
        sum += x[i] * y[i];

    return sum;
}

float operaciones_openmp(const std::vector<float> &x, const std::vector<float> &y)
{
    int n = (int)x.size();

    // Reserva grande: maximo posible de hilos
    std::vector<float> parciales(omp_get_max_threads(), 0.0f);

#pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        int delta = n / num_threads;
        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;

        if (thread_id == num_threads - 1)
        {
            end = n; // ultimo hilo toma sobrante
        }

        float suma_local = 0.0f;
        for (int i = start; i < end; ++i)
        {
            suma_local += x[i] * y[i];
        }

        parciales[thread_id] = suma_local;
    }

    float producto_escalar = 0.0f;
    for (float p : parciales)
    {
        producto_escalar += p;
    }

    return producto_escalar;
}

int main()
{
    fmt::print("============TRABAJO EN CLASE============\n");
    fmt::print("Integrantes:\n\tBorja Diego\n\tJami Mateo\n");
    fmt::print("=========================================\n\n");

    int n = 10;
    std::vector<float> x(n), y(n);

    for (int i = 0; i < n; ++i)
    {
        x[i] = i + 1; //vector a consecutivo: 1, 2, ..., n
        y[i] = 2 * i; //vector b consecutivo: 0, 2, 4, ..., 2*(n-1) 
    }

    fmt::print("============================================\n");
    fmt::print(" Producto escalar de dos vectores (n = {})\n", n);
    fmt::print("============================================\n");

    fmt::print("X = ");
    for (int i = 0; i < n; ++i)
    {
        fmt::print("{}{}", x[i], (i + 1 < n) ? ", " : "\n");
    }

    fmt::print("Y = ");
    for (int i = 0; i < n; ++i)
    {
        fmt::print("{}{}", y[i], (i + 1 < n) ? ", " : "\n");
    }

    fmt::print("--------------------------------------------\n");
    auto t0 = std::chrono::high_resolution_clock::now();
    float r1 = operaciones_vectoriales(x, y);
    auto t1 = std::chrono::high_resolution_clock::now();

    float r2 = operaciones_openmp(x, y);
    auto t2 = std::chrono::high_resolution_clock::now();

    auto simd_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    auto openmp_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double diff_ms = openmp_ms - simd_ms;

    fmt::print("SIMD   : {}\n", r1);
    fmt::print("Tiempo SIMD   : {:.3f} ms\n", simd_ms);
    fmt::print("OpenMP : {}\n", r2);
    fmt::print("Tiempo OpenMP : {:.3f} ms\n", openmp_ms);
    fmt::print("\nDiferencia tiempo (OpenMP - SIMD): {:.3f} ms\n", diff_ms);
    fmt::print("--------------------------------------------\n");
    return 0; 
}