#ifndef ECUACION_CALOR_H
#define ECUACION_CALOR_H

#include <vector>

struct ParametrosCalor {
    int nx = 1024;
    int ny = 1024;
    double lx = 1.0;
    double ly = 1.0;
    double alpha = 0.25;
    double dt = 5.0e-7;
    int max_iter = 1000;
    double tol = 1.0e-4;
};

// Inicializa la grilla con las condiciones de frontera (Dirichlet)
void inicializar_grilla(const ParametrosCalor& params, std::vector<double>& u);

// Funciones de cálculo para cada backend
double calcular_calor_serial(const ParametrosCalor& params, const std::vector<double>& u_old, std::vector<double>& u_new);
double calcular_calor_simd(const ParametrosCalor& params, const std::vector<double>& u_old, std::vector<double>& u_new);
double calcular_calor_omp(const ParametrosCalor& params, const std::vector<double>& u_old, std::vector<double>& u_new);
double calcular_calor_mpi(const ParametrosCalor& params, const std::vector<double>& u_local_old, std::vector<double>& u_local_new, int rank, int nprocs);

#endif // ECUACION_CALOR_H
