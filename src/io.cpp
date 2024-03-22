#include "io.hpp"

#include <array>
#include <fstream>
#include <memory>
#include <sstream>

#include "common.hpp"

namespace CCOMP {
namespace IO {

std::string read_file(const std::string &path) {
    trace("Reading file %s", path.c_str());
    std::ifstream file_stream(path);
    if (!file_stream) {
        die("Could not open file: %s", path.c_str());
    }
    std::stringstream buffer;
    buffer << file_stream.rdbuf();
    return buffer.str();
}

std::string exec(const std::string &command) {
    info("Exec: %s", command.c_str());
    std::array<char, 128> buffer;
    std::string result;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe) {
        die("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        result += buffer.data();
    }

    auto status = pclose(pipe);
    if (status == -1) {
        die("pclose() failed!");
    }

    return result;
}

std::string write_file(const std::string &path, const std::string &content) {
    trace("Writing file %s", path.c_str());
    std::ofstream file_stream(path);
    if (!file_stream) {
        die("Could not open file: %s", path.c_str());
    }
    file_stream << content;
    return path;
}

}  // namespace IO
}  // namespace CCOMP
