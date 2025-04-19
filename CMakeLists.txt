cmake_minimum_required(VERSION 3.22 FATAL_ERROR)
project(Solution)

# Definitions
set(BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(BIN_INT_DIR ${CMAKE_SOURCE_DIR}/bin-int)

# Parameters
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the main project/executable
add_subdirectory(vendor/ConsoleLib)
add_subdirectory(LinguaPie)
