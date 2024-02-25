#pragma once

#include "ast.hpp"

namespace CCOMP::Parser {
std::unique_ptr<CCOMP::AST::Program> parse(const std::string &source);
}  // namespace CCOMP::Parser
