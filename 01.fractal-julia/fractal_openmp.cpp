#include "fractal_openmp.h"
#include "palette.h"

#include <cstdint>
#include <complex>
#include <cmath>
#include <cstring>
#include <immintrin.h>
#include <omp.h>

extern int max_iteraciones;
extern std::complex<double> c;

#pragma omp declare simd
uint32_t acotado_openmp(double x, double y)
{
    int iteraciones = 1;

    double zr = x;
    double zi = y;

    while ((zr * zr + zi * zi) < 4.0 && iteraciones < max_iteraciones)
    {
        // Zn+1 = Zn^2 + c

        double dr = zr * zr - zi * zi + c.real();
        double di = 2 * zr * zi + c.imag();

        zr = dr;
        zi = di;

        iteraciones++;
    }

    if (iteraciones < max_iteraciones)
    {
        int index = iteraciones % PALETTE_SIZE;
        return color_ramp3[index]; //| 0xFF000000; // Agregar canal
    }
    return 0XFF000000; // Negro para los puntos que pertenecen al conjunto de Julia
}

void julia_openmp_regiones(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

#pragma omp parallel
    {
        int thread_count = omp_get_num_threads();
        int thread_id = omp_get_thread_num();

        int delta = std::ceil(width * 1.0 / thread_count);
        int start = thread_id * delta;
        int end = (thread_id + 1) * delta;
        if (thread_id == thread_count - 1)
        {
            end = width; //
        }

        for (int i = start; i < end; i++)
        {
            for (int j = 0; j < height; j++)
            {
                double x = x_min + i * dx;
                double y = y_min + j * dy;

                std::complex<double> z(x, y);

                auto color = acotado_openmp(x, y);

                pixel_buffer[j * width + i] = color;
            }
        }
    }
}
void julia_openmp_for(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    const int w = static_cast<int>(width);
    const int h = static_cast<int>(height);

#pragma omp parallel for schedule(static)
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j++)
        {
            double x = x_min + i * dx;
            double y = y_min + j * dy;

            auto color = acotado_openmp(x, y);

            pixel_buffer[j * width + i] = color;
        }
    }
}

void julia_openmp_for_simd(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    std::memset(pixel_buffer, 0xFF000000, width * height * sizeof(uint32_t));

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    const int w = static_cast<int>(width);
    const int h = static_cast<int>(height);
    __m256 xmin = _mm256_set1_ps(static_cast<float>(x_min));
    __m256 ymax = _mm256_set1_ps(static_cast<float>(y_max));
    __m256 sxscale = _mm256_set1_ps(static_cast<float>(dx));
    __m256 syscale = _mm256_set1_ps(static_cast<float>(dy));
    __m256 c_real = _mm256_set1_ps(static_cast<float>(c.real()));
    __m256 c_imag = _mm256_set1_ps(static_cast<float>(c.imag()));
    __m256 max_norma = _mm256_set1_ps(4.0f);
    __m256 one = _mm256_set1_ps(1.0f);

#pragma omp parallel for schedule(static)
    for (int i = 0; i < w; i++)
    {
        for (int j = 0; j < h; j += 8)
        {
            __m256 mx = _mm256_set1_ps(static_cast<float>(i));
            __m256 my = _mm256_set_ps(
                static_cast<float>(j + 7), static_cast<float>(j + 6), static_cast<float>(j + 5), static_cast<float>(j + 4),
                static_cast<float>(j + 3), static_cast<float>(j + 2), static_cast<float>(j + 1), static_cast<float>(j + 0));

            __m256 cr = _mm256_add_ps(xmin, _mm256_mul_ps(mx, sxscale));
            __m256 ci = _mm256_sub_ps(ymax, _mm256_mul_ps(my, syscale));

            int iter = 1;
            __m256 mk = _mm256_set1_ps(static_cast<float>(iter));
            __m256 zr = cr;
            __m256 zi = ci;

            while (iter < max_iteraciones)
            {
                __m256 zr2 = _mm256_mul_ps(zr, zr);
                __m256 zi2 = _mm256_mul_ps(zi, zi);
                __m256 zrzi = _mm256_mul_ps(zr, zi);

                zr = _mm256_add_ps(_mm256_sub_ps(zr2, zi2), c_real);
                zi = _mm256_add_ps(_mm256_add_ps(zrzi, zrzi), c_imag);

                zr2 = _mm256_mul_ps(zr, zr);
                zi2 = _mm256_mul_ps(zi, zi);
                __m256 norma2 = _mm256_add_ps(zr2, zi2);

                __m256 mask = _mm256_cmp_ps(norma2, max_norma, _CMP_LE_OS);
                mk = _mm256_add_ps(_mm256_and_ps(mask, one), mk);

                if (_mm256_testz_ps(mask, _mm256_set1_ps(-1.0f)))
                {
                    break;
                }

                iter++;
            }

            float d[8];
            _mm256_storeu_ps(d, mk);

            for (int it = 0; it < 8; it++)
            {
                int row = j + it;
                if (row >= h)
                {
                    continue;
                }

                int index = row * w + i;
                if (d[it] < max_iteraciones)
                {
                    int color_index = static_cast<int>(d[it]) % PALETTE_SIZE;
                    pixel_buffer[index] = color_ramp3[color_index];
                }
                else
                {
                    pixel_buffer[index] = 0xFF000000;
                }
            }
        }
    }
}
