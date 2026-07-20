cmake_minimum_required(VERSION 3.16)

project(CorreccionPruebaMPI)

find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(Freetype REQUIRED)

# Configuracion MPI
set(MPI_ROOT_DIR "C:/Program Files (x86)/Intel/oneAPI/mpi/latest")
set(MPI_INCLUDE_DIR "${MPI_ROOT_DIR}/include")
set(MPI_LIB_DIR "${MPI_ROOT_DIR}/lib")

include_directories(
        ${MPI_INCLUDE_DIR}
)

add_executable(prueba1-mpi 
        prueba1.cpp
        fractal_newton_mpi.cpp
        draw_text.cpp
        palette.cpp
)

target_link_libraries(prueba1-mpi PRIVATE 
        fmt::fmt
        SFML::Graphics
        SFML::Window
        SFML::System
        "${MPI_LIB_DIR}/impi.lib" 
        Freetype::Freetype
)

add_executable(prueba2-mpi 
        prueba2.cpp
        fractal_burning_ship_mpi.cpp
        draw_text.cpp
        palette.cpp
)

target_link_libraries(prueba2-mpi PRIVATE 
        fmt::fmt
        SFML::Graphics
        SFML::Window
        SFML::System
        "${MPI_LIB_DIR}/impi.lib" 
        Freetype::Freetype
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET prueba1-mpi POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:prueba1-mpi>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()