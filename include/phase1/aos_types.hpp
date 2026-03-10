#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct TripRecord {
    std::string hvfhs_license_num;
    std::string dispatching_base_num;
    std::string originating_base_num;

    std::optional<int64_t> request_ts;
    std::optional<int64_t> on_scene_ts;
    std::optional<int64_t> pickup_ts;
    std::optional<int64_t> dropoff_ts;

    std::optional<int32_t> pu_location_id;
    std::optional<int32_t> do_location_id;

    std::optional<double> trip_miles;
    std::optional<int32_t> trip_time;

    std::optional<double> base_passenger_fare;
    std::optional<double> tolls;
    std::optional<double> bcf;
    std::optional<double> sales_tax;
    std::optional<double> congestion_surcharge;
    std::optional<double> airport_fee;
    std::optional<double> tips;
    std::optional<double> driver_pay;

    std::optional<bool> shared_request_flag;
    std::optional<bool> shared_match_flag;
    std::optional<bool> access_a_ride_flag;
    std::optional<bool> wav_request_flag;
    std::optional<bool> wav_match_flag;
};

struct TripTableAoS {
    std::vector<TripRecord> rows;
};
