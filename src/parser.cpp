#include "parser.hpp"

#include "ANTLRInputStream.h"
#include "antlr/CLexer.h"
#include "antlr/CParser.h"
/* #include "antlr4-runtime.h" */
#include "common.hpp"

using CCOMP::AST::AST;

std::unique_ptr<Program> CCOMP::Parser::parse(const std::string &source) {
    trace("Parsing source code");

    antlr4::ANTLRInputStream input(source);
    CLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    CParser parser(&tokens);

    auto tree = parser.program();

    return std::move(tree->ast);
}
