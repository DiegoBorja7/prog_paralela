#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <mpi.h>
#include <fmt/core.h>

std::vector<int> read_file()
{
    std::fstream fs("datos.txt", std::ios::in);
    std::string line;
    std::vector<int> ret;

    if (!fs.is_open())
    {
        fmt::print("Error: No se pudo abrir 'datos.txt'\n");
        return ret;
    }

    while (std::getline(fs, line))
    {
        if (!line.empty())
        {
            ret.push_back(std::stoi(line));
        }
    }
    fs.close();
    return ret;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int nprocs, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int N_real = 0;
    std::vector<int> data_global;

    if (rank == 0)
    {
        data_global = read_file();
        N_real = data_global.size();
    }

    // Todos reciben el tamaño real de los datos
    MPI_Bcast(&N_real, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (N_real == 0)
    {
        if (rank == 0)
            fmt::print("No hay datos para procesar.\n");
        MPI_Finalize();
        return 0;
    }

    // Cálculo seguro de padding
    int padding = (nprocs - (N_real % nprocs)) % nprocs;
    int items_per_rank = (N_real + padding) / nprocs;

    if (rank == 0)
    {
        for (int i = 0; i < padding; ++i)
        {
            data_global.push_back(-1);
        }
    }

    std::vector<int> local_data(items_per_rank);

    MPI_Scatter(
        rank == 0 ? data_global.data() : nullptr, items_per_rank, MPI_INT,
        local_data.data(), items_per_rank, MPI_INT,
        0, MPI_COMM_WORLD);

    std::vector<int> local_freq(101, 0);
    long long local_sum = 0;
    int local_min = 101;
    int local_max = -1;

    for (int i = 0; i < items_per_rank; i++)
    {
        int val = local_data[i];

        if (val >= 0 && val <= 100)
        {
            local_freq[val]++;
            local_sum += val;
            if (val < local_min)
                local_min = val;
            if (val > local_max)
                local_max = val;
        }
    }

    long long global_sum = 0;
    int global_min = 0;
    int global_max = 0;

    MPI_Reduce(&local_sum, &global_sum, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_min, &global_min, 1, MPI_INT, MPI_MIN, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_max, &global_max, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    std::vector<int> global_freq(101, 0);

    if (rank == 0)
    {

        for (int j = 0; j <= 100; j++)
        {
            global_freq[j] = local_freq[j];
        }

        for (int p = 1; p < nprocs; p++)
        {
            std::vector<int> recv_freq(101);
            MPI_Recv(recv_freq.data(), 101, MPI_INT, p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            for (int j = 0; j <= 100; j++)
            {
                global_freq[j] += recv_freq[j];
            }
        }
    }
    else
    {
        MPI_Send(local_freq.data(), 101, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (rank == 0)
    {

        fmt::print("+-------+----------+\n");
        fmt::print("| Valor | Conteo   |\n");
        fmt::print("+-------+----------+\n");

        for (int i = 0; i <= 100; i++)
        {

            if (global_freq[i] > 0)
            {
                fmt::print("| {:<5} | {:<8} |\n", i, global_freq[i]);
            }
        }
        fmt::print("+-------+----------+\n\n");

        double promedio = static_cast<double>(global_sum) / N_real;

        fmt::print("Promedio de los datos: {:.2f}\n", promedio);
        fmt::print("Valor minimo:          {}\n", global_min);
        fmt::print("Valor maximo:          {}\n", global_max);
    }

    MPI_Finalize();
    return 0;
}