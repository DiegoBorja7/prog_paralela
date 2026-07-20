#include "ecuacion_calor.h"
#include <cmath>
#include <mpi.h>
#include <vector>

double calcular_calor_mpi(const ParametrosCalor &params, const std::vector<double> &u_old, std::vector<double> &u_new, int rank, int nprocs)
{
    double hx = params.lx / params.nx;
    double hy = params.ly / params.ny;
    double h2 = hx * hy;
    double constante = (params.alpha * params.dt) / h2;

    int rows_per_proc = params.ny / nprocs;
    int remainder = params.ny % nprocs;

    int local_ny = rows_per_proc + (rank < remainder ? 1 : 0);
    int row_start = rank * rows_per_proc + (rank < remainder ? rank : remainder);
    int row_end = row_start + local_ny;

    // Intercambio de Ghost Zones (Bordes Fantasmas) con MPI_Sendrecv
    if (rank > 0)
    {
        MPI_Sendrecv(&u_old[row_start * params.nx], params.nx, MPI_DOUBLE, rank - 1, 0,
                     &const_cast<std::vector<double> &>(u_old)[(row_start - 1) * params.nx], params.nx, MPI_DOUBLE, rank - 1, 1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Enviamos nuestra última fila al rank siguiente, recibimos su fila 0
    if (rank < nprocs - 1)
    {
        MPI_Sendrecv(&u_old[(row_end - 1) * params.nx], params.nx, MPI_DOUBLE, rank + 1, 1,
                     &const_cast<std::vector<double> &>(u_old)[row_end * params.nx], params.nx, MPI_DOUBLE, rank + 1, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Copiar bordes fijos verticales locales
    for (int j = row_start; j < row_end; j++)
    {
        u_new[j * params.nx + 0] = u_old[j * params.nx + 0];
        u_new[j * params.nx + (params.nx - 1)] = u_old[j * params.nx + (params.nx - 1)];
    }

    // Si somos el rank 0, copiar el borde superior fijo
    if (rank == 0)
    {
        for (int i = 0; i < params.nx; i++)
            u_new[0 * params.nx + i] = u_old[0 * params.nx + i];
    }
    // Si somos el último rank, copiar el borde inferior fijo
    if (rank == nprocs - 1)
    {
        for (int i = 0; i < params.nx; i++)
            u_new[(params.ny - 1) * params.nx + i] = u_old[(params.ny - 1) * params.nx + i];
    }

    double local_suma_residuos = 0.0;

    // Calcular interior local
    int compute_start = (rank == 0) ? 1 : row_start;
    int compute_end = (rank == nprocs - 1) ? params.ny - 1 : row_end;

    for (int j = compute_start; j < compute_end; j++)
    {
        for (int i = 1; i < params.nx - 1; i++)
        {
            int centro = j * params.nx + i;
            int arriba = (j - 1) * params.nx + i;
            int abajo = (j + 1) * params.nx + i;
            int izq = j * params.nx + (i - 1);
            int der = j * params.nx + (i + 1);

            u_new[centro] = u_old[centro] + constante * (u_old[arriba] + u_old[abajo] + u_old[izq] + u_old[der] - 4.0 * u_old[centro]);

            double diff = u_new[centro] - u_old[centro];
            local_suma_residuos += diff * diff;
        }
    }

    // Reducción del residuo global
    double global_suma_residuos = 0.0;
    MPI_Allreduce(&local_suma_residuos, &global_suma_residuos, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);

    std::vector<int> recvcounts(nprocs);
    std::vector<int> displs(nprocs);

    for (int i = 0; i < nprocs; i++)
    {
        int r_ny = rows_per_proc + (i < remainder ? 1 : 0);
        recvcounts[i] = r_ny * params.nx;
        displs[i] = (i * rows_per_proc + (i < remainder ? i : remainder)) * params.nx;
    }

    // Usamos MPI_IN_PLACE porque cada proceso ya tiene su propio pedazo calculado en u_new
    MPI_Allgatherv(MPI_IN_PLACE, 0, MPI_DATATYPE_NULL,
                   &u_new[0], recvcounts.data(), displs.data(), MPI_DOUBLE, MPI_COMM_WORLD);

    return std::sqrt(global_suma_residuos / (params.nx * params.ny));
}
