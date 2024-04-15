cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(grav LANGUAGES CXX)

add_definitions(-g)

find_package(quill REQUIRED)
find_package(SFML REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(EnTT REQUIRED)

add_executable(grav src/grav.cpp src/input_handling.cpp)

target_compile_features(grav PUBLIC cxx_std_20)
target_include_directories(
  grav PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(grav PRIVATE quill::quill sfml-graphics EnTT::EnTT
                                   Eigen3::Eigen)
