#pragma once

#include <cstdint>
#include <memory>
#include <regex>
#include <string>

namespace CCOMP {

enum class TokenType {
    UNKNOWN = 0,

    // Keywords
    VOID,
    INT,
    RETURN,
    IF,
    ELSE,
    WHILE,
    FOR,
    BREAK,
    CONTINUE,

    // Operators
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,

    // Brackets
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,

    // Punctuation
    SEMICOLON,
    COMMA,
    DOTDOTDOT,

    // Primary
    IDENTIFIER,
    NUMBER,
    STRING,

    // EOF
    END_OF_FILE,
};

std::string token_type_to_string(TokenType type);

struct Token {
    TokenType type;
    uint32_t line, column;
    std::string file;

    Token(TokenType type, uint32_t line, uint32_t column,
          const std::string &file)
        : type(type), line(line), column(column), file(file) {
    }

    virtual std::string to_string() const {
        return token_type_to_string(type) + " at " + file + ":" +
               std::to_string(line) + ":" + std::to_string(column + 1);
    }
};

struct Identifier : Token {
    std::string name;

    Identifier(const std::string &name, uint32_t line, uint32_t column,
               const std::string &file)
        : Token(TokenType::IDENTIFIER, line, column, file), name(name) {
    }

    virtual std::string to_string() const {
        return token_type_to_string(type) + "(" + name + ") at " + file + ":" +
               std::to_string(line) + ":" + std::to_string(column + 1);
    }
};

struct Number : Token {
    std::string value;

    Number(const std::string &value, uint32_t line, uint32_t column,
           const std::string &file)
        : Token(TokenType::NUMBER, line, column, file), value(value) {
    }

    virtual std::string to_string() const {
        return token_type_to_string(type) + "(" + value + ") at " + file + ":" +
               std::to_string(line) + ":" + std::to_string(column + 1);
    }
};

struct String : Token {
    std::string value;

    String(const std::string &value, uint32_t line, uint32_t column,
           const std::string &file)
        : Token(TokenType::STRING, line, column, file), value(value) {
    }

    virtual std::string to_string() const {
        return token_type_to_string(type) + "(" + value + ") at " + file + ":" +
               std::to_string(line) + ":" + std::to_string(column + 1);
    }
};

class Lexer {
   public:
    Lexer(const std::string &source);

    std::unique_ptr<Token> next_token();

   private:
    Token *next_token_internal();

    std::string current_file;
    const char *remaining;
    uint32_t line, column;
};

}  // namespace CCOMP
