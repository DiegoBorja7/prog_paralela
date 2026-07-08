#include <cmath>

__global__
void sumaKernel(float *a, float *b, float *c, int n) {
    int index = blockIdx.x * blockDim.x + threadIdx.x;
    if (index < n) {
        c[index] = a[index] + b[index];
    }
}

void sumaVectores(float *a, float *b, float *c, int n) {
    int threadsPerBlock = 1024;
    int num_blocks = std::ceil(n * 1.0 / threadsPerBlock);

    sumaKernel<<<num_blocks, threadsPerBlock>>>(a, b, c, n);
}