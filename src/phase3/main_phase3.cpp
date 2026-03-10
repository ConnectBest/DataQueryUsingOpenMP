#include "common/args.hpp"
#include "phase3/soa_loader.hpp"
#include "phase3/soa_queries_openmp.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using std::cerr;
using std::cout;

static void print_preview(const TripTableSoA& t, std::size_t limit) {
    auto n = std::min(limit, t.size());

    cout << "Loaded rows: " << t.size() << "\n";
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
        auto miles = t.trip_miles[i];
        auto fare = t.base_passenger_fare[i];
        auto tips = t.tips[i];
        auto shared = t.shared_request_flag[i];

        auto miles_s = std::isnan(miles) ? "" : (std::ostringstream{} << std::fixed << std::setprecision(2) << miles).str();
        auto fare_s  = std::isnan(fare)  ? "" : (std::ostringstream{} << std::fixed << std::setprecision(2) << fare).str();
        auto tips_s  = std::isnan(tips)  ? "" : (std::ostringstream{} << std::fixed << std::setprecision(2) << tips).str();

        cout << std::left
             << std::setw(8) << (t.pu_location_id[i] ? std::to_string(t.pu_location_id[i]) : "")
             << std::setw(8) << (t.do_location_id[i] ? std::to_string(t.do_location_id[i]) : "")
             << std::setw(10) << miles_s
             << std::setw(10) << (t.trip_time[i] >= 0 ? std::to_string(t.trip_time[i]) : "")
             << std::setw(12) << fare_s
             << std::setw(12) << tips_s
             << std::setw(8) << (shared == 0 ? "" : (shared == 2 ? "Y" : "N"))
             << "\n";
    }
}

int main(int argc, char** argv) {
    try {
        auto args = parse_args(argc, argv);

        cout << "Phase 3:\n";

        auto t0 = std::chrono::steady_clock::now();
        auto table = load_trip_csv_soa(args.path, args.header);
        auto t1 = std::chrono::steady_clock::now();

        auto load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        cout << "Load time: " << load_ms << " ms\n";

        auto q0 = std::chrono::steady_clock::now();
        auto r1 = query_tips_gt_soa_openmp(table, 5.0);
        auto q1 = std::chrono::steady_clock::now();

        auto r2 = query_fare_gt_soa_openmp(table, 40.0);
        auto q2 = std::chrono::steady_clock::now();

        auto r3 = query_miles_gt_soa_openmp(table, 10.0);
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
