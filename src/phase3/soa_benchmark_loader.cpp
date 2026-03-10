#include "phase3/soa_benchmark_loader.hpp"

#include "common/csv.hpp"
#include "common/convert.hpp"

#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>

static inline double kNaN() {
    return std::numeric_limits<double>::quiet_NaN();
}

static inline double f64_or_missing(const std::optional<double>& v) {
    return v ? *v : kNaN();
}

static void append_trip_row_soa_benchmark(TripTableSoABenchmark& t, const std::vector<std::string>& f) {
    if (f.size() != 24) {
        throw std::runtime_error(
            "Unexpected column count. Expected 24, got: " + std::to_string(f.size())
        );
    }

    // Only materialize the 3 benchmarked columns
    t.trip_miles.push_back(f64_or_missing(to_f64(f[9])));
    t.base_passenger_fare.push_back(f64_or_missing(to_f64(f[11])));
    t.tips.push_back(f64_or_missing(to_f64(f[17])));
}

TripTableSoABenchmark load_trip_csv_soa_benchmark(const std::string& path, bool has_header) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Failed to open: " + path);

    TripTableSoABenchmark t{};
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

        append_trip_row_soa_benchmark(t, fields);
    }

    return t;
}
