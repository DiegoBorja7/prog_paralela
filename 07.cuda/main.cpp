#include <iostream>
#include <vector>
#include <fmt/core.h>

#include <cuda_runtime.h>

const size_t VECTOR_SIZE = 1024 * 1024;

extern void sumaVectores(float *a, float *b, float *c, int n);

// Macro for error checking
#define CHECK_CUDA_ERROR(call) { \
    cudaError_t err = call; \
    if (err != cudaSuccess) { \
        fmt::print(stderr, "CUDA error in {}:{}: {}\n", __FILE__, __LINE__, cudaGetErrorString(err)); \
        return -1; \
    } \
}

int main()
{
    int deviceCount = 0;
    cudaError_t err = cudaGetDeviceCount(&deviceCount);
    if (err != cudaSuccess || deviceCount == 0) {
        fmt::print(stderr, "No CUDA devices found or CUDA is not supported. Error: {} - {}\n", (int)err, cudaGetErrorString(err));
        return -1;
    }

    int deviceID = 0;
    CHECK_CUDA_ERROR(cudaSetDevice(deviceID));

    cudaDeviceProp deviceProp;
    CHECK_CUDA_ERROR(cudaGetDeviceProperties(&deviceProp, deviceID));

    fmt::print("Nombre del dispositivo CUDA: {}\n", deviceProp.name);
    fmt::print("Capability de cómputo: {}.{}\n", deviceProp.major, deviceProp.minor);
    fmt::print("Memoria Global: {} bytes | En GigaBytes: {} GB\n", deviceProp.totalGlobalMem, deviceProp.totalGlobalMem / (1024 * 1024 * 1024));
    fmt::print("Memoria Compartida Por Bloque: {} bytes | En Kilobytes: {} KB\n", deviceProp.sharedMemPerBlock, deviceProp.sharedMemPerBlock / 1024);
    fmt::print("Registros Por Bloque: {} | En Kilobytes: {} KB\n", deviceProp.regsPerBlock, deviceProp.regsPerBlock * sizeof(float) / 1024);
    fmt::print("Tamaño del Warp: {}\n", deviceProp.warpSize);
    fmt::print("Hilos máximos Por Bloque: {}\n", deviceProp.maxThreadsPerBlock);
    fmt::print("Hilos máximos Por Multiprocesador: {}\n", deviceProp.maxThreadsPerMultiProcessor);
    fmt::print("Bloques máximos Por Grid: {}\n", deviceProp.maxBlocksPerMultiProcessor);
    fmt::print("Tamaño del bloque: {} x {} x {}\n", deviceProp.maxGridSize[0], deviceProp.maxGridSize[1], deviceProp.maxGridSize[2]);

    //--inicializar host
    fmt::print("\n");
    fmt::print("Inicializando vectores...\n");
    float* h_A = new float[VECTOR_SIZE];
    float* h_B = new float[VECTOR_SIZE];
    float* h_C = new float[VECTOR_SIZE];

    for(size_t i=0; i<VECTOR_SIZE; i++){
        h_A[i] = 1.0f;
        h_B[i] = 2.0f;
        h_C[i] = 0.0f;
    }
    //--inicializar device
    float* d_A;
    float* d_B;
    float* d_C;

    size_t size_in_bytes = VECTOR_SIZE * sizeof(float);

    cudaMalloc((void**)&d_A, size_in_bytes);
    cudaMalloc((void**)&d_B, size_in_bytes);
    cudaMalloc((void**)&d_C, size_in_bytes);

    //--copiar del host al device
    cudaMemcpy(d_A, h_A, size_in_bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_B, h_B, size_in_bytes, cudaMemcpyHostToDevice);

    //-invocacion del kernel
    sumaVectores(d_A, d_B, d_C, VECTOR_SIZE);

    //--copiar del device al host
    cudaMemcpy(h_C, d_C, size_in_bytes, cudaMemcpyDeviceToHost);

    //--imprimir resultados
    fmt::print("Primeros 10 elementos del vector resultante C:\n");
    for(size_t i=0; i<10; i++){
        fmt::println("C[{}] = {}", i, h_C[i]);
    }

    //--liberar memoria
    cudaFree(d_A);
    cudaFree(d_B);
    cudaFree(d_C);

    delete[] h_A;
    delete[] h_B;
    delete[] h_C;
    return 0;
}