#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "common.hpp"
#include "visitors/ASTVisitor.hpp"

namespace CCOMP::AST {

class AST {
   public:
    AST(uint32_t line, uint32_t column) : line(line), column(column) {
    }
    virtual ~AST() = default;

    [[nodiscard]] uint32_t get_line() const {
        return line;
    }
    [[nodiscard]] uint32_t get_column() const {
        return column;
    }

    virtual void *accept(ASTVisitor &visitor, void *args) = 0;

   public:
    uint32_t line, column;
};

#define AST_VISIT_METHODS()                                  \
    void *accept(ASTVisitor &visitor, void *args) override { \
        return visitor.visit(*this, args);                   \
    }

class Block : public AST {
   public:
    Block(uint32_t line, uint32_t column) : AST(line, column), statements() {
    }

    void add_statement(std::unique_ptr<AST> statement) {
        statements.push_back(std::move(statement));
    }

    AST_VISIT_METHODS()

   public:
    std::vector<std::unique_ptr<AST>> statements;
};

class Constant : public AST {
   public:
    Constant(uint32_t line, uint32_t column, std::string value)
        : AST(line, column), value(std::move(value)) {
    }

    AST_VISIT_METHODS()

   public:
    std::string value;
};

class Identifier : public AST {
   public:
    Identifier(uint32_t line, uint32_t column, std::string name)
        : AST(line, column), name(std::move(name)) {
    }

    AST_VISIT_METHODS()

   public:
    std::string name;
};

class Type : public AST {
   public:
    Type(uint32_t line, uint32_t column) : AST(line, column) {
    }
};

class PrimitiveType : public Type {
   public:
    enum class Primitive {
        NONE,
        VOID,
        INT,
    };

    PrimitiveType(uint32_t line, uint32_t column, Primitive name)
        : Type(line, column), primitive(name) {
    }

    AST_VISIT_METHODS()

    std::string to_string() const {
        switch (primitive) {
            case Primitive::VOID:
                return "void";
            case Primitive::INT:
                return "int";
            default:
                die("Invalid primitive type");
                return "";
        }
    }

   public:
    Primitive primitive;
};

class Declaration : public AST {
   public:
    Declaration(uint32_t line, uint32_t column,
                std::unique_ptr<Identifier> name, std::unique_ptr<Type> type)
        : AST(line, column), name(std::move(name)), type(std::move(type)) {
    }

    bool is_public = true;

   public:
    std::unique_ptr<Identifier> name;
    std::unique_ptr<Type> type;
};

class VariableDeclaration : public Declaration {
   public:
    VariableDeclaration(uint32_t line, uint32_t column,
                        std::unique_ptr<Identifier> name,
                        std::unique_ptr<Type> type, std::unique_ptr<AST> value)
        : Declaration(line, column, std::move(name), std::move(type)),
          value(std::move(value)) {
    }

    AST_VISIT_METHODS()

   public:
    std::unique_ptr<AST> value;
    bool global = false;
};

class ParameterDeclaration : public Declaration {
   public:
    ParameterDeclaration(uint32_t line, uint32_t column,
                         std::unique_ptr<Identifier> name,
                         std::unique_ptr<Type> type)
        : Declaration(line, column, std::move(name), std::move(type)) {
    }

    AST_VISIT_METHODS()
};

class FunctionCall : public AST {
   public:
    FunctionCall(uint32_t line, uint32_t column,
                 std::unique_ptr<Identifier> name)
        : AST(line, column), name(std::move(name)), arguments() {
    }

    AST_VISIT_METHODS()

    void add_argument(std::unique_ptr<AST> argument) {
        arguments.push_back(std::move(argument));
    }

   public:
    std::unique_ptr<Identifier> name;
    std::vector<std::unique_ptr<AST>> arguments;
};

class FunctionDefinition : public Declaration {
   public:
    FunctionDefinition(uint32_t line, uint32_t column,
                       std::unique_ptr<Identifier> name,
                       std::unique_ptr<Type> type, std::unique_ptr<Block> body)
        : Declaration(line, column, std::move(name), std::move(type)),
          parameters(),
          body(std::move(body)) {
    }

    AST_VISIT_METHODS()

    void add_parameter(std::unique_ptr<ParameterDeclaration> parameter) {
        parameters.push_back(std::move(parameter));
    }

   public:
    std::vector<std::unique_ptr<ParameterDeclaration>> parameters;
    std::unique_ptr<Block> body;
};

class FunctionDeclaration : public Declaration {
   public:
    FunctionDeclaration(uint32_t line, uint32_t column,
                        std::unique_ptr<Identifier> name,
                        std::unique_ptr<Type> type)
        : Declaration(line, column, std::move(name), std::move(type)),
          parameters() {
    }

    AST_VISIT_METHODS()

    void add_parameter(std::unique_ptr<ParameterDeclaration> parameter) {
        parameters.push_back(std::move(parameter));
    }

   public:
    std::vector<std::unique_ptr<ParameterDeclaration>> parameters;
};

class Program : public AST {
   public:
    Program(uint32_t line, uint32_t column)
        : AST(line, column), declarations() {
    }

    AST_VISIT_METHODS()

    void add_declaration(std::unique_ptr<AST> declaration) {
        declarations.push_back(std::move(declaration));
    }

   public:
    std::string file_location;
    std::vector<std::unique_ptr<AST>> declarations;
};

class BinaryExpression : public AST {
   public:
    enum class Operator {
        NONE = 0,
        PLUS,
        MINUS,
        MUL,
        DIV,
        REM,
    };

    BinaryExpression(uint32_t line, uint32_t column, std::unique_ptr<AST> left,
                     std::unique_ptr<AST> right, Operator op)
        : AST(line, column),
          left(std::move(left)),
          right(std::move(right)),
          op(op) {
    }

    AST_VISIT_METHODS()

    std::string op_to_str() {
        switch (op) {
            case Operator::PLUS:
                return "+";
            case Operator::MINUS:
                return "-";
            case Operator::MUL:
                return "*";
            case Operator::DIV:
                return "/";
            case Operator::REM:
                return "%";
            default:
                die("Invalid operator");
                return "";
        }
    }

    static inline Operator str_to_op(const std::string &s) {
        switch (s[0]) {
            case '+':
                return Operator::PLUS;
            case '-':
                return Operator::MINUS;
            case '*':
                return Operator::MUL;
            case '/':
                return Operator::DIV;
            case '%':
                return Operator::REM;
            default:
                die("Invalid operator: %s", s.c_str());
                return Operator::NONE;
        }
    }

   public:
    std::unique_ptr<AST> left, right;
    Operator op;
};

class Return : public AST {
   public:
    Return(uint32_t line, uint32_t column, std::unique_ptr<AST> value)
        : AST(line, column), value(std::move(value)) {
    }

    AST_VISIT_METHODS()

   public:
    std::unique_ptr<AST> value;
};

#undef AST_VISIT_METHODS
}  // namespace CCOMP::AST
