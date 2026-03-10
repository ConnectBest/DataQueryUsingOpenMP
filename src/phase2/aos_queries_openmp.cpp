#include "phase2/aos_queries_openmp.hpp"

QueryResult query_tips_gt_aos_openmp(const TripTableAoS& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(t.rows.size()); i++) {
        const auto& row = t.rows[i];
        if (row.tips && *row.tips > threshold) {
            count++;
            sum += *row.tips;
        }
    }

    return {count, sum};
}

QueryResult query_fare_gt_aos_openmp(const TripTableAoS& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(t.rows.size()); i++) {
        const auto& row = t.rows[i];
        if (row.base_passenger_fare && *row.base_passenger_fare > threshold) {
            count++;
            sum += *row.base_passenger_fare;
        }
    }

    return {count, sum};
}

QueryResult query_miles_gt_aos_openmp(const TripTableAoS& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(t.rows.size()); i++) {
        const auto& row = t.rows[i];
        if (row.trip_miles && *row.trip_miles > threshold) {
            count++;
            sum += *row.trip_miles;
        }
    }

    return {count, sum};
}
