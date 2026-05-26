cmake_minimum_required(VERSION 3.16)

project(Trabajo02)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)


add_executable(trabajo02 
        main.cpp
        grayscale.cpp
        image_io.cpp
)

target_link_libraries(trabajo02 PRIVATE 
  fmt::fmt
  SFML::System
  SFML::Graphics
  SFML::Window
)

target_compile_options(trabajo02 PRIVATE -mavx)
target_include_directories(trabajo02 PRIVATE ${CMAKE_SOURCE_DIR}/third_party)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET trabajo02 POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:trabajo02>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()
