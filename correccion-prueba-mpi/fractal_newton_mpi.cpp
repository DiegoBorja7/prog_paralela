#include "fractal_newton_mpi.h"
#include "palette.h"
#include <complex>
#include <cmath>

// Las 3 raíces de z^3 - 1 = 0
const std::complex<double> w0(1.0, 0.0);
const std::complex<double> w1(-0.5, std::sqrt(3.0) / 2.0);
const std::complex<double> w2(-0.5, -std::sqrt(3.0) / 2.0);

const double epsilon = 1e-4;

uint32_t color_newton(int root, int iteraciones)
{
    int index = (root * 5 + iteraciones) % PALETTE_SIZE;
    return color_ramp[index];
}

ComputeResult newton_mpi(double x_min, double y_min, double x_max, double y_max,
                         uint32_t width, uint32_t height,
                         uint32_t row_start, uint32_t row_end,
                         uint32_t *pixel_buffer,
                         int max_iteraciones)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    uint32_t total_iters = 0;

    for (uint32_t j = row_start; j < row_end; j++)
    {
        double y = y_max - j * dy;
        for (uint32_t i = 0; i < width; i++)
        {
            double x = x_min + i * dx;
            std::complex<double> z(x, y);

            int iter = 0;
            int root = -1;
            bool escaped = false;

            while (iter < max_iteraciones)
            {
                if (std::abs(z) > 2.0)
                {
                    escaped = true;
                    break;
                }

                if (std::abs(z - w0) < epsilon)
                {
                    root = 0;
                    break;
                }
                if (std::abs(z - w1) < epsilon)
                {
                    root = 1;
                    break;
                }
                if (std::abs(z - w2) < epsilon)
                {
                    root = 2;
                    break;
                }

                // Evitar división por cero
                if (std::abs(z) < 1e-6)
                {
                    break;
                }

                // z_{n+1} = z_n - (z_n^3 - 1) / (3 * z_n^2)
                std::complex<double> z2 = z * z;
                std::complex<double> z3 = z2 * z;
                z = z - (z3 - 1.0) / (3.0 * z2);

                iter++;
            }

            total_iters += iter;

            uint32_t color = 0xFF000000; // Negro por defecto
            if (root != -1 && !escaped)
            {
                color = color_newton(root, iter);
            }

            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }

    return {total_iters};
}