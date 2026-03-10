#include "phase1/aos_queries_serial.hpp"

QueryResult query_tips_gt_aos_serial(const TripTableAoS& t, double threshold) {
    QueryResult r{};
    for (const auto& row : t.rows) {
        if (row.tips && *row.tips > threshold) {
            r.count++;
            r.sum += *row.tips;
        }
    }
    return r;
}

QueryResult query_fare_gt_aos_serial(const TripTableAoS& t, double threshold) {
    QueryResult r{};
    for (const auto& row : t.rows) {
        if (row.base_passenger_fare && *row.base_passenger_fare > threshold) {
            r.count++;
            r.sum += *row.base_passenger_fare;
        }
    }
    return r;
}

QueryResult query_miles_gt_aos_serial(const TripTableAoS& t, double threshold) {
    QueryResult r{};
    for (const auto& row : t.rows) {
        if (row.trip_miles && *row.trip_miles > threshold) {
            r.count++;
            r.sum += *row.trip_miles;
        }
    }
    return r;
}
