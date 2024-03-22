#pragma once

#include <string>

namespace CCOMP::IO {

std::string read_file(const std::string &path);
std::string write_file(const std::string &path, const std::string &content);

std::string exec(const std::string &command);

}  // namespace CCOMP::IO
