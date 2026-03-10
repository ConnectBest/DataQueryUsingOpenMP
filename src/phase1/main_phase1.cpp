#include "common/args.hpp"
#include "phase1/aos_loader.hpp"
#include "phase1/aos_queries_serial.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <type_traits>

using std::cerr;
using std::cout;

template <typename T>
static std::string opt_to_string(const std::optional<T>& v) {
    if (!v) return "";
    if constexpr (std::is_same_v<T, bool>) return *v ? "Y" : "N";
    return std::to_string(*v);
}

static void print_preview(const TripTableAoS& t, std::size_t limit) {
    auto n = std::min(limit, t.rows.size());

    cout << "Loaded rows: " << t.rows.size() << "\n";
    cout << "Preview first " << n << " rows:\n\n";

    cout << std::left
         << std::setw(8) << "PU"
         << std::setw(8) << "DO"
         << std::setw(10) << "miles"
         << std::setw(10) << "time_s"
         << std::setw(12) << "fare"
         << std::setw(12) << "tips"
         << std::setw(8) << "shared"
         << "\n";

    cout << std::string(68, '-') << "\n";

    for (std::size_t i = 0; i < n; i++) {
        const auto& r = t.rows[i];

        cout << std::left
             << std::setw(8) << opt_to_string(r.pu_location_id)
             << std::setw(8) << opt_to_string(r.do_location_id)
             << std::setw(10) << (r.trip_miles ? (std::ostringstream{} << std::fixed << std::setprecision(2) << *r.trip_miles).str() : "")
             << std::setw(10) << opt_to_string(r.trip_time)
             << std::setw(12) << (r.base_passenger_fare ? (std::ostringstream{} << std::fixed << std::setprecision(2) << *r.base_passenger_fare).str() : "")
             << std::setw(12) << (r.tips ? (std::ostringstream{} << std::fixed << std::setprecision(2) << *r.tips).str() : "")
             << std::setw(8) << opt_to_string(r.shared_request_flag)
             << "\n";
    }
}

int main(int argc, char** argv) {
    try {
        auto args = parse_args(argc, argv);

        cout << "Phase 1:\n";

        auto t0 = std::chrono::steady_clock::now();
        auto table = load_trip_csv_aos(args.path, args.header);
        auto t1 = std::chrono::steady_clock::now();

        auto load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        cout << "Load time: " << load_ms << " ms\n";
        cerr << "sizeof(TripRecord)=" << sizeof(TripRecord) << "\n";
        cerr << "rows=" << table.rows.size() << "\n";

        auto q0 = std::chrono::steady_clock::now();
        auto r1 = query_tips_gt_aos_serial(table, 5.0);
        auto q1 = std::chrono::steady_clock::now();

        auto r2 = query_fare_gt_aos_serial(table, 40.0);
        auto q2 = std::chrono::steady_clock::now();

        auto r3 = query_miles_gt_aos_serial(table, 10.0);
        auto q3 = std::chrono::steady_clock::now();

        auto tips_ms = std::chrono::duration_cast<std::chrono::milliseconds>(q1 - q0).count();
        auto fare_ms = std::chrono::duration_cast<std::chrono::milliseconds>(q2 - q1).count();
        auto miles_ms = std::chrono::duration_cast<std::chrono::milliseconds>(q3 - q2).count();

        cout << "Query tips>5.0:   " << tips_ms << " ms, count=" << r1.count << "\n";
        cout << "Query fare>40.0:  " << fare_ms << " ms, count=" << r2.count << "\n";
        cout << "Query miles>10.0: " << miles_ms << " ms, count=" << r3.count << "\n";

        print_preview(table, args.limit);
        return 0;
    } catch (const std::exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        usage(argv[0]);
        return 2;
    }
}
