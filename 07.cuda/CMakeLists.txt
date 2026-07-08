cmake_minimum_required(VERSION 3.16)

project(Cuda01)

find_package(fmt CONFIG REQUIRED)
# Configuracion CUDA
enable_language(CXX CUDA)

include_directories(
        ${MPI_INCLUDE_DIR}
)

add_executable(cuda01 
        main.cpp
        kernel.cu
)

target_link_libraries(cuda01 PRIVATE 
        fmt::fmt
)

target_include_directories(cuda01 PRIVATE 
        ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES}
)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET cuda01 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:cuda01>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()
