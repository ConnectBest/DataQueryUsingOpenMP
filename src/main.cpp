#include <chrono>
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
#include <sstream>
#include <limits>
#include <cmath>

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

    char buf[64];
    if (s.size() >= sizeof(buf)) return std::nullopt; // or handle longer
    std::memcpy(buf, s.data(), s.size());
    buf[s.size()] = '\0';

    char* end = nullptr;
    double v = std::strtod(buf, &end);
    if (end == buf) return std::nullopt;
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

static inline double kNaN() { return std::numeric_limits<double>::quiet_NaN(); }
static inline bool is_missing(double v) { return std::isnan(v); }

static inline int64_t i64_or_missing(const std::optional<int64_t>& v) { return v ? *v : -1; }
static inline int32_t i32_or_missing(const std::optional<int32_t>& v) { return v ? *v : -1; }
static inline uint16_t u16_or_missing(const std::optional<int32_t>& v) { return v ? static_cast<uint16_t>(*v) : 0; }
static inline double f64_or_missing(const std::optional<double>& v) { return v ? *v : kNaN(); }
static inline uint8_t flag_or_missing(const std::optional<bool>& v) {
    if (!v) return 0;
    return *v ? 2 : 1;
}

struct TripTable {
    // Keep strings for now if you must; Phase 3 best is to dictionary-encode.
    // If you want to keep it simple for first pass, store strings in separate vectors:
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

        request_ts.reserve(n); on_scene_ts.reserve(n); pickup_ts.reserve(n); dropoff_ts.reserve(n);
        pu_location_id.reserve(n); do_location_id.reserve(n);
        trip_miles.reserve(n); trip_time.reserve(n);

        base_passenger_fare.reserve(n); tolls.reserve(n); bcf.reserve(n); sales_tax.reserve(n);
        congestion_surcharge.reserve(n); airport_fee.reserve(n); tips.reserve(n); driver_pay.reserve(n);

        shared_request_flag.reserve(n); shared_match_flag.reserve(n); access_a_ride_flag.reserve(n);
        wav_request_flag.reserve(n); wav_match_flag.reserve(n);
    }
};

static void append_trip_row(TripTable& t, const std::vector<std::string>& f) {
    if (f.size() != 24) {
        throw std::runtime_error("Unexpected column count. Expected 24, got: " + std::to_string(f.size()));
    }

    // strings (kept as separate vectors; can be dictionary-encoded later)
    t.hvfhs_license_num.push_back(f[0]);
    t.dispatching_base_num.push_back(f[1]);
    t.originating_base_num.push_back(f[2]);

    // timestamps
    t.request_ts.push_back(i64_or_missing(to_epoch_seconds(f[3])));
    t.on_scene_ts.push_back(i64_or_missing(to_epoch_seconds(f[4])));
    t.pickup_ts.push_back(i64_or_missing(to_epoch_seconds(f[5])));
    t.dropoff_ts.push_back(i64_or_missing(to_epoch_seconds(f[6])));

    // ids
    t.pu_location_id.push_back(u16_or_missing(to_i32(f[7])));
    t.do_location_id.push_back(u16_or_missing(to_i32(f[8])));

    // metrics
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

    // flags as compact tristate
    t.shared_request_flag.push_back(flag_or_missing(to_flag(f[19])));
    t.shared_match_flag.push_back(flag_or_missing(to_flag(f[20])));
    t.access_a_ride_flag.push_back(flag_or_missing(to_flag(f[21])));
    t.wav_request_flag.push_back(flag_or_missing(to_flag(f[22])));
    t.wav_match_flag.push_back(flag_or_missing(to_flag(f[23])));
}

// ---------- Reader ----------

static TripTable load_trip_csv(const std::string& path, bool has_header) {
    std::ifstream in(path);
    if (!in) throw std::runtime_error("Failed to open: " + path);

    TripTable t{};
    t.reserve(100'000'000);

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
            append_trip_row(t, fields);
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
    std::size_t count = 0;
    double sum = 0.0;

    const auto& tips = t.tips;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(tips.size()); i++) {
        double v = tips[i];
        if (!std::isnan(v) && v > threshold) {
            count++;
            sum += v;
        }
    }

    return QueryResult{count, sum};
}

static QueryResult query_fare_gt(const TripTable& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    const auto& fare = t.base_passenger_fare;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(fare.size()); i++) {
        double v = fare[i];
        if (!std::isnan(v) && v > threshold) {
            count++;
            sum += v;
        }
    }

    return QueryResult{count, sum};
}

static QueryResult query_miles_gt(const TripTable& t, double threshold) {
    std::size_t count = 0;
    double sum = 0.0;

    const auto& miles = t.trip_miles;

    #pragma omp parallel for reduction(+:count,sum) schedule(static)
    for (std::int64_t i = 0; i < static_cast<std::int64_t>(miles.size()); i++) {
        double v = miles[i];
        if (!std::isnan(v) && v > threshold) {
            count++;
            sum += v;
        }
    }

    return QueryResult{count, sum};
}

// ---------- Pretty print table ----------

template <typename T>
static std::string opt_to_string(const std::optional<T>& v) {
    if (!v) return "";
    if constexpr (std::is_same_v<T, bool>) return *v ? "Y" : "N";
    return std::to_string(*v);
}

static void print_preview(const TripTable& t, std::size_t limit) {
    auto n = std::min(limit, t.size());
    cout << "Loaded rows: " << t.size() << "\n";
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
        auto pu = t.pu_location_id[i];
        auto d0 = t.do_location_id[i];
        auto miles = t.trip_miles[i];
        auto time_s = t.trip_time[i];
        auto fare = t.base_passenger_fare[i];
        auto tips = t.tips[i];
        auto shared = t.shared_request_flag[i]; // 0/1/2

        auto miles_s = std::isnan(miles) ? "" : (std::ostringstream{} << std::fixed << std::setprecision(2) << miles).str();
        auto fare_s  = std::isnan(fare)  ? "" : (std::ostringstream{} << std::fixed << std::setprecision(2) << fare).str();
        auto tips_s  = std::isnan(tips)  ? "" : (std::ostringstream{} << std::fixed << std::setprecision(2) << tips).str();

        cout << std::left
             << std::setw(8)  << (pu ? std::to_string(pu) : "")
             << std::setw(8)  << (d0 ? std::to_string(d0) : "")
             << std::setw(10) << miles_s
             << std::setw(10) << (time_s >= 0 ? std::to_string(time_s) : "")
             << std::setw(12) << fare_s
             << std::setw(12) << tips_s
             << std::setw(8)  << (shared == 0 ? "" : (shared == 2 ? "Y" : "N"))
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