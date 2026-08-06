#include <iostream>
#include <omp.h>
#include <fmt/core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Wrapper de la funcion de CUDA
extern "C" void kernel_grayscale(unsigned char *src_image, unsigned char *dst_image, int width, int height);

// Solucion 1: OpenMP
void grayscale_openmp(unsigned char *src_image, unsigned char *dst_image, int width, int height) {
    int num_pixels = width * height;
    
    // Paralelismo a nivel de ciclo (for-paralelo)
    #pragma omp parallel for
    for (int i = 0; i < num_pixels; i++) {
        int index = i * 3; // RGB 3 channels
        unsigned char r = src_image[index];
        unsigned char g = src_image[index + 1];
        unsigned char b = src_image[index + 2];
        
        // Calculo del valor de gris usando la formula de luminosidad
        unsigned char gray = (unsigned char)(0.21f * r + 0.72f * g + 0.07f * b);
        
        dst_image[index] = gray;
        dst_image[index + 1] = gray;
        dst_image[index + 2] = gray;
    }
}

int main() {
    int width, height, channels;
    
    // Forzamos a que tenga 3 canales (RGB) en lugar de 4 (RGBA)
    uint8_t* rgb_pixels = stbi_load("image01.jpg", &width, &height, &channels, 3); 
    
    if (!rgb_pixels) {
        fmt::println("ERROR: No se pudo cargar image01.jpg. Asegurate de que la imagen exista en esta carpeta.");
        return 1;
    }
    
    fmt::println("Imagen cargada exitosamente: {}x{} pixeles.", width, height);
    
    size_t img_size = width * height * 3;
    uint8_t* gray_pixels_omp = (uint8_t*)malloc(img_size);
    uint8_t* gray_pixels_cuda = (uint8_t*)malloc(img_size);
    
    // ============================================
    // 1. OPENMP IMPLEMENTACION
    // ============================================
    fmt::println("Iniciando procesamiento con OpenMP...");
    double start_omp = omp_get_wtime();
    
    grayscale_openmp(rgb_pixels, gray_pixels_omp, width, height);
    
    double end_omp = omp_get_wtime();
    fmt::println("=> OpenMP finalizado. Tiempo: {:.5f} segundos", end_omp - start_omp);
    
    // Escritura de imagen OMP
    stbi_write_png("img-gris-omp.png", width, height, 3, gray_pixels_omp, width * 3);
    
    // ============================================
    // 2. CUDA IMPLEMENTACION
    // ============================================
    fmt::println("Iniciando procesamiento con CUDA...");
    double start_cuda = omp_get_wtime();
    
    kernel_grayscale(rgb_pixels, gray_pixels_cuda, width, height);
    
    double end_cuda = omp_get_wtime();
    fmt::println("=> CUDA finalizado (con transferencias). Tiempo: {:.5f} segundos", end_cuda - start_cuda);
    
    // Escritura de imagen CUDA
    stbi_write_png("img-gris-cuda.png", width, height, 3, gray_pixels_cuda, width * 3);
    
    // Liberacion de memoria
    stbi_image_free(rgb_pixels);
    free(gray_pixels_omp);
    free(gray_pixels_cuda);
    
    fmt::println("Ejecucion terminada. Revisa los archivos img-gris-omp.png e img-gris-cuda.png.");
    
    return 0;
}