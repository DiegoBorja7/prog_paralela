#ifndef CALOR_SERIAL_H
#define CALOR_SERIAL_H

#include <vector>
#include <cstdint>


void iniciar_calor(std::vector<double>& u, int nx, int ny);

double calor_serial(const std::vector<double>& u_old, 
    std::vector<double>& u_new, 
    int nx, int ny, double lx, double ly,
     double alpha, double dt);

void acotado(const std::vector<double>& u,
     uint32_t* pixel_buffer, int nx, int ny,
      uint32_t width, uint32_t height);

#endif
