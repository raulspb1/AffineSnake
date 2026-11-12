cmake_minimum_required(VERSION 3.14)
project(AffineSnake LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(affine_snake src/main.cpp src/snake.h src/snake.cpp)

target_include_directories(affine_snake PRIVATE src)
target_link_libraries(affine_snake PRIVATE Qt5::Widgets)
