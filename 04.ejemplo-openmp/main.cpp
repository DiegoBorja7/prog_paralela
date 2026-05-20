#include <iostream>
#include <fmt/core.h>
#include <omp.h>

void Hola_OpenMP()
{
    fmt::print("Adios mundo serial, Hola OpenMP!\n");
    #pragma omp parallel num_threads(4) // Indica que el bloque de código siguiente se ejecutará en paralelo utilizando 4 threads
    {
        int thereads_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();
    #pragma omp master
        {
            fmt::print("Soy el thread maestro, tengo {} threads en total.\n", thereads_count);
        }
        fmt::print("Tengo {} threads y mi thread ID es {}.\n", thereads_count, thread_id);
    }
}

void barra_por_hilo()
{
    fmt::print("Barra por hilo\n");

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        std::string msg = "";

    #pragma omp parallel for

        for (int i = 0; i < thread_id; ++i)
        {
            msg += "*";
        }

        fmt::print("Thread ID es {} . msg: {}\n", thread_id, msg);
    }
}


int main()
{
    //Hola_OpenMP();
    //barra_por_hilo();

    int num_elementos = 15;

    #pragma omp parallel for num_threads(4)
    for (int i = 0; i < 10; ++i)
    {
        fmt::print("i: {} thread id: {}\n", i, omp_get_thread_num());
    }

    #pragma omp parallel num_threads(8)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();

        int delta = num_elementos / num_threads;
        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;

        if (thread_id == num_threads - 1){
            end = num_elementos;
        }

        fmt::print("Thread ID: {}. Procesando elementos desde {} hasta {}.\n", thread_id, start, end);
    }

    #pragma omp parallel num_threads(8)
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        for (int i = 0; i < num_elementos; i+=num_threads){
            fmt::print("Thread ID: {}. Procesando elemento {}.\n", thread_id, i);
        }
    }

    return 0;
}

