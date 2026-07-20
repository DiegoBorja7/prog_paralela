#ifndef CALOR_SIMD_H
#define CALOR_SIMD_H

#include <vector>

double calor_simd(const std::vector<double>& u_old, 
    std::vector<double>& u_new, 
    int nx, int ny, double lx, double ly,
     double alpha, double dt);

#endif
