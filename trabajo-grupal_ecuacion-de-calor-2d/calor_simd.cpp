#include "ecuacion_calor.h"
#include <cmath>
#include <immintrin.h>

double calcular_calor_simd(const ParametrosCalor &params, const std::vector<double> &u_old, std::vector<double> &u_new)
{
    double hx = params.lx / params.nx;
    double hy = params.ly / params.ny;
    double h2 = hx * hy;

    double constante = (params.alpha * params.dt) / h2;
    double suma_residuos = 0.0;

    // Copiar bordes fijos
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

    __m256d v_const = _mm256_set1_pd(constante);
    __m256d v_cuatro = _mm256_set1_pd(4.0);
    __m256d v_suma_residuos = _mm256_setzero_pd();

    for (int j = 1; j < params.ny - 1; j++)
    {
        int i = 1;
        // Procesar en bloques de 4 doubles (256 bits) usando AVX2
        for (; i <= params.nx - 5; i += 4)
        {
            int centro = j * params.nx + i;
            int arriba = (j - 1) * params.nx + i;
            int abajo = (j + 1) * params.nx + i;
            int izq = j * params.nx + (i - 1);
            int der = j * params.nx + (i + 1);

            __m256d v_centro = _mm256_loadu_pd(&u_old[centro]);
            __m256d v_arriba = _mm256_loadu_pd(&u_old[arriba]);
            __m256d v_abajo = _mm256_loadu_pd(&u_old[abajo]);
            __m256d v_izq = _mm256_loadu_pd(&u_old[izq]);
            __m256d v_der = _mm256_loadu_pd(&u_old[der]);

            // Sumar vecinos: arriba + abajo + izq + der
            __m256d v_vecinos = _mm256_add_pd(_mm256_add_pd(v_arriba, v_abajo), _mm256_add_pd(v_izq, v_der));

            // 4.0 * centro
            __m256d v_4centro = _mm256_mul_pd(v_cuatro, v_centro);

            // vecinos - 4*centro
            __m256d v_laplaciano = _mm256_sub_pd(v_vecinos, v_4centro);

            // centro + constante * laplaciano
            __m256d v_nuevo = _mm256_add_pd(v_centro, _mm256_mul_pd(v_const, v_laplaciano));

            // Guardar resultado
            _mm256_storeu_pd(&u_new[centro], v_nuevo);

            // Calcular residuo: diff = v_nuevo - v_centro
            __m256d v_diff = _mm256_sub_pd(v_nuevo, v_centro);
            __m256d v_diff_sq = _mm256_mul_pd(v_diff, v_diff);
            v_suma_residuos = _mm256_add_pd(v_suma_residuos, v_diff_sq);
        }

        for (; i < params.nx - 1; i++)
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

    // Sumar horizontalmente el registro AVX v_suma_residuos
    double temp_residuos[4];
    _mm256_storeu_pd(temp_residuos, v_suma_residuos);
    suma_residuos += temp_residuos[0] + temp_residuos[1] + temp_residuos[2] + temp_residuos[3];

    return std::sqrt(suma_residuos / (params.nx * params.ny));
}
