#pragma once

#include "phase3/soa_types.hpp"
#include <string>

TripTableSoA load_trip_csv_soa(const std::string& path, bool has_header);
