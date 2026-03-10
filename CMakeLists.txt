cmake_minimum_required(VERSION 3.20)
project(cmpe275_mini1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

add_library(common
        src/common/args.cpp
        src/common/csv.cpp
        src/common/convert.cpp
)

target_compile_options(common PRIVATE -Wall -Wextra -Wpedantic)

add_library(phase1_aos
        src/phase1/aos_loader.cpp
        src/phase1/aos_queries_serial.cpp
)
target_link_libraries(phase1_aos PRIVATE common)

add_executable(tlc_phase1
        src/phase1/main_phase1.cpp
)
target_link_libraries(tlc_phase1 PRIVATE common phase1_aos)

find_package(OpenMP REQUIRED)

add_library(phase2_aos_openmp
        src/phase2/aos_queries_openmp.cpp
)
target_link_libraries(phase2_aos_openmp PRIVATE common phase1_aos OpenMP::OpenMP_CXX)

add_executable(tlc_phase2
        src/phase2/main_phase2.cpp
)
target_link_libraries(tlc_phase2 PRIVATE phase2_aos_openmp OpenMP::OpenMP_CXX)

add_library(phase3_soa
        src/phase3/soa_loader.cpp
        src/phase3/soa_queries_openmp.cpp
)
target_link_libraries(phase3_soa PRIVATE common OpenMP::OpenMP_CXX)

add_executable(tlc_phase3
        src/phase3/main_phase3.cpp
)
target_link_libraries(tlc_phase3 PRIVATE common phase3_soa OpenMP::OpenMP_CXX)