#pragma once

#include <cstdint>
#include <memory>
#include <sstream>
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

class Type : public virtual AST {
   public:
    Type(uint32_t line, uint32_t column) : AST(line, column) {
    }

    void set_array_dimensions(int dimensions) {
        array_dimensions = dimensions;
        array_sizes.resize(dimensions);
    }

    void set_array_dimension(int i, std::unique_ptr<AST> dimension) {
        if (i >= array_sizes.size()) {
            die("Invalid array dimension: %d", i);
        }
        array_sizes[i] = std::move(dimension);
    }

   public:
    bool is_pointer = false;
    int array_dimensions = 0;
    std::vector<std::unique_ptr<AST>> array_sizes;
};

class NamedType : public Type {
   public:
    NamedType(uint32_t line, uint32_t column, std::unique_ptr<Identifier> name)
        : Type(line, column), AST(line, column) {
        this->name = name->name;
    }

    AST_VISIT_METHODS()

   public:
    std::string name;
};

class PrimitiveType : public Type {
   public:
    enum class KeyWords {
        VOID,
        SIGNED,
        UNSIGNED,
        CHAR,
        SHORT,
        INT,
        LONG,
        VA_LIST,
    };

    PrimitiveType(uint32_t line, uint32_t column)
        : Type(line, column), AST(line, column) {
    }

    AST_VISIT_METHODS()

    void add_keyword(KeyWords k) {
        keywords.push_back(k);
    }

    [[nodiscard]] std::string to_string() const {
        std::stringstream s;
        for (KeyWords k : keywords) {
            switch (k) {
                case KeyWords::VOID:
                    s << "void ";
                    break;
                case KeyWords::SIGNED:
                    s << "signed ";
                    break;
                case KeyWords::UNSIGNED:
                    s << "unsigned ";
                    break;
                case KeyWords::CHAR:
                    s << "char ";
                    break;
                case KeyWords::SHORT:
                    s << "short ";
                    break;
                case KeyWords::INT:
                    s << "int ";
                    break;
                case KeyWords::LONG:
                    s << "long ";
                    break;
                case KeyWords::VA_LIST:
                    s << "va_list ";
                    break;
            }
        }
        return s.str();
    }

   public:
    std::vector<KeyWords> keywords;
};

class Declaration : public virtual AST {
   public:
    Declaration(uint32_t line, uint32_t column, std::unique_ptr<AST> name,
                std::unique_ptr<Type> type)
        : AST(line, column),
          name(std::move(name)),
          m_type(std::move(type)),
          m_type_ref(nullptr) {
    }

    Declaration(uint32_t line, uint32_t column, std::unique_ptr<AST> name,
                Type *type)
        : AST(line, column),
          name(std::move(name)),
          m_type(nullptr),
          m_type_ref(type) {
    }

    bool is_public = true;

    Type *type() {
        if (m_type) {
            return m_type.get();
        }
        return m_type_ref;
    }

   public:
    std::unique_ptr<AST> name;

   private:
    std::unique_ptr<Type> m_type;
    Type *m_type_ref;
};

class TypeDef : public Declaration {
   public:
    TypeDef(uint32_t line, uint32_t column, std::unique_ptr<Type> org,
            std::unique_ptr<NamedType> named)
        : Declaration(line, column, std::move(named), std::move(org)),
          AST(line, column) {
    }

    AST_VISIT_METHODS();
};

class VariableDeclaration : public Declaration {
   public:
    VariableDeclaration(uint32_t line, uint32_t column,
                        std::unique_ptr<Identifier> name,
                        std::unique_ptr<Type> type, std::unique_ptr<AST> value)
        : Declaration(line, column, std::move(name), std::move(type)),
          AST(line, column),
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
        : Declaration(line, column, std::move(name), std::move(type)),
          AST(line, column) {
    }

    AST_VISIT_METHODS()
};

class ArrayInitializationList : public AST {
   public:
    ArrayInitializationList(uint32_t line, uint32_t column)
        : AST(line, column), values() {
    }

    AST_VISIT_METHODS()

    void add_value(std::unique_ptr<AST> value) {
        values.push_back(std::move(value));
    }

   public:
    std::vector<std::unique_ptr<AST>> values;
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
          AST(line, column),
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
          AST(line, column),
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

class UnaryExpression : public AST {
   public:
    enum class Operator {
        NONE = 0,
        DEREFERENCE,
        ADDRESS,
    };

    UnaryExpression(uint32_t line, uint32_t column, std::unique_ptr<AST> value,
                    Operator op)
        : AST(line, column), value(std::move(value)), op(op) {
    }

    AST_VISIT_METHODS()

    std::string op_to_str() {
        switch (op) {
            case Operator::DEREFERENCE:
                return "deref";
            case Operator::ADDRESS:
                return "addr of";
            default:
                die("Invalid operator");
                return "";
        }
    }

    static inline Operator str_to_op(const std::string &s) {
        switch (s[0]) {
            case '*':
                return Operator::DEREFERENCE;
            case '&':
                return Operator::ADDRESS;
            default:
                die("Invalid operator: %s", s.c_str());
                return Operator::NONE;
        }
    }

   public:
    std::unique_ptr<AST> value;
    Operator op;
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

class StructDeclaration : public Declaration, public Type {
   public:
    StructDeclaration(uint32_t line, uint32_t column,
                      std::unique_ptr<Identifier> name)
        : Declaration(line, column, std::move(name), this),
          Type(line, column),
          AST(line, column) {
    }

    AST_VISIT_METHODS()
};

class StructDefinition : public StructDeclaration {
   public:
    StructDefinition(uint32_t line, uint32_t column,
                     std::unique_ptr<Identifier> name)
        : StructDeclaration(line, column, std::move(name)), AST(line, column) {
    }

    AST_VISIT_METHODS()

    void add_member(std::unique_ptr<VariableDeclaration> member) {
        members.push_back(std::move(member));
    }

   public:
    std::vector<std::unique_ptr<VariableDeclaration>> members;
};

class UnionDeclaration : public Declaration, public Type {
   public:
    UnionDeclaration(uint32_t line, uint32_t column,
                     std::unique_ptr<Identifier> name)
        : Declaration(line, column, std::move(name), this),
          Type(line, column),
          AST(line, column) {
    }

    AST_VISIT_METHODS()
};

class UnionDefinition : public UnionDeclaration {
   public:
    UnionDefinition(uint32_t line, uint32_t column,
                    std::unique_ptr<Identifier> name)
        : UnionDeclaration(line, column, std::move(name)), AST(line, column) {
    }

    AST_VISIT_METHODS()

    void add_member(std::unique_ptr<VariableDeclaration> member) {
        members.push_back(std::move(member));
    }

   public:
    std::vector<std::unique_ptr<VariableDeclaration>> members;
};

class SizeOf : public AST {
   public:
    SizeOf(uint32_t line, uint32_t column, std::unique_ptr<Type> type)
        : AST(line, column), type(std::move(type)) {
    }

    AST_VISIT_METHODS()

   public:
    std::unique_ptr<Type> type;
};

#undef AST_VISIT_METHODS
}  // namespace CCOMP::AST
