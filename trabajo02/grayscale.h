#ifndef GRAYSCALE_H
#define GRAYSCALE_H

#include <cstdint>

void rgba_to_gray_simd(const uint8_t* rgba_pixels, uint8_t* gray_pixels, int width, int height);
void rgba_to_gray_openmp_manual(const uint8_t* rgba_pixels, uint8_t* gray_pixels, int width, int height);

#endif
