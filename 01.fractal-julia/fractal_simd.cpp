#include "fractal_simd.h"
#include "palette.h"

#include <complex>

#include <cstring>

extern int max_iteraciones;
extern std::complex<double> c;

void julia_simd(double x_min, double y_min, double x_max, double y_max, uint32_t width, uint32_t height, uint32_t *pixel_buffer){
    std::memset(pixel_buffer, 0xFF000000, width * height * sizeof(uint32_t));
}