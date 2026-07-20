cmake_minimum_required(VERSION 3.16)

project(EcuacionCalor2D)

find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenMP REQUIRED)

# Configuracion MPI
set(MPI_ROOT_DIR "C:/Program Files (x86)/Intel/oneAPI/mpi/latest")
set(MPI_INCLUDE_DIR "${MPI_ROOT_DIR}/include")
set(MPI_LIB_DIR "${MPI_ROOT_DIR}/lib")

include_directories(
        ${MPI_INCLUDE_DIR}
)

add_executable(calor2d 
        main.cpp
        calor_serial.cpp
        calor_simd.cpp
        calor_omp.cpp
        calor_mpi.cpp
        palette.cpp
)

# Activar optimizaciones SIMD en GCC (AVX2)
target_compile_options(calor2d PRIVATE -O3 -mavx2)

target_link_libraries(calor2d PRIVATE 
        fmt::fmt
        SFML::Graphics
        SFML::Window
        SFML::System
        Freetype::Freetype
        OpenMP::OpenMP_CXX
        "${MPI_LIB_DIR}/impi.lib" 
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET calor2d POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:calor2d>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()