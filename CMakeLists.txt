# cmake_minimum_required(VERSION 3.13)

# project(actuator_lib)

# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 17)

# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# FetchContent_MakeAvailable(googletest)

# add_executable(actuator_lib
# test/test_main.cpp
# actuator_lib.cpp
# )
# configure_file(test/main.uf2 main.uf2 COPYONLY)

# # Recursively add all files in the "serial" directory
# file(GLOB_RECURSE SERIAL_SOURCES "serial/*.cc")
# target_sources(actuator_lib PRIVATE ${SERIAL_SOURCES})
# include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# target_link_libraries(actuator_lib
#     pthread
#     GTest::gtest_main
# )

# include(GoogleTest)


cmake_minimum_required(VERSION 3.13)

project(actuator_lib)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Find the required Qt5 modules
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Test)

# Automatically run moc on QObject classes
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# FetchContent for GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

# Add the executable target
add_executable(actuator_lib
        test/test_main.cpp
        actuator_lib.cpp
        test/GetGPIOTest.h
)

# Recursively add all files in the "serial" directory
file(GLOB_RECURSE SERIAL_SOURCES "serial/*.cc")
target_sources(actuator_lib PRIVATE ${SERIAL_SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Link the Qt5 and GoogleTest libraries
target_link_libraries(actuator_lib
        Qt5::Core
        Qt5::Widgets
        Qt5::Test
        pthread
        GTest::gtest_main
)

# Ensure MOC includes the generated moc files
set_property(TARGET actuator_lib PROPERTY AUTOMOC ON)

# Configure the test files
configure_file(test/main.uf2 main.uf2 COPYONLY)
