#include "args.hpp"

#include <cstring>

#include "common.hpp"

CCOMP::Arguments::Arguments(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "-E", 2) == 0) {
            trace("Args: stop after preprocessing");
            stop_after_preprocessing = true;
        } else if (strncmp(argv[i], "--dot", 5) == 0) {
            if (i + 1 >= argc) {
                die("No dot file provided");
            }
            trace("Args: dot file %s", argv[i + 1]);
            dot_path = argv[i + 1];
            i++;
        } else {
            trace("Args: source file %s", argv[i]);
            source_path = argv[i];
        }
    }

    if (source_path.empty()) {
        die("No source file provided");
    }
}
