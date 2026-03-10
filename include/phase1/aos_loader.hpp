#pragma once

#include "phase1/aos_types.hpp"

#include <string>
#include <vector>

TripRecord parse_trip_row_aos(const std::vector<std::string>& f);
TripTableAoS load_trip_csv_aos(const std::string& path, bool has_header);
