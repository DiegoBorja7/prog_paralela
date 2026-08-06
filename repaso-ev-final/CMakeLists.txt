cmake_minimum_required(VERSION 3.16)
project(ImageBlur LANGUAGES C CXX CUDA)

# SFML, fmt, OpenMP
find_package(SFML COMPONENTS Graphics Window System REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(OpenMP COMPONENTS CXX REQUIRED)

set(CMAKE_CUDA_ARCHITECTURES "61")

# Ejecutable
add_executable(ImageBlur 
        main.cpp
        kernel.cu
)

# Librerias
target_link_libraries(ImageBlur PRIVATE 
        fmt::fmt
        SFML::Graphics
        SFML::Window
        SFML::System
        OpenMP::OpenMP_CXX
)

target_include_directories(ImageBlur PRIVATE 
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET ImageBlur POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        "${MINGW_BIN_DIR}/libgomp-1.dll"
                        $<TARGET_FILE_DIR:ImageBlur>
                COMMENT "Copiando DLLs necesarias para MinGW y OpenMP"
        )
endif()