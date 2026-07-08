#include <iostream>
#include <vector>
#include <fmt/core.h>
#include <SFML/Graphics.hpp>
#include <complex>

#include <cuda_runtime.h>

#define WIDTH 1600
#define HEIGHT 900

#ifdef _WIN32
#include <windows.h>
#endif

double x_min = -1.5;
double x_max = 1.5;
double y_min = -1.0;
double y_max = 1.0;

uint32_t *pixel_buffer = nullptr;

#define CHECK(expr) {                               \
        auto internal_error = (expr);               \
        if (internal_error!=cudaSuccess) {          \
            fmt::println("{}: {} in {} at line {}", (int )internal_error, cudaGetErrorString(internal_error), __FILE__, __LINE__);    \
            exit(EXIT_FAILURE);                     \
        }                                           \
    }

int main()
{

    fmt::print("hola rey xD\n");

    return 0;
}