#include "lexer.hpp"

#include <cstring>
#include <memory>

#include "common.hpp"

using CCOMP::Lexer;
using CCOMP::Token;

Lexer::Lexer(const std::string &source) {
    remaining = source.c_str();  // source will will outlive remaining
    line = 0;
    column = 0;
}

static std::string parse_file_name(const std::string &text) {
    size_t pos = text.find("\"");

    if (pos == std::string::npos) {
        die("Can not find file name");
    }

    std::string erg = text.substr(pos + 1);

    pos = erg.find("\"");

    if (pos == std::string::npos) {
        die("Can not find file name");
    }

    return erg.substr(0, pos);
}

static inline bool is_number(char c) {
    return c >= '0' && c <= '9';
}

static inline bool is_lower(char c) {
    return c >= 'a' && c <= 'z';
}

static inline bool is_upper(char c) {
    return c >= 'A' && c <= 'Z';
}

static inline bool is_alpha(char c) {
    return is_lower(c) || is_upper(c);
}

static inline bool is_alphanum(char c) {
    return is_alpha(c) || is_number(c);
}

std::unique_ptr<Token> Lexer::next_token() {
    Token *token;

    do {
        token = next_token_internal();
    } while (token == nullptr);

    return std::unique_ptr<Token>(token);
}

Token *Lexer::next_token_internal() {
    const char *start = remaining;
#define CASE(x, type) \
    case x:           \
        remaining++;  \
        column++;     \
        return new Token(TokenType::type, line, column - 1, current_file)

    switch (*remaining) {
        case '\0':
            return new Token(TokenType::END_OF_FILE, line, column,
                             current_file);
        case ' ':
        case '\t':
        case '\r':
            remaining++;
            column++;
            return nullptr;
        case '\n':
            remaining++;
            line++;
            column = 0;
            return nullptr;
        case '#':
            while (*remaining != '\n' && *remaining != '\0') {
                remaining++;
            }
            line = 0;
            current_file =
                parse_file_name(std::string(start, remaining - start));
            return nullptr;
            CASE('+', PLUS);
            CASE('-', MINUS);
            CASE('*', STAR);
            CASE('/', SLASH);
            CASE('%', PERCENT);
            CASE('(', LPAREN);
            CASE(')', RPAREN);
            CASE('{', LBRACE);
            CASE('}', RBRACE);
            CASE('[', LBRACKET);
            CASE(']', RBRACKET);
            CASE(';', SEMICOLON);
            CASE(',', COMMA);
    }
#undef CASE

#define KEYWORD(word, type)                                       \
    do {                                                          \
        size_t len = strlen(word);                                \
        if (strncmp(remaining, word, len) == 0) {                 \
            remaining += len;                                     \
            column += len;                                        \
            return new Token(TokenType::type, line, column - len, \
                             current_file);                       \
        }                                                         \
    } while (0)

    KEYWORD("void", VOID);
    KEYWORD("...", DOTDOTDOT);
    KEYWORD("int", INT);
    KEYWORD("return", RETURN);
    KEYWORD("if", IF);
    KEYWORD("else", ELSE);
    KEYWORD("while", WHILE);
    KEYWORD("for", FOR);
    KEYWORD("break", BREAK);
#undef KEYWORD

    if (is_number(*remaining)) {
        bool had_dot = false;

        while (is_number(*remaining) || (!had_dot && *remaining == '.')) {
            remaining++;
            column++;
            if (*remaining == '.') {
                had_dot = true;
            }
        }
        return new Number(std::string(start, remaining - start), line,
                          column - (remaining - start), current_file);
    } else if (is_alpha(*remaining) || *remaining == '_') {
        while (is_alphanum(*remaining) || *remaining == '_') {
            remaining++;
            column++;
        }
        return new Identifier(std::string(start, remaining - start), line,
                              column - (remaining - start), current_file);
    } else if (*remaining == '"') {
        remaining++;
        column++;

        bool found = false;

        while (!found) {
            char prev = *(remaining - 1);

            if (*remaining == '\0') {
                die("Unterminated string");
            }

            if (*remaining == '\n' && prev != '\\') {
                die("Unterminated string");
            }

            if (*remaining == '"' && prev != '\\') {
                found = true;
            }

            remaining++;
            column++;
        }
        return new String(std::string(start + 1, remaining - start - 2), line,
                          column - (remaining - start), current_file);
    }

    die("Can not parse token: %.50s", remaining);
}

std::string CCOMP::token_type_to_string(TokenType type) {
#define CASE(x)        \
    case TokenType::x: \
        return #x;

    switch (type) {
        case TokenType::UNKNOWN:
            die("Unknown token type");
            CASE(VOID)
            CASE(INT)
            CASE(RETURN)
            CASE(IF)
            CASE(ELSE)
            CASE(WHILE)
            CASE(FOR)
            CASE(BREAK)
            CASE(CONTINUE)
            CASE(PLUS)
            CASE(MINUS)
            CASE(STAR)
            CASE(SLASH)
            CASE(PERCENT)
            CASE(LPAREN)
            CASE(RPAREN)
            CASE(LBRACE)
            CASE(RBRACE)
            CASE(LBRACKET)
            CASE(RBRACKET)
            CASE(SEMICOLON)
            CASE(COMMA)
            CASE(DOTDOTDOT)
            CASE(IDENTIFIER)
            CASE(NUMBER)
            CASE(STRING)
            CASE(END_OF_FILE)
    }
    die("Unknown token type");
#undef CASE
}
