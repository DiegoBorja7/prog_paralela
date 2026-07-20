#ifndef CALOR_OPENMP_H
#define CALOR_OPENMP_H

#include <vector>

double calor_openmp(const std::vector<double>& u_old, 
    std::vector<double>& u_new, 
    int nx, int ny, double lx, double ly,
     double alpha, double dt);

#endif
