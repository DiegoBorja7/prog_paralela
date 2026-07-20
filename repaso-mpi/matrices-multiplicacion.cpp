#include <iostream>
#include <fmt/core.h>
#include <mpi.h>
#include <vector>
#include <cmath> // Necesario para std::ceil
#include <algorithm>
 
#define MATRIZ_DIM 25 // Definido con Z
 
void imprimir_matriz(const std::vector<double> &A, int rows, int cols)
{
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            int index = i * cols + j;
            fmt::print("{:.2f} ", A[index]);
        }
        fmt::print("\n");
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

void imprimir_vector_2(const std::vector<double>& x, int max_print = 30)
{
    int n = static_cast<int>(x.size());

    if (n <= max_print)
    {
        for (int i = 0; i < n; i++)
        {
            fmt::print("{:.2f}\n", x[i]);
        }
        return;
    }

    fmt::print("Mostrando primeros 10 y ultimos 10 valores:\n");
    for (int i = 0; i < 10; i++)
    {
        fmt::print("x[{}] = {:.2f}\n", i, x[i]);
    }

    fmt::print("...\n");

    for (int i = n - 10; i < n; i++)
    {
        fmt::print("x[{}] = {:.2f}\n", i, x[i]);
    }
}

void generar_bloque_matriz(std::vector<double>& A_local, int inicio, int filas, int n)
{
    for (int i = 0; i < filas; i++)
    {
        int fila_global = inicio + i;

        for (int j = 0; j < n; j++)
        {
            int index = i * n + j;
            A_local[index] = fila_global;
        }
    }
}

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

int filas_para_rank(int n, int nprocs, int rank)
{
    int base = n / nprocs;
    int resto = n % nprocs;
    return base + (rank < resto ? 1 : 0);
}

int offset_para_rank(int n, int nprocs, int rank)
{
    int base = n / nprocs;
    int resto = n % nprocs;
    return rank * base + std::min(rank, resto);
}

int valor_fila_matriz(int fila_global, int n)
{
    if (fila_global >= n)
    {
        return 0;
    }
    return fila_global;
}

