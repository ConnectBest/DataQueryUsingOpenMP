#include "common/csv.hpp"

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> out;
    out.reserve(32);

    std::string field;
    field.reserve(64);

    bool in_quotes = false;

    for (std::size_t i = 0; i < line.size(); i++) {
        char c = line[i];

        if (in_quotes) {
            if (c == '"') {
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
            } else {
                field.push_back(c);
            }
        }
    }

    out.push_back(std::move(field));
    return out;
}
