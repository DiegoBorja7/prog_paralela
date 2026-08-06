#include <cuda_runtime.h>
#include <stdio.h>
#include <math.h>

#define CHANNELS 4

// La matriz del Kernel Gaussiano
__constant__ int d_K[3][3] = {
    {1, 2, 1},
    {2, 4, 2},
    {1, 2, 1}};

__global__ void gaussian_blur_kernel(const unsigned char *src, unsigned char *dst, int width, int height)
{
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    int total_pixels = width * height;

    if (index >= total_pixels)
        return;

    int y = index / width;
    int x = index % width;

    int r = 0, g = 0, b = 0, a = 0;
    int weight_sum = 0;

    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            int cur_x = x + j;
            int cur_y = y + i;

            if (cur_x >= 0 && cur_x < width && cur_y >= 0 && cur_y < height)
            {
                int neighbor_weight = d_K[i + 1][j + 1];
                int neighbor_idx = (cur_y * width + cur_x) * CHANNELS;

                r += src[neighbor_idx] * neighbor_weight;
                g += src[neighbor_idx + 1] * neighbor_weight;
                b += src[neighbor_idx + 2] * neighbor_weight;
                a += src[neighbor_idx + 3] * neighbor_weight;

                weight_sum += neighbor_weight;
            }
        }
    }

    int pixel_idx = index * CHANNELS;

    if (weight_sum > 0)
    {
        dst[pixel_idx] = r / weight_sum;
        dst[pixel_idx + 1] = g / weight_sum;
        dst[pixel_idx + 2] = b / weight_sum;
        dst[pixel_idx + 3] = a / weight_sum;
    }
}

extern "C" double kernel_gaussian_blur(const unsigned char *src_image, unsigned char *dst_image, int width, int height)
{
    size_t buffer_size = width * height * CHANNELS;

    unsigned char *d_src, *d_dst;
    cudaMalloc(&d_src, buffer_size);
    cudaMalloc(&d_dst, buffer_size);

    cudaMemcpy(d_src, src_image, buffer_size, cudaMemcpyHostToDevice);

    int thr_per_blk = 1024;
    int blk_in_grid = ceil(float(width * height) / thr_per_blk);

    cudaEvent_t start, stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop);

    cudaEventRecord(start);
    gaussian_blur_kernel<<<blk_in_grid, thr_per_blk>>>(d_src, d_dst, width, height);
    cudaEventRecord(stop);

    cudaMemcpy(dst_image, d_dst, buffer_size, cudaMemcpyDeviceToHost);

    cudaEventSynchronize(stop);
    float milliseconds = 0;
    cudaEventElapsedTime(&milliseconds, start, stop);

    cudaFree(d_src);
    cudaFree(d_dst);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    return (double)milliseconds;
}