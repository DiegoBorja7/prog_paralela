#ifndef CALOR_MPI_H
#define CALOR_MPI_H

#include <vector>

double calor_mpi_step(const std::vector<double>& u_old, 
    std::vector<double>& u_new, 
    int nx, int local_ny, int global_ny, double lx, double ly,
     double alpha, double dt, bool is_top, bool is_bottom);

#endif
