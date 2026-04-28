#include "fractal_serial.h"
#include "palette.h"

#include <complex>

/**
 * Dado c, z0
 * z(n+1) = z(n)^2 + c
 * Si |z(n)| > 2, entonces z0 no pertenece al conjunto de Julia asociado a c
 * Si |z(n)| <= 2 para todo n, entonces z0 pertenece al conjunto de Julia asociado a c
 */
extern int max_iteraciones;
extern std::complex<double> c;

uint32_t acotado_1(std::complex<double> z0)
{
    int iteraciones = 1;
    std::complex<double> z = z0;
    while (std::abs(z) < 2.0 && iteraciones < max_iteraciones)
    {
        z = z * z + c;
        iteraciones++;
    }

    if (iteraciones < max_iteraciones)
    {
        int index = iteraciones % PALETTE_SIZE;
        return color_ramp[index] | 0xFF000000; // Agregar canal
    }
    return 0XFF000000; // Negro para los puntos que pertenecen al conjunto de Julia
}

uint32_t acotado_2(double x, double y)
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
        return color_ramp2[index]; //| 0xFF000000; // Agregar canal

    }
    return 0XFF000000; // Negro para los puntos que pertenecen al conjunto de Julia
}

void julia_serial_1(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = x_min + i * dx;
            double y = y_min + j * dy;

            std::complex<double> z(x, y);

            auto color = acotado_1(z);
                        
            pixel_buffer[j * width + i] = color;
        }
    }
}

void julia_serial_2(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            double x = x_min + i * dx;
            double y = y_min + j * dy;

            std::complex<double> z(x, y);

            auto color = acotado_2(x, y);

            pixel_buffer[j * width + i] = color;
        }
    }
}