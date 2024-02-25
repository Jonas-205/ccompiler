#include "args.hpp"
#include "common.hpp"
#include "io.hpp"
#include "parser.hpp"
#include "preprocessor.hpp"
#include "visitors/dotVisitor.hpp"
#include "visitors/printVisitor.hpp"

using CCOMP::Arguments;
using CCOMP::Parser::parse;

void run(const Arguments &args) {
    std::string file_content = preprocessor(args);

    if (args.stop_after_preprocessing) {
        printf("%s", file_content.c_str());
        return;
    }

    auto ast = parse(file_content);
    ast->file_location = args.source_path;

    // Print the AST
    /* CCOMP::AST::PrintVisitor printer; */
    /* ast->accept(printer, nullptr); */

    // Generate Visually
    if (!args.dot_path.empty()) {
        CCOMP::AST::DotVisitor::generate(*ast, args.dot_path);
    }
}

int main(int argc, char **argv) {
    auto args = Arguments(argc, argv);
    run(args);
}
