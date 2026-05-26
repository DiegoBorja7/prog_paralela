cmake_minimum_required(VERSION 3.16)

project(TrabajoEnClase)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

find_package(fmt CONFIG REQUIRED)


add_executable(trabajo-en-clase 
        main.cpp
)

target_link_libraries(trabajo-en-clase PRIVATE 
  fmt::fmt 
)

target_compile_options(trabajo-en-clase PRIVATE -mavx)

if(MINGW)
        get_filename_component(MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET trabajo-en-clase POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${MINGW_BIN_DIR}/libstdc++-6.dll"
                        "${MINGW_BIN_DIR}/libgcc_s_seh-1.dll"
                        "${MINGW_BIN_DIR}/libwinpthread-1.dll"
                        $<TARGET_FILE_DIR:trabajo-en-clase>
                COMMENT "Copying MinGW runtime DLLs"
        )
endif()

