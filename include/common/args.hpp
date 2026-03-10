#pragma once

#include <cstddef>
#include <string>

struct Args {
    std::string path;
    std::size_t limit = 20;
    bool header = true;
};

void usage(const char* prog);
Args parse_args(int argc, char** argv);
