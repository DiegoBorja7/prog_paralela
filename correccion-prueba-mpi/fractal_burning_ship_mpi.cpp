#include "fractal_burning_ship_mpi.h"
#include "palette.h"
#include <cmath>

void burning_ship_mpi(double x_min, double y_min, double x_max, double y_max,
               uint32_t width, uint32_t height,
               uint32_t row_start, uint32_t row_end,
               uint32_t *pixel_buffer,
               int max_iteraciones,
               uint32_t *local_hist)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    // Inicializar el histograma local en 0
    for (int i = 0; i < 16; i++) {
        local_hist[i] = 0;
    }

    for (uint32_t j = row_start; j < row_end; j++)
    {
        double y = y_max - j * dy; // y invertido para coordenadas cartesianas
        
        for (uint32_t i = 0; i < width; i++)
        {
            double x = x_min + i * dx;
            
            double zr = 0.0;
            double zi = 0.0;
            int iter = 0;

            // Condición de escape: |z|^2 > 4
            while ((zr * zr + zi * zi) <= 4.0 && iter < max_iteraciones)
            {
                double abs_zr = std::abs(zr);
                double abs_zi = std::abs(zi);
                
                // Zn+1 = (|Re(Zn)| + i|Im(Zn)|)^2 + c
                double temp_r = abs_zr * abs_zr - abs_zi * abs_zi + x;
                double temp_i = 2.0 * abs_zr * abs_zi + y;
                
                zr = temp_r;
                zi = temp_i;
                
                iter++;
            }

            uint32_t color = 0xFF000000; // Negro por defecto (no escapó)
            
            if (iter < max_iteraciones) {
                // Escapó
                color = color_ramp2[iter % PALETTE_SIZE];
                
                // Actualizar histograma según rúbrica: local_hist[iter*16/max_iter]++
                int bin = (iter * 16) / max_iteraciones;
                if (bin >= 16) bin = 15; // Por seguridad
                local_hist[bin]++;
            }

            // Guardar en el buffer local (indexación relativa a row_start)
            pixel_buffer[(j - row_start) * width + i] = color;
        }
    }
}
