#include "common/convert.hpp"

#include <cstdlib>
#include <ctime>
#include <string>

std::string_view trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
    return s;
}

std::optional<int32_t> to_i32(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;

    int sign = 1;
    std::size_t i = 0;
    if (s[0] == '-') {
        sign = -1;
        i = 1;
    }

    int64_t v = 0;
    for (; i < s.size(); i++) {
        char c = s[i];
        if (c < '0' || c > '9') break;
        v = v * 10 + (c - '0');
    }
    return static_cast<int32_t>(v * sign);
}

std::optional<double> to_f64(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;

    std::string tmp(s);
    char* end = nullptr;
    double v = std::strtod(tmp.c_str(), &end);
    if (end == tmp.c_str()) return std::nullopt;
    return v;
}

std::optional<bool> to_flag(std::string_view s) {
    s = trim(s);
    if (s.empty()) return std::nullopt;

    char c = s.front();
    if (c == 'Y' || c == 'y') return true;
    if (c == 'N' || c == 'n') return false;
    return std::nullopt;
}

std::optional<int64_t> to_epoch_seconds(std::string_view s) {
    s = trim(s);
    if (s.empty() || s.size() < 19) return std::nullopt;

    auto dig2 = [&](std::size_t pos) -> int {
        return (s[pos] - '0') * 10 + (s[pos + 1] - '0');
    };

    std::tm tm{};
    tm.tm_year = (s[0] - '0') * 1000 + (s[1] - '0') * 100 + (s[2] - '0') * 10 + (s[3] - '0') - 1900;
    tm.tm_mon = dig2(5) - 1;
    tm.tm_mday = dig2(8);
    tm.tm_hour = dig2(11);
    tm.tm_min = dig2(14);
    tm.tm_sec = dig2(17);

#if defined(__APPLE__) || defined(__linux__)
    time_t t = timegm(&tm);
    if (t == (time_t)-1) return std::nullopt;
    return static_cast<int64_t>(t);
#else
    time_t t = std::mktime(&tm);
    if (t == (time_t)-1) return std::nullopt;
    return static_cast<int64_t>(t);
#endif
}
