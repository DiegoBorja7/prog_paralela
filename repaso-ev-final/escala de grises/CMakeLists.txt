cmake_minimum_required(VERSION 3.16)
project(RepasoExamen LANGUAGES C CXX CUDA)

find_package(fmt CONFIG REQUIRED)
find_package(OpenMP COMPONENTS CXX REQUIRED)

# Configuracion MPI (Intel oneAPI)
set(MPI_ROOT_DIR "C:/Program Files (x86)/Intel/oneAPI/mpi/latest")
set(MPI_INCLUDE_DIR "${MPI_ROOT_DIR}/include")
set(MPI_LIB_DIR "${MPI_ROOT_DIR}/lib")

# Configuracion CUDA
set(CMAKE_CUDA_ARCHITECTURES "61")

include_directories(
        ${MPI_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 1. Main and CUDA executable (OpenMP + CUDA)
add_executable(RepasoExamen 
        main.cpp
        kernel.cu
)
target_link_libraries(RepasoExamen PRIVATE 
        fmt::fmt
        OpenMP::OpenMP_CXX
)
target_include_directories(RepasoExamen PRIVATE 
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

# 2. MPI executable
add_executable(RepasoExamen_MPI
        main_mpi.cpp
)
target_link_libraries(RepasoExamen_MPI PRIVATE 
        fmt::fmt
        "${MPI_LIB_DIR}/impi.lib"
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET RepasoExamen POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        "${MINGW_BIN_DIR}/libgomp-1.dll"
                        $<TARGET_FILE_DIR:RepasoExamen>
                COMMENT "Copying MinGW runtime DLLs for RepasoExamen"
        )
        add_custom_command(TARGET RepasoExamen_MPI POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:RepasoExamen_MPI>
                COMMENT "Copying MinGW runtime DLLs for RepasoExamen_MPI"
        )
endif()