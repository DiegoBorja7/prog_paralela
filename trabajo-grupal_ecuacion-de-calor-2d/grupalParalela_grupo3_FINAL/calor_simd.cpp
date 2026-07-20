#include "calor_simd.h"
#include <cmath>
#include <immintrin.h>

double calor_simd(const std::vector<double>& u_old, 
    std::vector<double>& u_new, int nx, int ny, 
    double lx, double ly, double alpha, double dt) {
    double hx = lx / nx;
    double hy = ly / ny;
    double h2 = hx * hy;
    double c = (alpha * dt) / h2;

    double dif_cuad = 0.0;

    for (int j = 0; j < ny; j++) {
        u_new[j * nx + 0] = u_old[j * nx + 0];
        u_new[j * nx + nx - 1] = u_old[j * nx + nx - 1];
    }
    for (int i = 0; i < nx; i++) {
        u_new[0 * nx + i] = u_old[0 * nx + i];
        u_new[(ny - 1) * nx + i] = u_old[(ny - 1) * nx + i];
    }

    __m256d c_vec = _mm256_set1_pd(c);
    __m256d four_vec = _mm256_set1_pd(4.0);
    __m256d diff_sq_sum_vec = _mm256_setzero_pd();

    for (int j = 1; j < ny - 1; j++) {
        int i = 1;
        for (; i <= nx - 1 - 4; i += 4) {
            int idx = j * nx + i;
            
            __m256d u_ij = _mm256_loadu_pd(&u_old[idx]);
            __m256d u_up = _mm256_loadu_pd(&u_old[(j - 1) * nx + i]);
            __m256d u_down = _mm256_loadu_pd(&u_old[(j + 1) * nx + i]);
            __m256d u_left = _mm256_loadu_pd(&u_old[idx - 1]);
            __m256d u_right = _mm256_loadu_pd(&u_old[idx + 1]);

            __m256d sum_neighbors = _mm256_add_pd(
                _mm256_add_pd(u_right, u_left),
                _mm256_add_pd(u_down, u_up)
            );
            
            __m256d unew = _mm256_add_pd(u_ij, 
                _mm256_mul_pd(c_vec, 
                    _mm256_sub_pd(sum_neighbors, _mm256_mul_pd(four_vec, u_ij))
                )
            );
            
            _mm256_storeu_pd(&u_new[idx], unew);

            __m256d diff = _mm256_sub_pd(unew, u_ij);
            __m256d diff_sq = _mm256_mul_pd(diff, diff);
            diff_sq_sum_vec = _mm256_add_pd(diff_sq_sum_vec, diff_sq);
        }

        for (; i < nx - 1; i++) {
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
    
    double temp[4];
    _mm256_storeu_pd(temp, diff_sq_sum_vec);
    dif_cuad += temp[0] + temp[1] + temp[2] + temp[3];

    return std::sqrt(dif_cuad / (nx * ny));
}
