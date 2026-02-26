cmake_minimum_required(VERSION 3.20)
project(tlc LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tlc
        src/main.cpp
)

# Basic performance-friendly flags for Release
if (NOT MSVC)
    target_compile_options(tlc PRIVATE -Wall -Wextra -Wpedantic)
endif()