//version 1 donde para N fijo se calcula filas por rank y se envian bloques de la matriz A, el vector b se envia completo a cada rank
void main_1(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
 
    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
    int rows_per_rank = std::ceil((MATRIZ_DIM) * 1.0 / nprocs);
 
    auto filas_para_rank = [&](int proc)
    {
        int inicio = proc * rows_per_rank;
        if (inicio >= MATRIZ_DIM)
        {
            return 0;
        }
        return std::min(rows_per_rank, MATRIZ_DIM - inicio);
    };
 
    auto offset_para_rank = [&](int proc)
    {
        return proc * rows_per_rank;
    };
 
    // Todo lo que hace el proceso 0 debe ir dentro de este bloque
    if (rank == 0)
    {
        std::vector<double> A(MATRIZ_DIM * MATRIZ_DIM);
        std::vector<double> b(MATRIZ_DIM);              
        std::vector<double> x(MATRIZ_DIM);            
  
        for (int i = 0; i < MATRIZ_DIM; i++)
        {
            for (int j = 0; j < MATRIZ_DIM; j++)
            {
                int index = i * MATRIZ_DIM + j; // Cálculo del índice para la matriz A
                A[index] = i;
            }
        }
 
        for (int i = 0; i < MATRIZ_DIM; i++)
        {
            b[i] = 1.0;
        }
 
        int padding = rows_per_rank * nprocs - MATRIZ_DIM;
 
        fmt::print("MATRIZ_DIM: {}, NPROCS: {}, rows_per_rank: {}, padding: {}\n",
                   MATRIZ_DIM, nprocs, rows_per_rank, padding);
 
        // enviar dimesiones y datos
        for (int i = 1; i < nprocs; i++)
        {
            int filas = filas_para_rank(i);
            int inicio = offset_para_rank(i);
 
            // enviar dimensiones
            std::vector<int> data = {MATRIZ_DIM, filas};
 
            MPI_Send(
                data.data(),  
                2,            
                MPI_INT,      
                i,      
                0,            
                MPI_COMM_WORLD
            );
 
            const double *buffer = A.data();
            MPI_Send(
                filas > 0 ? &buffer[inicio * MATRIZ_DIM] : buffer,
                filas * MATRIZ_DIM,                                
                MPI_DOUBLE,                                        
                i,                                                
                0,                                                
                MPI_COMM_WORLD                                    
            );
            MPI_Send(
                b.data(),      // buffer
                MATRIZ_DIM,    // count
                MPI_DOUBLE,    // Tipo de datos
                i,             // RANK destino
                0,             // TAG
                MPI_COMM_WORLD // Grupo
            );
        }
 
        int filas_locales = filas_para_rank(0);
        std::vector<double> A_local(A.begin(), A.begin() + filas_locales * MATRIZ_DIM);
        std::vector<double> x_local(filas_locales);
        multiplicar_matriz_vector(A_local, b, x_local, filas_locales, MATRIZ_DIM);
 
        for (int i = 0; i < filas_locales; i++)
        {
            x[i] = x_local[i];
        }
 
        for (int i = 1; i < nprocs; i++)
        {
            int filas = filas_para_rank(i);
            std::vector<double> x_parcial(filas);
 
            MPI_Recv(
                x_parcial.data(),
                filas,
                MPI_DOUBLE,
                i,
                1,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE);
 
            int inicio = offset_para_rank(i);
            for (int j = 0; j < filas; j++)
            {
                x[inicio + j] = x_parcial[j];
            }
        }
 
        fmt::print("Resultado global de la multiplicacion:\n");
        imprimir_vector(x, MATRIZ_DIM, 1);
        // fmt::print("RANK: {}, {} x {}\n", rank, rows_per_rank, MATRIZ_DIM);
    }
    else
    {
        std::vector<int> data_rec(2);
 
        MPI_Recv(
            data_rec.data(), // buffer
            2,               // count
            MPI_INT,         // Tipo de datos
            0,               // RANK origen
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
 
        int matrix_dim = data_rec[0];
        int rows = data_rec[1];
 
        fmt::print("RANK: {}, {} x {}\n", rank, rows, matrix_dim);
 
        std::vector<double> A_local(rows * matrix_dim);
        std::vector<double> b_local(matrix_dim);
        MPI_Recv(
            A_local.data(),
            rows * matrix_dim,
            MPI_DOUBLE,
            0,
            0,              
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
 
        MPI_Recv(
            b_local.data(),
            matrix_dim,    
            MPI_DOUBLE,    
            0,              
            0,              
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE);
 
        if (rank == 1)
        {
            imprimir_vector(A_local, rows, matrix_dim);
        }
 
        std::vector<double> x_local(rows);
        multiplicar_matriz_vector(A_local, b_local, x_local, rows, matrix_dim);
 
        MPI_Send(
            x_local.data(),
            rows,
            MPI_DOUBLE,
            0,
            1,
            MPI_COMM_WORLD);
 
        if (rank == 1)
        {
            fmt::print("Resultado de la multiplicacion:\n");
            imprimir_vector(x_local, rows, 1);
        }
    }
 
    MPI_Finalize();
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int n = MATRIZ_DIM;

    if (argc > 1)
    {
        n = std::atoi(argv[1]);
    }

    if (n <= 0)
    {
        if (rank == 0)
        {
            fmt::print("N debe ser mayor que 0.\n");
        }

        MPI_Finalize();
        return 1;
    }

    int padding = (nprocs - (n % nprocs)) % nprocs;
    int n_padded = n + padding;
    int rows_per_rank = n_padded / nprocs;

    int filas_locales = rows_per_rank;
    int inicio_local = rank * rows_per_rank;

    if (rank == 0)
    {
        fmt::print("MATRIZ_DIM: {}, NPROCS: {}, padding: {}, n_padded: {}, rows_per_rank: {}\n",
                   n, nprocs, padding, n_padded, rows_per_rank);

        for (int p = 0; p < nprocs; p++)
        {
            int inicio = p * rows_per_rank;
            fmt::print(
                "Rank {} -> inicio: {}, filas: {}\n",
                p,
                inicio,
                rows_per_rank
            );
        }

        std::vector<double> b(n, 1.0);
        std::vector<double> x(n, 0.0);

        for (int p = 1; p < nprocs; p++)
        {
            int filas = rows_per_rank;
            int inicio = p * rows_per_rank;

            std::vector<int> data = {n, filas, inicio};

            MPI_Send(data.data(), 3, MPI_INT, p, 0, MPI_COMM_WORLD);

            MPI_Send(b.data(), n, MPI_DOUBLE, p, 1, MPI_COMM_WORLD);
        }

        std::vector<double> A_local(filas_locales * n);
        std::vector<double> x_local(filas_locales);

        for (int i = 0; i < filas_locales; i++)
        {
            int fila_global = inicio_local + i;
            for (int j = 0; j < n; j++)
            {
                A_local[i * n + j] = valor_fila_matriz(fila_global, n);
            }
        }
        multiplicar_matriz_vector(A_local, b, x_local, filas_locales, n);

        for (int i = 0; i < filas_locales; i++)
        {
            int fila_global = inicio_local + i;
            if (fila_global < n)
            {
                x[fila_global] = x_local[i];
            }
        }

        for (int p = 1; p < nprocs; p++)
        {
            int filas = rows_per_rank;
            int inicio = p * rows_per_rank;

            std::vector<double> x_parcial(filas);

            MPI_Recv(
                x_parcial.data(),
                filas,
                MPI_DOUBLE,
                p,
                2,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );

            for (int i = 0; i < filas; i++)
            {
                int fila_global = inicio + i;
                if (fila_global < n)
                {
                    x[fila_global] = x_parcial[i];
                }
            }
        }

        fmt::print("Resultado global de la multiplicacion A * b:\n");
        imprimir_vector_2(x);
    }
    else
    {
        std::vector<int> data(3);

        MPI_Recv(
            data.data(),
            3,
            MPI_INT,
            0,
            0,
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );

        int matrix_dim = data[0];
        int filas = data[1];
        int inicio = data[2];

        fmt::print("Rank {} -> inicio: {}, filas: {}\n", rank, inicio, filas);

        if (filas > 0)
        {
            std::vector<double> b(matrix_dim);
            std::vector<double> A_local(filas * matrix_dim);
            std::vector<double> x_local(filas);

            MPI_Recv(
                b.data(),
                matrix_dim,
                MPI_DOUBLE,
                0,
                1,
                MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );

            for (int i = 0; i < filas; i++)
            {
                int fila_global = inicio + i;
                for (int j = 0; j < matrix_dim; j++)
                {
                    A_local[i * matrix_dim + j] = valor_fila_matriz(fila_global, matrix_dim);
                }
            }
            multiplicar_matriz_vector(A_local, b, x_local, filas, matrix_dim);

            MPI_Send(
                x_local.data(),
                filas,
                MPI_DOUBLE,
                0,
                2,
                MPI_COMM_WORLD
            );
        }
    }

    MPI_Finalize();
    return 0;
}
