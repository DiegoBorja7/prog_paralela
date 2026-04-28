cmake_minimum_required(VERSION 3.16)

project(FractalJulia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)

add_executable(fractal-julia 
        main.cpp
        fractal_serial.cpp
        fractal_simd.cpp
        palette.cpp)

target_link_libraries(fractal-julia PRIVATE 
  fmt::fmt 
  SFML::System 
  SFML::Graphics 
  SFML::Window)

if(EXISTS "${CMAKE_SOURCE_DIR}/arial.ttf")
        add_custom_command(TARGET fractal-julia POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${CMAKE_SOURCE_DIR}/arial.ttf"
                        $<TARGET_FILE_DIR:fractal-julia>/arial.ttf
                COMMENT "Copying arial.ttf"
        )
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/textura.jpg")
        add_custom_command(TARGET fractal-julia POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${CMAKE_SOURCE_DIR}/textura.jpg"
                        $<TARGET_FILE_DIR:fractal-julia>/textura.jpg
                COMMENT "Copying textura.jpg"
        )
endif()

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET fractal-julia POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:fractal-julia>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()

