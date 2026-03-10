#pragma once

#include "phase3/soa_benchmark_types.hpp"
#include <string>

TripTableSoABenchmark load_trip_csv_soa_benchmark(const std::string& path, bool has_header);
