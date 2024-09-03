cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(usb_actuator_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Enable exception handling
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

pico_sdk_init()

# add all .cpp and .h files in the AccelStepper library to the project
add_library(AccelStepper INTERFACE)
target_include_directories(AccelStepper INTERFACE AccelStepper)
target_sources(AccelStepper INTERFACE AccelStepper/AccelStepper.cpp)

add_executable(main
  main.cpp
)
target_link_libraries(main 
  pico_stdlib 
  pico_multicore
  hardware_adc
  hardware_uart
  hardware_timer
  AccelStepper
)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 1)

pico_add_extra_outputs(main)