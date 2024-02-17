#include "args.hpp"
#include "common.hpp"
#include "lexer.hpp"
#include "preprocessor.hpp"

using CCOMP::Arguments;
using CCOMP::preprocessor;

void run(const Arguments &args) {
    std::string file_content = preprocessor(args);

    if (args.stop_after_preprocessing) {
        printf("%s", file_content.c_str());
        return;
    }

    auto tokens = CCOMP::Lexer(file_content);
    std::unique_ptr<CCOMP::Token> token;

    do {
        token = tokens.next_token();
        trace("%s", token->to_string().c_str());
    } while (token->type != CCOMP::TokenType::END_OF_FILE);

    /* die("Not implemented"); */
}

int main(int argc, char **argv) {
    auto args = Arguments(argc, argv);
    run(args);
}
