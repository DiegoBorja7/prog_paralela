cmake_minimum_required(VERSION 3.16)

project(EjemploMPI)

find_package(fmt CONFIG REQUIRED)

# Configuracion MPI
set(MPI_ROOT_DIR "C:/Program Files (x86)/Intel/oneAPI/mpi/latest")
set(MPI_INCLUDE_DIR "${MPI_ROOT_DIR}/include")
set(MPI_LIB_DIR "${MPI_ROOT_DIR}/lib")

include_directories(
        ${MPI_INCLUDE_DIR}
)

add_executable(ejemplo-mpi 
        main.cpp
)

target_link_libraries(ejemplo-mpi PRIVATE 
        fmt::fmt
        "${MPI_LIB_DIR}/impi.lib" 
)

add_executable(matrices-multiplicacion 
        matrices-multiplicacion.cpp
)

target_link_libraries(matrices-multiplicacion PRIVATE 
        fmt::fmt
        "${MPI_LIB_DIR}/impi.lib" 
)

add_executable(multplicacion-comunicacion-colectiva 
        multplicacion-comunicacion-colectiva.cpp
)

target_link_libraries(multplicacion-comunicacion-colectiva PRIVATE 
        fmt::fmt
        "${MPI_LIB_DIR}/impi.lib" 
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET matrices-multiplicacion POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:matrices-multiplicacion>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()
