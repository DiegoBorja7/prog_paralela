cmake_minimum_required(VERSION 3.16)

project(Fractaldll)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")

find_package(fmt CONFIG REQUIRED)

add_library(fractal-dll SHARED
        fractal_simd.cpp
        palette.cpp)

target_link_libraries(fractal-dll PRIVATE 
  fmt::fmt 
)

