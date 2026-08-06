#include <mpi.h>
#include <iostream>
#include <vector>
#include <fmt/core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// Solucion 2: MPI
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int width = 0, height = 0, channels = 0;
    uint8_t* rgb_pixels = nullptr;
    uint8_t* gray_pixels = nullptr;

    // Solo el Maestro lee la imagen del disco
    if (rank == 0) {
        rgb_pixels = stbi_load("image01.jpg", &width, &height, &channels, 3);
        if (!rgb_pixels) {
            fmt::println("Error loading image01.jpg");
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        gray_pixels = (uint8_t*)malloc(width * height * 3);
    }

    // El maestro comunica (Broadcast) las dimensiones a los trabajadores
    MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Logica de distribucion (comunicacion colectiva ScatterV)
    int total_pixels = width * height;
    int pixels_per_proc = total_pixels / size;
    int remainder = total_pixels % size;

    std::vector<int> sendcounts(size);
    std::vector<int> displs(size);

    int sum = 0;
    for (int i = 0; i < size; i++) {
        // Los primeros 'remainder' rangos procesan 1 pixel extra para que cuadre todo
        sendcounts[i] = (pixels_per_proc + (i < remainder ? 1 : 0)) * 3; // *3 por los canales RGB
        displs[i] = sum;
        sum += sendcounts[i];
    }

    int recvcount = sendcounts[rank];
    std::vector<uint8_t> local_rgb(recvcount);
    std::vector<uint8_t> local_gray(recvcount);

    // ============================================
    // COMUNICACION COLECTIVA 1: Repartir el trabajo
    // ============================================
    double start_time = MPI_Wtime();
    MPI_Scatterv(rgb_pixels, sendcounts.data(), displs.data(), MPI_UINT8_T,
                 local_rgb.data(), recvcount, MPI_UINT8_T,
                 0, MPI_COMM_WORLD);

    // ============================================
    // PROCESAMIENTO DISTRIBUIDO
    // ============================================
    for (int i = 0; i < recvcount; i += 3) {
        unsigned char r = local_rgb[i];
        unsigned char g = local_rgb[i + 1];
        unsigned char b = local_rgb[i + 2];
        
        // Calculo del valor de gris usando la formula de luminosidad
        unsigned char gray = (unsigned char)(0.21f * r + 0.72f * g + 0.07f * b);
        
        local_gray[i] = gray;
        local_gray[i + 1] = gray;
        local_gray[i + 2] = gray;
    }

    // ============================================
    // COMUNICACION COLECTIVA 2: Recolectar resultados
    // ============================================
    MPI_Gatherv(local_gray.data(), recvcount, MPI_UINT8_T,
                gray_pixels, sendcounts.data(), displs.data(), MPI_UINT8_T,
                0, MPI_COMM_WORLD);

    // Solo el Maestro junta el rompecabezas y escribe el archivo final
    if (rank == 0) {
        double end_time = MPI_Wtime();
        stbi_write_png("img-gris-mpi.png", width, height, 3, gray_pixels, width * 3);
        fmt::println("=> MPI finalizado exitosamente. Tiempo total (con comunicaciones): {:.5f} segundos", end_time - start_time);
        fmt::println("Ejecucion terminada. Revisa el archivo img-gris-mpi.png.");
        stbi_image_free(rgb_pixels);
        free(gray_pixels);
    }

    MPI_Finalize();
    return 0;
}
