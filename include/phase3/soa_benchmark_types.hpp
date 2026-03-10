#pragma once

#include <cstddef>
#include <vector>

struct TripTableSoABenchmark {
    std::vector<double> trip_miles;
    std::vector<double> base_passenger_fare;
    std::vector<double> tips;

    std::size_t size() const { return trip_miles.size(); }

    void reserve(std::size_t n) {
        trip_miles.reserve(n);
        base_passenger_fare.reserve(n);
        tips.reserve(n);
    }
};
