# This file is now not needed because compilation is done via Qt.
# It is left here for refrence.

cmake_minimum_required(VERSION 3.13)

project(actuator_lib_test)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

add_library(actuator_lib INTERFACE)
target_include_directories(actuator_lib INTERFACE actuator_lib)
target_sources(actuator_lib INTERFACE actuator_lib/actuator_lib.cpp actuator_lib/serialib.cpp)

add_executable(actuator_lib_test main.cpp actuator_lib/actuator_lib.cpp actuator_lib/serialib.cpp)
target_link_libraries(actuator_lib_test actuator_lib)