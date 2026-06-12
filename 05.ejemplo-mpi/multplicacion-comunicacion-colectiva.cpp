#include <mpi.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <fmt/core.h>

#define MATRIZ_DIM 25

void multiplicar_matriz_vector(const std::vector<double> &A, const std::vector<double> &b, std::vector<double> &x, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        double sum = 0.0;
        x[i] = 0.0;
        for (int j = 0; j < cols; j++)
        {
            int index = i * cols + j;
            sum += A[index] * b[j];
        }
        x[i] = sum;
    }
}

void imprimir_vector(const std::vector<double> &b, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int index = i * cols + j;
            fmt::print("{:.2f} ", b[index]);
        }
        fmt::print("\n");
    }
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int rows_per_rank = std::ceil((MATRIZ_DIM) * 1.0 / nprocs);

    auto filas_para_rank = [&](int proc)
    {
        int inicio = proc * rows_per_rank;
        if (inicio >= MATRIZ_DIM) return 0;
        return std::min(rows_per_rank, MATRIZ_DIM - inicio);
    };

    auto offset_para_rank = [&](int proc)
    {
        return proc * rows_per_rank;
    };

    int filas_locales = filas_para_rank(rank);

    // Preparación de variables para operaciones colectivas (Scatterv y Gatherv)
    // Todos los procesos deben conocer cómo se reparte la información
    std::vector<int> sendcounts_A(nprocs);
    std::vector<int> displs_A(nprocs);
    std::vector<int> recvcounts_x(nprocs);
    std::vector<int> displs_x(nprocs);

    for (int i = 0; i < nprocs; i++)
    {
        int filas = filas_para_rank(i);
        recvcounts_x[i] = filas;                       // Cuántos elementos de 'x' devuelve cada rank
        displs_x[i] = offset_para_rank(i);             // Desde dónde se insertan en 'x' global
        sendcounts_A[i] = filas * MATRIZ_DIM;          // Cuántos elementos de 'A' recibe cada rank
        displs_A[i] = offset_para_rank(i) * MATRIZ_DIM;// Desde dónde se leen en 'A' global
    }

    // Vectores globales
    std::vector<double> A; 
    std::vector<double> x;
    
    // Vector b es global para todos, porque todos lo necesitan entero
    std::vector<double> b(MATRIZ_DIM);

    // Vectores locales donde cada proceso recibirá su pedazo de trabajo
    std::vector<double> A_local(filas_locales * MATRIZ_DIM);
    std::vector<double> x_local(filas_locales);

    // Inicialización de datos en el proceso 0
    if (rank == 0)
    {
        A.resize(MATRIZ_DIM * MATRIZ_DIM);
        x.resize(MATRIZ_DIM);

        for (int i = 0; i < MATRIZ_DIM; i++)
        {
            b[i] = 1.0;
            for (int j = 0; j < MATRIZ_DIM; j++)
            {
                int index = i * MATRIZ_DIM + j;
                A[index] = i;
            }
        }
        fmt::print("MATRIZ_DIM: {}, NPROCS: {}, rows_per_rank: {}\n", MATRIZ_DIM, nprocs, rows_per_rank);
    }

    // Comunicación colectiva: Distribuir datos a todos los procesos
    
    // Todos reciben el vector 'b' completo desde el rank 0
    MPI_Bcast(b.data(), MATRIZ_DIM, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    // El rank 0 reparte la matriz 'A' en pedazos variables a 'A_local' de cada proceso
    MPI_Scatterv(
        rank == 0 ? A.data() : nullptr, // Buffer de envío (solo válido en root)
        sendcounts_A.data(),            // Arreglo de cantidades a enviar
        displs_A.data(),                // Arreglo de desplazamientos
        MPI_DOUBLE,                     // Tipo de dato a enviar
        A_local.data(),                 // Buffer de recepción (donde llega el pedazo)
        filas_locales * MATRIZ_DIM,     // Cantidad que recibe ESTE proceso
        MPI_DOUBLE,                     // Tipo de dato a recibir
        0,                              // RANK root
        MPI_COMM_WORLD                  // Comunicador
    );


    if (filas_locales > 0) 
    {
        multiplicar_matriz_vector(A_local, b, x_local, filas_locales, MATRIZ_DIM);
    }
    
    // Cada proceso entrega su 'x_local' y el rank 0 lo ensambla en el 'x' global
    MPI_Gatherv(
        x_local.data(),                 // Buffer que envía el proceso
        filas_locales,                  // Cuánto envía este proceso
        MPI_DOUBLE,                     // Tipo de dato enviado
        rank == 0 ? x.data() : nullptr, // Buffer donde aterriza todo (solo en root)
        recvcounts_x.data(),            // Arreglo de cuántos recibe de cada uno
        displs_x.data(),                // Arreglo de dónde colocar cada pedazo
        MPI_DOUBLE,                     // Tipo de dato recibido
        0,                              // RANK root
        MPI_COMM_WORLD                  // Comunicador
    );

    
    if (rank == 0)
    {
        fmt::print("Resultado global de la multiplicacion:\n");
        imprimir_vector(x, MATRIZ_DIM, 1);
    }

    MPI_Finalize();
    return 0;
}