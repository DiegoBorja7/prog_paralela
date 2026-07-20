cmake_minimum_required(VERSION 3.18)

# Definimos exactamente tu arquitectura para evitar advertencias de CMake
set(CMAKE_CUDA_ARCHITECTURES "61")

project(FractalCuda)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)
# Configuracion CUDA
enable_language(CXX CUDA)

include_directories(
        ${MPI_INCLUDE_DIR}
        ${SFML_INCLUDE_DIRS}
)

add_executable(FractalCuda 
        main.cpp
        kernel.cu
        palette.cpp
)

target_link_libraries(FractalCuda PRIVATE 
        fmt::fmt
        SFML::System 
        SFML::Graphics 
        SFML::Window
)

target_include_directories(FractalCuda PRIVATE 
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET FractalCuda POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:FractalCuda>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()
