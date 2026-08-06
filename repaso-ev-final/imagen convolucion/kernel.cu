#include <stdio.h>

#define image_channels 4

// Se guarda en el caché ultrarrápido de la GPU
__constant__ int d_k[3][3] = {
    {-1, -1, -1},
    {-1, 8, -1},
    {-1, -1, -1}};

// Funcion que se ejecuta en el device y procesa un pixel de la imagen
__device__ void process_pixel(unsigned char *src_image, unsigned char *dst_image, int width, int height, int x, int y, int blur_step)
{
    if (x == 0 || y == 0 || x >= width - 1 || y >= height - 1)
        return;

    int r = 0;
    int g = 0;
    int b = 0;

    for (int i = -1; i <= 1; i++)
    {
        for (int j = -1; j <= +1; j++)
        {
            int index = ((y + j) * width + (i + x)) * image_channels;

            auto celda = d_k[i + 1][j + 1];

            r = r + celda * src_image[index];
            g = g + celda * src_image[index + 1];
            b = b + celda * src_image[index + 2];
        }
    }

    int index = (y * width + x) * image_channels;

    dst_image[index] = min(255, abs(r));
    dst_image[index + 1] = min(255, abs(g));
    dst_image[index + 2] = min(255, abs(b));
    dst_image[index + 3] = 255;
}

// Funcion de kernel que se ejecuta en el device
__global__ void kernel_blur_image(unsigned char *src_image, unsigned char *dst_image, int width, int height, int blur_step)
{

    int index = blockDim.x * blockIdx.x + threadIdx.x;

    //
    if (index >= width * height)
        return;

    int pix_y = index / width;
    int pix_x = index % width;

    process_pixel(src_image, dst_image, width, height, pix_x, pix_y, blur_step);
}

// Funcion de kernel que se ejecuta en el host y lanza el kernel en el device
extern "C" void kernel_blur(unsigned char *src_image, unsigned char *dst_image, int width, int height, int blur_step)
{
    int thr_per_blk = 1024;
    int blk_in_grid = ceil(float(width * height) / thr_per_blk);

    kernel_blur_image<<<blk_in_grid, thr_per_blk>>>(src_image, dst_image, width, height, blur_step);
}