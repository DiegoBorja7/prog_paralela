#include "fractal_simd.h"
#include "palette.h"

#include <complex>

#include <cstring>

#include <immintrin.h>

extern int max_iteraciones;
extern std::complex<double> c;

void julia_simd(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    std::memset(pixel_buffer, 0xFF000000, width * height * sizeof(uint32_t));

    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    // Cargar los valores iniciales en registros SIMD
    //(xmin, xmin, xmin, xmin, xmin, xmin, xmin, xmin)
    //(-1.5,-1.5,-1.5,-1.5,-1.5,-1.5,-1.5,-1.5)
    __m256 xmin = _mm256_set1_ps(x_min);

    // Cargar los valores iniciales en registros SIMD
    //(ymax, ymax, ymax, ymax, ymax, ymax, ymax, ymax)
    //(1,1,1,1,1,1,1,1)
    __m256 ymax = _mm256_set1_ps(y_max);

    __m256 sxscale = _mm256_set1_ps(dx); //(dx, dx, dx, dx, dx, dx, dx, dx)
    __m256 syscale = _mm256_set1_ps(dy); //(dy, dy, dy, dy, dy, dy, dy, dy)

    __m256 c_real = _mm256_set1_ps(c.real()); //(cx, cx, cx, cx, cx, cx, cx, cx)
    __m256 c_imag = _mm256_set1_ps(c.imag()); //(cy, cy, cy, cy, cy, cy, cy, cy)

    __m256 max_norma = _mm256_set1_ps(4.0f);

    __m256 one = _mm256_set1_ps(1.0f); // (1, 1, 1, 1, 1, 1, 1, 1)

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j += 8)
        {
            //(i, i, i, i, i, i, i, i)
            __m256 mx = _mm256_set1_ps(i);
            //(j+7, j+6, j+5, j+4, j+3, j+2, j+1, j+0)
            __m256 my = _mm256_set_ps(j + 7, j + 6, j + 5, j + 4, j + 3, j + 2, j + 1, j + 0);

            // xmin + mx*xscale --> (x0,x1,x2,x3,x4,x5,x6,x7) <-- real
            __m256 cr = _mm256_add_ps(xmin, _mm256_mul_ps(mx, sxscale));
            // ymax - my*yscale --> (y0,y1,y2,y3,y4,y5,y6,y7) <-- imaginario
            __m256 ci = _mm256_sub_ps(ymax, _mm256_mul_ps(my, syscale));

            // verificar si los 8 complejos (cr, ci) estan acotados o no
            int iter = 1;
            __m256 mk = _mm256_set1_ps(iter); // iteraciones

            __m256 zr = cr; // zr0, zr1, zr2, zr3, zr4, zr5, zr6, zr7
            __m256 zi = ci; // zi0, zi1, zi2, zi3, zi4, zi5, zi6, zi7

            while (iter < max_iteraciones)
            {

                __m256 zr2 = _mm256_mul_ps(zr, zr);  // zr^2
                __m256 zi2 = _mm256_mul_ps(zi, zi);  // zi^2
                __m256 zrzi = _mm256_mul_ps(zr, zi); // zr*zi

                zr = _mm256_add_ps(_mm256_sub_ps(zr2, zi2), c_real);   // zr^2 - zi^2 + c_real
                zi = _mm256_add_ps(_mm256_add_ps(zrzi, zrzi), c_imag); // 2*zr*zi + c_imag

                // --normas
                zr2 = _mm256_mul_ps(zr, zr);             // zr^2
                zi2 = _mm256_mul_ps(zi, zi);             // zi^2
                __m256 norma2 = _mm256_add_ps(zr2, zi2); // norma^2

                // si norma2 <= 4.0f entonces devuelve 0xFFFF0000 (AZUL) sino devuelve 0x0000000
                __m256 mask = _mm256_cmp_ps(norma2, max_norma, _CMP_LE_OS);

                mk = _mm256_add_ps(_mm256_and_ps(mask, one), mk); // si es menor o igual a 4.0f entonces devuelve 0xFFFF0000 (AZUL) sino devuelve 0x0000000

                if (_mm256_testz_ps(mask, _mm256_set1_ps(-1)))
                {

                    break;
                }

                iter++;
            }

            float d[8];
            _mm256_storeu_ps(d, mk); // almacena el resultado de mk

            for (int it = 0; it < 8; it++)
            {
                int index = (j + it) * width + i;

                if (index < width * height)
                {
                    if (d[it] < max_iteraciones)
                    {
                        int color_index = (int)d[it] % PALETTE_SIZE;
                        pixel_buffer[index] = color_ramp[color_index];
                    }
                    else
                    {
                        pixel_buffer[index] = 0xFF000000; // NEGRO
                    }
                }
            }
        }
    }
}