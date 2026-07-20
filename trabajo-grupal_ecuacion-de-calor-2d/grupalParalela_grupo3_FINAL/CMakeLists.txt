cmake_minimum_required(VERSION 3.16)

project(calor-2d)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -mavx2 -fopenmp")
# set(CMAKE_C_FLAGS) Flags para c

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)
find_package(Freetype REQUIRED)


set(MPI_ROOT_DIR "C:/Program Files (x86)/Intel/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPI_ROOT_DIR}/include")
set(MPI_LIB "${MPI_ROOT_DIR}/lib")

include_directories(
    ${MPI_INCLUDE}
)


add_executable(calor-2d 
main_calor.cpp
calor_serial.cpp
calor_simd.cpp
calor_openmp.cpp
palette.cpp
)

target_link_libraries(calor-2d PRIVATE 
fmt::fmt
SFML::System
SFML::Graphics
SFML::Window
)

add_executable(calor-mpi
main_calor_mpi.cpp
calor_mpi.cpp
calor_serial.cpp
palette.cpp
draw_text.cpp
)

target_link_libraries(calor-mpi PRIVATE 
fmt::fmt
SFML::System
SFML::Graphics
SFML::Window
Freetype::Freetype
${MPI_LIB}/impi.lib
)
if(MINGW)
    get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
    add_custom_command(TARGET calor-2d POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_BIN_DIR}/libstdc++-6.dll"
            "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
            "${MINGW_BIN_DIR}/libwinpthread-1.dll"
            $<TARGET_FILE_DIR:calor-2d>
    )
    add_custom_command(TARGET calor-mpi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${MINGW_BIN_DIR}/libstdc++-6.dll"
            "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
            "${MINGW_BIN_DIR}/libwinpthread-1.dll"
            $<TARGET_FILE_DIR:calor-mpi>
    )
endif()