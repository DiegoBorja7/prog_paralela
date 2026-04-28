cmake_minimum_required(VERSION 3.16)

project(Ejemplo02)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(fmt CONFIG REQUIRED)
find_package(SFML COMPONENTS System Graphics Window CONFIG REQUIRED)

add_executable(ejemplo02 main.cpp)

target_link_libraries(ejemplo02 PRIVATE 
  fmt::fmt 
  SFML::System 
  SFML::Graphics 
  SFML::Window)

add_custom_command(TARGET ejemplo02 POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/textura.jpg"
          "$<TARGET_FILE_DIR:ejemplo02>/textura.jpg"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "${CMAKE_SOURCE_DIR}/arial.ttf"
          "$<TARGET_FILE_DIR:ejemplo02>/arial.ttf"
  COMMENT "Copying runtime assets to output folder")
