#include <iostream>
#include <fmt/core.h>

#include <vector>

#include <mpi.h>

#define DIM 13

void imprimir_matriz(const std::string& msg, const std::vector<float>& M, int rows, int cols)
{
    fmt::println("{} [", msg);

    for (int i = 0; i < rows; i++) {
        fmt::print("{:>2}: ", i);
        for (int j = 0; j < cols; j++) {
            int index = i * cols + j;

            fmt::print("{:4.1f} ", M[index]);
        }
        fmt::println("");
    }
    fmt::println("]");
}

int main(int argc, char** argv) {

    MPI_Init(&argc, &argv);
    int nprocs;
    int rank;

    //-ranks
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::vector<float> U;
    std::vector<float> V(DIM);

    std::vector<float> M;

    int rows_alloc = DIM;
    int padding = 0;

    if(DIM % nprocs!=0) {
        rows_alloc = std::ceil(DIM / (float )nprocs) * nprocs;
        padding = rows_alloc - DIM;
    }

    int rows_per_rank = rows_alloc / nprocs;

    if(rank==0) {

        fmt::println( "DIM={}, rows_alloc={}, padding={}", DIM, rows_alloc, padding );

        U.resize(rows_alloc, 0); 

        M.resize(rows_alloc*DIM, 0);

        for(int i=0;i<DIM;i++) {
            U[i] = 1;
            V[i] = 2;
        }
    }

    //enviar vector V a todos los RANKs
    MPI_Bcast( V.data(), DIM, MPI_FLOAT, 0, MPI_COMM_WORLD );

    //enviar porción del vector U a los RANKs
    std::vector<float> U_local(rows_per_rank);

    MPI_Scatter( 
        U.data(), rows_per_rank, MPI_FLOAT,
        U_local.data(), rows_per_rank, MPI_FLOAT, 
        0, MPI_COMM_WORLD 
    );

    //calcular la porción en cada RANK
    std::vector<float> M_local(rows_per_rank*DIM);

    int filas = rows_per_rank;
    if (rank == nprocs - 1) {
        filas = filas - padding;
    }

    for (int i = 0; i < filas; i++) {
        for (int j = 0; j < DIM; j++) {
            M_local[i * DIM + j] = U_local[i] * V[j];
        }
    }

    // recibir de los RANKs al RANK_0
    MPI_Gather(
        M_local.data(), rows_per_rank*DIM, MPI_FLOAT,
        M.data(), rows_per_rank*DIM, MPI_FLOAT,
        0, MPI_COMM_WORLD 
    );

    //imprimir el resultado
    if(rank==0) {
        imprimir_matriz("resultado ", M, DIM, DIM );
    }

    MPI_Finalize();

    return 0;
}