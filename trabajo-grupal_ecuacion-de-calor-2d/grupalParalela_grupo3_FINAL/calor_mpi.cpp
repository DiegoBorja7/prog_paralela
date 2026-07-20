#include "calor_mpi.h"
#include <cmath>

double calor_mpi_step(const std::vector<double>& u_old, 
    std::vector<double>& u_new, 
    int nx, int local_ny, int global_ny, double lx, double ly,
     double alpha, double dt, bool is_top, bool is_bottom) {
    
    double hx = lx / nx;
    double hy = ly / global_ny;
    double h2 = hx * hy;
    double c = (alpha * dt) / h2;

    double dif_cuad = 0.0;

    int j_start = 1;
    if (is_top) {
        j_start = 2; 
        for (int i = 0; i < nx; i++) {
            u_new[1 * nx + i] = u_old[1 * nx + i];
        }
    }

    int j_end = local_ny;
    if (is_bottom) {
        j_end = local_ny - 1;
        for (int i = 0; i < nx; i++) {
            u_new[local_ny * nx + i] = u_old[local_ny * nx + i];
        }
    }

    for (int j = j_start; j <= j_end; j++) {
        u_new[j * nx + 0] = u_old[j * nx + 0];
        u_new[j * nx + nx - 1] = u_old[j * nx + nx - 1];

        for (int i = 1; i < nx - 1; i++) {
            int idx = j * nx + i;
            
            double u_ij = u_old[idx];
            double u_up = u_old[(j - 1) * nx + i];
            double u_down = u_old[(j + 1) * nx + i];
            double u_left = u_old[idx - 1];
            double u_right = u_old[idx + 1];

            double unew = u_ij + c * (u_right + u_left + u_down + u_up - 4.0 * u_ij);
            u_new[idx] = unew;

            double diff = unew - u_ij;
            dif_cuad += diff * diff;
        }
    }
    
    return dif_cuad;
}
