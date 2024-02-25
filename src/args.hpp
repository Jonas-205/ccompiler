#pragma once

#include <iostream>

namespace CCOMP {

struct Arguments {
    Arguments(int argc, char **argv);

   public:
    std::string source_path;
    std::string dot_path;

    bool stop_after_preprocessing = false;
};

}  // namespace CCOMP
