#include <cuda_runtime.h>
#include <stdio.h>
#include <math.h>

// Kernel para convertir una imagen RGB a escala de grises
__global__ void grayscale_kernel(unsigned char *src_image, unsigned char *dst_image, int width, int height) {
    // Calculo del indice 1D
    int index = blockDim.x * blockIdx.x + threadIdx.x;
    int total_pixels = width * height;

    // Guardaespaldas para hilos fantasma
    if (index >= total_pixels)
        return;

    int pixel_idx = index * 3; // RGB = 3 bytes por pixel
    unsigned char r = src_image[pixel_idx];
    unsigned char g = src_image[pixel_idx + 1];
    unsigned char b = src_image[pixel_idx + 2];

    // Calculo del valor de gris usando la formula de luminosidad
    unsigned char gray = (unsigned char)(0.21f * r + 0.72f * g + 0.07f * b);

    dst_image[pixel_idx] = gray;
    dst_image[pixel_idx + 1] = gray;
    dst_image[pixel_idx + 2] = gray;
}

// Wrapper para enlazar con main.cpp
extern "C" void kernel_grayscale(unsigned char *src_image, unsigned char *dst_image, int width, int height) {
    size_t buffer_size = width * height * 3;

    // 1. Reservar memoria en el Device (GPU)
    unsigned char *d_src, *d_dst;
    cudaMalloc(&d_src, buffer_size);
    cudaMalloc(&d_dst, buffer_size);

    // 2. Transferir imagen Host -> Device
    cudaMemcpy(d_src, src_image, buffer_size, cudaMemcpyHostToDevice);

    // 3. Grid 1D (como pide el PDF)
    int thr_per_blk = 1024;
    int blk_in_grid = ceil(float(width * height) / thr_per_blk);

    // 4. Lanzamiento asincrono del kernel
    grayscale_kernel<<<blk_in_grid, thr_per_blk>>>(d_src, d_dst, width, height);
    
    // Sincronizar para evitar errores
    cudaDeviceSynchronize();

    // 5. Transferir resultados Device -> Host
    cudaMemcpy(dst_image, d_dst, buffer_size, cudaMemcpyDeviceToHost);

    // 6. Liberar memoria
    cudaFree(d_src);
    cudaFree(d_dst);
}