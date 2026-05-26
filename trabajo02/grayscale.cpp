#include "grayscale.h"

#include <immintrin.h>
#include <omp.h>

namespace
{
uint8_t clamp_to_u8(float value)
{
    if (value < 0.0f)
    {
        return 0;
    }
    if (value > 255.0f)
    {
        return 255;
    }
    return static_cast<uint8_t>(value + 0.5f);
}
}

void rgba_to_gray_simd(const uint8_t* rgba_pixels, uint8_t* gray_pixels, int width, int height)
{
    const int total_pixels = width * height;

    const __m256 wr = _mm256_set1_ps(0.21f);
    const __m256 wg = _mm256_set1_ps(0.72f);
    const __m256 wb = _mm256_set1_ps(0.07f);

    int i = 0;
    alignas(32) float r[8];
    alignas(32) float g[8];
    alignas(32) float b[8];
    alignas(32) float gray[8];

    for (; i + 7 < total_pixels; i += 8)
    {
        for (int lane = 0; lane < 8; ++lane)
        {
            const int px = (i + lane) * 4;
            r[lane] = static_cast<float>(rgba_pixels[px + 0]);
            g[lane] = static_cast<float>(rgba_pixels[px + 1]);
            b[lane] = static_cast<float>(rgba_pixels[px + 2]);
        }

        const __m256 vr = _mm256_load_ps(r);
        const __m256 vg = _mm256_load_ps(g);
        const __m256 vb = _mm256_load_ps(b);

        __m256 vgray = _mm256_mul_ps(vr, wr);
        vgray = _mm256_add_ps(vgray, _mm256_mul_ps(vg, wg));
        vgray = _mm256_add_ps(vgray, _mm256_mul_ps(vb, wb));

        _mm256_store_ps(gray, vgray);

        for (int lane = 0; lane < 8; ++lane)
        {
            gray_pixels[i + lane] = clamp_to_u8(gray[lane]);
        }
    }

    for (; i < total_pixels; ++i)
    {
        const int px = i * 4;
        const float gval = 0.21f * rgba_pixels[px + 0] + 0.72f * rgba_pixels[px + 1] + 0.07f * rgba_pixels[px + 2];
        gray_pixels[i] = clamp_to_u8(gval);
    }
}

void rgba_to_gray_openmp_manual(const uint8_t* rgba_pixels, uint8_t* gray_pixels, int width, int height)
{
    const int total_pixels = width * height;

#pragma omp parallel
    {
        const int thread_id = omp_get_thread_num();
        const int thread_count = omp_get_num_threads();

        const int delta = total_pixels / thread_count;
        const int start = thread_id * delta;
        int end = (thread_id + 1) * delta;
        if (thread_id == thread_count - 1)
        {
            end = total_pixels;
        }

        for (int i = start; i < end; ++i)
        {
            const int px = i * 4;
            const float gval = 0.21f * rgba_pixels[px + 0] + 0.72f * rgba_pixels[px + 1] + 0.07f * rgba_pixels[px + 2];
            gray_pixels[i] = clamp_to_u8(gval);
        }
    }
}
