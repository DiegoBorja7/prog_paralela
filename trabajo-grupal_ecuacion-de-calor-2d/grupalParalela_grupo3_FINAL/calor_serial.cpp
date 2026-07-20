#include "calor_serial.h"
#include "palette.h"
#include <cmath>
#include <algorithm>

void iniciar_calor(std::vector<double>& u, int nx, int ny) {
    std::fill(u.begin(), u.end(), 0.0);
    for (int i = 0; i < nx; i++) {
        u[i] = 100.0;
    }
}

double calor_serial(const std::vector<double>& u_old, 
    std::vector<double>& u_new,
     int nx, 
     int ny, 
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

    for (int j = 1; j < ny - 1; j++) {
        for (int i = 1; i < nx - 1; i++) {
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
    
    return std::sqrt(dif_cuad / (nx * ny));
}

void acotado(const std::vector<double>& u, uint32_t* pixel_buffer,
     int nx, int ny, uint32_t width, uint32_t height) {
    double scale_x = (double)nx / width;
    double scale_y = (double)ny / height;

    for (uint32_t py = 0; py < height; py++) {
        for (uint32_t px = 0; px < width; px++) {
            int gx = std::min((int)(px * scale_x), nx - 1);
            int gy = std::min((int)(py * scale_y), ny - 1);
            
            double temp = u[gy * nx + gx];
            int color_idx = (int)((temp / 100.0) * (PALETTE_SIZE - 1));
            color_idx = std::max(0, std::min(color_idx, PALETTE_SIZE - 1));
            
            pixel_buffer[py * width + px] = color_ramp3[color_idx];
        }
    }
}
