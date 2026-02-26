#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

using std::cerr;
using std::cout;
using std::string;

// ---------- CLI parsing ----------

struct Args {
    std::string path;
    std::size_t limit = 20;   // rows to print
    bool header = true;
};

static void usage(const char* prog) {
    cerr << "Usage: " << prog << " -p <file.csv> [--limit N] [--no-header]\n";
}

static Args parse_args(int argc, char** argv) {
    Args a{};
    for (int i = 1; i < argc; i++) {
        std::string_view arg = argv[i];

        if (arg == "-p") {
            if (i + 1 >= argc) throw std::runtime_error("Missing value for -p");
            a.path = argv[++i];
        } else if (arg == "--limit") {
            if (i + 1 >= argc) throw std::runtime_error("Missing value for --limit");
            a.limit = static_cast<std::size_t>(std::stoull(argv[++i]));
        } else if (arg == "--no-header") {
            a.header = false;
        } else if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            std::exit(0);
        } else {
            throw std::runtime_error("Unknown arg: " + std::string(arg));
        }
    }

    if (a.path.empty()) throw std::runtime_error("You must provide -p <file.csv>");
    return a;
}

// ---------- CSV parsing ----------

static std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> out;
    out.reserve(32);

    std::string field;
    field.reserve(64);

    bool in_quotes = false;

    for (std::size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (in_quotes) {
            if (c == '"') {
                // Double quote inside quoted field => escaped quote
                if (i + 1 < line.size() && line[i + 1] == '"') {
                    field.push_back('"');
                    i++;
                } else {
                    in_quotes = false;
                }
            } else {
                field.push_back(c);
            }
        } else {
            if (c == '"') {
                in_quotes = true;
            } else if (c == ',') {
                out.push_back(std::move(field));
                field.clear();
            } else if (c == '\r') {
                // ignore CR
            } else {
                field.push_back(c);
            }
        }
    }

    out.push_back(std::move(field));
    return out;
}

// ---------- Primitive conversions ----------

static inline std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
    return s;
}

static std::optional<int32_t> to_i32(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;
    // fast-ish parse
    int sign = 1;
    std::size_t i = 0;
    if (s[0] == '-') { sign = -1; i = 1; }
    int64_t v = 0;
    for (; i < s.size(); i++) {
        char c = s[i];
        if (c < '0' || c > '9') break;
        v = v * 10 + (c - '0');
    }
    return static_cast<int32_t>(v * sign);
}

static std::optional<int64_t> to_i64(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;
    int sign = 1;
    std::size_t i = 0;
    if (s[0] == '-') { sign = -1; i = 1; }
    int64_t v = 0;
    for (; i < s.size(); i++) {
        char c = s[i];
        if (c < '0' || c > '9') break;
        v = v * 10 + (c - '0');
    }
    return v * sign;
}

static std::optional<double> to_f64(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;
    // std::strtod expects null-terminated
    std::string tmp(s);
    char* end = nullptr;
    double v = std::strtod(tmp.c_str(), &end);
    if (end == tmp.c_str()) return std::nullopt;
    return v;
}

static std::optional<bool> to_flag(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;
    // Your sample uses Y/N and sometimes blank / space.
    char c = s.front();
    if (c == 'Y' || c == 'y') return true;
    if (c == 'N' || c == 'n') return false;
    return std::nullopt;
}

static std::optional<int64_t> to_epoch_seconds(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;
    if (s.size() < 19) return std::nullopt;

    auto dig2 = [&](std::size_t pos) -> int {
        return (s[pos] - '0') * 10 + (s[pos + 1] - '0');
    };

    std::tm tm{};
    tm.tm_year = (s[0]-'0')*1000 + (s[1]-'0')*100 + (s[2]-'0')*10 + (s[3]-'0') - 1900;
    tm.tm_mon  = dig2(5) - 1;
    tm.tm_mday = dig2(8);
    tm.tm_hour = dig2(11);
    tm.tm_min  = dig2(14);
    tm.tm_sec  = dig2(17);

#if defined(__APPLE__) || defined(__linux__)
    time_t t = timegm(&tm); // treat as UTC
    if (t == (time_t)-1) return std::nullopt;
    return static_cast<int64_t>(t);
#else
    // Portable fallback: interpret as local time (less ideal)
    time_t t = std::mktime(&tm);
    if (t == (time_t)-1) return std::nullopt;
    return static_cast<int64_t>(t);
#endif
}

// ---------- Data model ----------

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
    std::optional<int32_t> trip_time; // seconds

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

struct TripTable {
    std::vector<TripRecord> rows;
};

