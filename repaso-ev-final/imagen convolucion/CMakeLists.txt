cmake_minimum_required(VERSION 3.16)

project(RepasoExamen)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS Network Graphics Window Audio System CONFIG REQUIRED)

# Configuracion CUDA
enable_language(CXX CUDA)
set(CMAKE_CUDA_ARCHITECTURES "61")

include_directories(
        ${MPI_INCLUDE_DIR}
)

add_executable(RepasoExamen 
        main.cpp
        kernel.cu
)

target_link_libraries(RepasoExamen PRIVATE 
        fmt::fmt
        SFML::Graphics
)

target_include_directories(RepasoExamen PRIVATE 
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET RepasoExamen POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:RepasoExamen>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()
