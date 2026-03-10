#include "phase1/aos_loader.hpp"

#include "common/convert.hpp"
#include "common/csv.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

TripRecord parse_trip_row_aos(const std::vector<std::string>& f) {
    if (f.size() != 24) {
        throw std::runtime_error(
            "Unexpected column count. Expected 24, got: " + std::to_string(f.size())
        );
    }

    TripRecord r{};

    r.hvfhs_license_num = f[0];
    r.dispatching_base_num = f[1];
    r.originating_base_num = f[2];

    r.request_ts = to_epoch_seconds(f[3]);
    r.on_scene_ts = to_epoch_seconds(f[4]);
    r.pickup_ts = to_epoch_seconds(f[5]);
    r.dropoff_ts = to_epoch_seconds(f[6]);

    r.pu_location_id = to_i32(f[7]);
    r.do_location_id = to_i32(f[8]);

    r.trip_miles = to_f64(f[9]);
    r.trip_time = to_i32(f[10]);

    r.base_passenger_fare = to_f64(f[11]);
    r.tolls = to_f64(f[12]);
    r.bcf = to_f64(f[13]);
    r.sales_tax = to_f64(f[14]);
    r.congestion_surcharge = to_f64(f[15]);
    r.airport_fee = to_f64(f[16]);
    r.tips = to_f64(f[17]);
    r.driver_pay = to_f64(f[18]);

    r.shared_request_flag = to_flag(f[19]);
    r.shared_match_flag = to_flag(f[20]);
    r.access_a_ride_flag = to_flag(f[21]);
    r.wav_request_flag = to_flag(f[22]);
    r.wav_match_flag = to_flag(f[23]);

    return r;
}

TripTableAoS load_trip_csv_aos(const std::string& path, bool has_header) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Failed to open: " + path);

    TripTableAoS t{};
    t.rows.reserve(1'000'000);

    std::string line;
    bool first = true;

    while (std::getline(in, line)) {
        if (first && has_header) {
            first = false;
            continue;
        }
        first = false;

        auto fields = split_csv_line(line);
        if (fields.size() == 1 && fields[0].empty()) continue;

        t.rows.push_back(parse_trip_row_aos(fields));
    }

    return t;
}
