#include "common/args.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

void usage(const char* prog) {
    std::cerr << "Usage: " << prog << " -p <file.csv> [--limit N] [--no-header]\n";
}

Args parse_args(int argc, char** argv) {
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
