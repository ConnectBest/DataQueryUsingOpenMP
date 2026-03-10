#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

std::string_view trim(std::string_view s);
std::optional<int32_t> to_i32(std::string_view s);
std::optional<double> to_f64(std::string_view s);
std::optional<bool> to_flag(std::string_view s);
std::optional<int64_t> to_epoch_seconds(std::string_view s);
