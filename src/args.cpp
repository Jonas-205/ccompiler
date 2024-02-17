#include "args.hpp"

#include "common.hpp"

CCOMP::Arguments::Arguments(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i] == std::string("-E")) {
            trace("Args: stop after preprocessing");
            stop_after_preprocessing = true;
        } else {
            trace("Args: source file %s", argv[i]);
            source_path = argv[i];
        }
    }

    if (source_path.empty()) {
        die("No source file provided");
    }
}
