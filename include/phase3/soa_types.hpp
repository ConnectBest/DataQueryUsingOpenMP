#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct TripTableSoA {
    std::vector<std::string> hvfhs_license_num;
    std::vector<std::string> dispatching_base_num;
    std::vector<std::string> originating_base_num;

    std::vector<int64_t> request_ts;
    std::vector<int64_t> on_scene_ts;
    std::vector<int64_t> pickup_ts;
    std::vector<int64_t> dropoff_ts;

    std::vector<uint16_t> pu_location_id;
    std::vector<uint16_t> do_location_id;

    std::vector<double> trip_miles;
    std::vector<int32_t> trip_time;

    std::vector<double> base_passenger_fare;
    std::vector<double> tolls;
    std::vector<double> bcf;
    std::vector<double> sales_tax;
    std::vector<double> congestion_surcharge;
    std::vector<double> airport_fee;
    std::vector<double> tips;
    std::vector<double> driver_pay;

    std::vector<uint8_t> shared_request_flag;
    std::vector<uint8_t> shared_match_flag;
    std::vector<uint8_t> access_a_ride_flag;
    std::vector<uint8_t> wav_request_flag;
    std::vector<uint8_t> wav_match_flag;

    std::size_t size() const { return tips.size(); }

    void reserve(std::size_t n) {
        hvfhs_license_num.reserve(n);
        dispatching_base_num.reserve(n);
        originating_base_num.reserve(n);

        request_ts.reserve(n);
        on_scene_ts.reserve(n);
        pickup_ts.reserve(n);
        dropoff_ts.reserve(n);

        pu_location_id.reserve(n);
        do_location_id.reserve(n);

        trip_miles.reserve(n);
        trip_time.reserve(n);

        base_passenger_fare.reserve(n);
        tolls.reserve(n);
        bcf.reserve(n);
        sales_tax.reserve(n);
        congestion_surcharge.reserve(n);
        airport_fee.reserve(n);
        tips.reserve(n);
        driver_pay.reserve(n);

        shared_request_flag.reserve(n);
        shared_match_flag.reserve(n);
        access_a_ride_flag.reserve(n);
        wav_request_flag.reserve(n);
        wav_match_flag.reserve(n);
    }
};
