#ifndef FRACTAL_NEWTON_MPI_H
#define FRACTAL_NEWTON_MPI_H

#include <stdint.h>

struct ComputeResult {
    uint32_t total_iterations;
};

ComputeResult newton_mpi(double x_min, double y_min, double x_max, double y_max, 
                   uint32_t width, uint32_t height, 
                   uint32_t row_start, uint32_t row_end, 
                   uint32_t* pixel_buffer,
                   int max_iteraciones);

#endif // FRACTAL_NEWTON_MPI_H