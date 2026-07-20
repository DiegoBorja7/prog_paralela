#include "ecuacion_calor.h"
#include <cmath>
#include <omp.h>

double calcular_calor_omp(const ParametrosCalor &params, const std::vector<double> &u_old, std::vector<double> &u_new)
{
    double hx = params.lx / params.nx;
    double hy = params.ly / params.ny;
    double h2 = hx * hy;

    double constante = (params.alpha * params.dt) / h2;
    double suma_residuos = 0.0;

    for (int i = 0; i < params.nx; i++)
    {
        u_new[0 * params.nx + i] = u_old[0 * params.nx + i];
        u_new[(params.ny - 1) * params.nx + i] = u_old[(params.ny - 1) * params.nx + i];
    }
    for (int j = 0; j < params.ny; j++)
    {
        u_new[j * params.nx + 0] = u_old[j * params.nx + 0];
        u_new[j * params.nx + (params.nx - 1)] = u_old[j * params.nx + (params.nx - 1)];
    }

// Paralelizar con OpenMP
#pragma omp parallel for reduction(+ : suma_residuos) collapse(2)
    for (int j = 1; j < params.ny - 1; j++)
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
            suma_residuos += diff * diff;
        }
    }

    return std::sqrt(suma_residuos / (params.nx * params.ny));
}