// Map columns by index based on header ordering.
static TripRecord parse_trip_row(const std::vector<std::string>& f) {
    if (f.size() != 24) {
        throw std::runtime_error(
            "Unexpected column count. Expected 24, got: " +
            std::to_string(f.size())
        );
    }

    TripRecord r{};

    r.hvfhs_license_num    = f[0];
    r.dispatching_base_num = f[1];
    r.originating_base_num = f[2];

    r.request_ts  = to_epoch_seconds(f[3]);
    r.on_scene_ts = to_epoch_seconds(f[4]);
    r.pickup_ts   = to_epoch_seconds(f[5]);
    r.dropoff_ts  = to_epoch_seconds(f[6]);

    r.pu_location_id = to_i32(f[7]);
    r.do_location_id = to_i32(f[8]);

    r.trip_miles = to_f64(f[9]);
    r.trip_time  = to_i32(f[10]);

    r.base_passenger_fare  = to_f64(f[11]);
    r.tolls                = to_f64(f[12]);
    r.bcf                  = to_f64(f[13]);
    r.sales_tax            = to_f64(f[14]);
    r.congestion_surcharge = to_f64(f[15]);
    r.airport_fee          = to_f64(f[16]);
    r.tips                 = to_f64(f[17]);
    r.driver_pay           = to_f64(f[18]);

    r.shared_request_flag = to_flag(f[19]);
    r.shared_match_flag   = to_flag(f[20]);
    r.access_a_ride_flag  = to_flag(f[21]);
    r.wav_request_flag    = to_flag(f[22]);
    r.wav_match_flag      = to_flag(f[23]);

    return r;
}

// ---------- Reader ----------

static TripTable load_trip_csv(const std::string& path, bool has_header) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Failed to open: " + path);

    TripTable t{};
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

        try {
            t.rows.push_back(parse_trip_row(fields));
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Parse error: ") + e.what());
        }
    }

    return t;
}

// ---------- Querying ----------

struct QueryResult {
    std::size_t count = 0;
    double sum = 0.0;
};

static QueryResult query_tips_gt(const TripTable& t, double threshold) {
    QueryResult r;
    for (const auto& row : t.rows) {
        if (row.tips && *row.tips > threshold) {
            r.count++;
            r.sum += *row.tips;
        }
    }
    return r;
}

static QueryResult query_fare_gt(const TripTable& t, double threshold) {
    QueryResult r;
    for (const auto& row : t.rows) {
        if (row.base_passenger_fare && *row.base_passenger_fare > threshold) {
            r.count++;
            r.sum += *row.base_passenger_fare;
        }
    }
    return r;
}

static QueryResult query_miles_gt(const TripTable& t, double threshold) {
    QueryResult r;
    for (const auto& row : t.rows) {
        if (row.trip_miles && *row.trip_miles > threshold) {
            r.count++;
            r.sum += *row.trip_miles;
        }
    }
    return r;
}

// ---------- Pretty print table ----------

template <typename T>
static std::string opt_to_string(const std::optional<T>& v) {
    if (!v) return "";
    if constexpr (std::is_same_v<T, bool>) return *v ? "Y" : "N";
    return std::to_string(*v);
}

static void print_preview(const TripTable& t, std::size_t limit) {
    auto n = std::min(limit, t.rows.size());

    cout << "Loaded rows: " << t.rows.size() << "\n";
    cout << "Preview first " << n << " rows:\n\n";

    cout << std::left
         << std::setw(8)  << "PU"
         << std::setw(8)  << "DO"
         << std::setw(10) << "miles"
         << std::setw(10) << "time_s"
         << std::setw(12) << "fare"
         << std::setw(12) << "tips"
         << std::setw(8)  << "shared"
         << "\n";

    cout << std::string(68, '-') << "\n";

    for (std::size_t i = 0; i < n; i++) {
        const auto& r = t.rows[i];
        cout << std::left
             << std::setw(8)  << opt_to_string(r.pu_location_id)
             << std::setw(8)  << opt_to_string(r.do_location_id)
             << std::setw(10) << (r.trip_miles ? (std::ostringstream{} << std::fixed << std::setprecision(2) << *r.trip_miles).str() : "")
             << std::setw(10) << opt_to_string(r.trip_time)
             << std::setw(12) << (r.base_passenger_fare ? (std::ostringstream{} << std::fixed << std::setprecision(2) << *r.base_passenger_fare).str() : "")
             << std::setw(12) << (r.tips ? (std::ostringstream{} << std::fixed << std::setprecision(2) << *r.tips).str() : "")
             << std::setw(8)  << opt_to_string(r.shared_request_flag)
             << "\n";
    }
}

// ---------- main ----------

int main(int argc, char** argv) {
    try {
        auto args = parse_args(argc, argv);

        auto t0 = std::chrono::steady_clock::now();
        TripTable table = load_trip_csv(args.path, args.header);
        auto t1 = std::chrono::steady_clock::now();

        auto load_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
        cout << "Load time: " << load_ms << " ms\n";

        auto q0 = std::chrono::steady_clock::now();
        auto r1 = query_tips_gt(table, 5.0);
        auto q1 = std::chrono::steady_clock::now();

        auto r2 = query_fare_gt(table, 40.0);
        auto q2 = std::chrono::steady_clock::now();

        auto r3 = query_miles_gt(table, 10.0);
        auto q3 = std::chrono::steady_clock::now();

        auto tips_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(q1 - q0).count();
        auto fare_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(q2 - q1).count();
        auto miles_ms = std::chrono::duration_cast<std::chrono::milliseconds>(q3 - q2).count();

        std::cout << "Query tips>5.0:   " << tips_ms  << " ms, count=" << r1.count << "\n";
        std::cout << "Query fare>40.0:  " << fare_ms  << " ms, count=" << r2.count << "\n";
        std::cout << "Query miles>10.0: " << miles_ms << " ms, count=" << r3.count << "\n";

        print_preview(table, args.limit);
        return 0;
    } catch (const std::exception& e) {
        cerr << "ERROR: " << e.what() << "\n";
        usage(argv[0]);
        return 2;
    }
}