#include "phase3/soa_queries_openmp.hpp"

#include <cmath>

QueryResult query_tips_gt_soa_openmp(const TripTableSoA& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(t.tips.size()); i++) {
        double v = t.tips[i];
        if (!std::isnan(v) && v > threshold) {
            count++;
            sum += v;
        }
    }

    return {count, sum};
}

QueryResult query_fare_gt_soa_openmp(const TripTableSoA& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(t.base_passenger_fare.size()); i++) {
        double v = t.base_passenger_fare[i];
        if (!std::isnan(v) && v > threshold) {
            count++;
            sum += v;
        }
    }

    return {count, sum};
}

QueryResult query_miles_gt_soa_openmp(const TripTableSoA& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(t.trip_miles.size()); i++) {
        double v = t.trip_miles[i];
        if (!std::isnan(v) && v > threshold) {
            count++;
            sum += v;
        }
    }

    return {count, sum};
}
