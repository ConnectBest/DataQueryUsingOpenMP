#include "phase3/soa_loader.hpp"

#include "common/convert.hpp"
#include "common/csv.hpp"

#include <cmath>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

static inline double kNaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

static inline int64_t i64_or_missing(const std::optional<int64_t>& v) {
    return v ? *v : -1;
}

static inline int32_t i32_or_missing(const std::optional<int32_t>& v) {
    return v ? *v : -1;
}

static inline uint16_t u16_or_missing(const std::optional<int32_t>& v) {
    return v ? static_cast<uint16_t>(*v) : 0;
}

static inline double f64_or_missing(const std::optional<double>& v) {
    return v ? *v : kNaN();
}

static inline uint8_t flag_or_missing(const std::optional<bool>& v) {
    if (!v) return 0;
    return *v ? 2 : 1;
}

static void append_trip_row_soa(TripTableSoA& t, const std::vector<std::string>& f) {
    if (f.size() != 24) {
        throw std::runtime_error(
            "Unexpected column count. Expected 24, got: " + std::to_string(f.size())
        );
    }

    t.hvfhs_license_num.push_back(f[0]);
    t.dispatching_base_num.push_back(f[1]);
    t.originating_base_num.push_back(f[2]);

    t.request_ts.push_back(i64_or_missing(to_epoch_seconds(f[3])));
    t.on_scene_ts.push_back(i64_or_missing(to_epoch_seconds(f[4])));
    t.pickup_ts.push_back(i64_or_missing(to_epoch_seconds(f[5])));
    t.dropoff_ts.push_back(i64_or_missing(to_epoch_seconds(f[6])));

    t.pu_location_id.push_back(u16_or_missing(to_i32(f[7])));
    t.do_location_id.push_back(u16_or_missing(to_i32(f[8])));

    t.trip_miles.push_back(f64_or_missing(to_f64(f[9])));
    t.trip_time.push_back(i32_or_missing(to_i32(f[10])));

    t.base_passenger_fare.push_back(f64_or_missing(to_f64(f[11])));
    t.tolls.push_back(f64_or_missing(to_f64(f[12])));
    t.bcf.push_back(f64_or_missing(to_f64(f[13])));
    t.sales_tax.push_back(f64_or_missing(to_f64(f[14])));
    t.congestion_surcharge.push_back(f64_or_missing(to_f64(f[15])));
    t.airport_fee.push_back(f64_or_missing(to_f64(f[16])));
    t.tips.push_back(f64_or_missing(to_f64(f[17])));
    t.driver_pay.push_back(f64_or_missing(to_f64(f[18])));

    t.shared_request_flag.push_back(flag_or_missing(to_flag(f[19])));
    t.shared_match_flag.push_back(flag_or_missing(to_flag(f[20])));
    t.access_a_ride_flag.push_back(flag_or_missing(to_flag(f[21])));
    t.wav_request_flag.push_back(flag_or_missing(to_flag(f[22])));
    t.wav_match_flag.push_back(flag_or_missing(to_flag(f[23])));
}

TripTableSoA load_trip_csv_soa(const std::string& path, bool has_header) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Failed to open: " + path);

    TripTableSoA t{};
    t.reserve(1'000'000);

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

        append_trip_row_soa(t, fields);
    }

    return t;
}
