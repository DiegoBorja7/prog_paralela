#include <cstdio>

const int PALETTE_SIZE = 16;

// Hace que esta variable este en la GPU
__constant__ unsigned int d_color_ramp[PALETTE_SIZE];

// este modificador me dice que solo es accesible desde la GPU
__device__
    uint32_t
    acotado(
        double cr, double ci,
        int max_iteraciones,
        double x, double y)
{

    int iter = 1;
    double zr = x;
    double zi = y;

    while (iter < max_iteraciones && (zr * zr + zi * zi) < 4.0)
    {
        double dr = zr * zr - zi * zi + cr;
        double di = 2.0 * zr * zi + ci;

        zr = dr;
        zi = di;

        iter++;
    }
    if (iter < max_iteraciones)
    {
        int index = iter % PALETTE_SIZE;
        return d_color_ramp[index];
    }
    return 0xFF000000;
}

__global__ void julia_kernel(double centro_real, double centro_img, int num_interaciones,
                             double x_min, double y_min, double x_max, double y_max,
                             uint32_t width,
                             uint32_t height,
                             uint32_t *pixel_buffer)
{
    double dx = (x_max - x_min) / width;
    double dy = (y_max - y_min) / height;

    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < width * height)
    {
        int i = index / width; // fila
        int j = index % width; // columna

        double x = x_min + j * dx;
        double y = y_max - i * dy;

        auto color = acotado(centro_real, centro_img, num_interaciones, x, y);

        pixel_buffer[i * width + j] = color;
    }
}

// ------------ Solo CPU --------------

void copiar_paleta(unsigned int *h_paleta)
{
    // Para copiar la paleta desde la CPU a la GPU
    cudaMemcpyToSymbol(d_color_ramp, h_paleta, PALETTE_SIZE * sizeof(unsigned int));
}

void julia_gpu(
    double centro_real, double centro_img, int num_interaciones,
    double x_min, double y_min, double x_max, double y_max,
    uint32_t width,
    uint32_t height,
    uint32_t *pixel_buffer)
{

    int threads_per_block = 1024;
    int blocks_per_grid = std::ceil((double)width * height / threads_per_block);

    julia_kernel<<<blocks_per_grid, threads_per_block>>>(
        centro_real, centro_img,
        num_interaciones,
        x_min, y_min,
        x_max, y_max,
        width, height,
        pixel_buffer);

    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) {
        printf("Error de CUDA al lanzar kernel: %s\n", cudaGetErrorString(err));
    }
}
