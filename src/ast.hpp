#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "common.hpp"
#include "visitors/ASTVisitor.hpp"

/* #define AST_TRACE(p)                                              \ */
/*     do {                                                          \ */
/*         printf("Visiting %s at %d:%d (", __func__, line, column); \ */
/*         std::cout << p;                                           \ */
/*         printf(")\n");                                            \ */
/*     } while (0) */

#define AST_TRACE(p)

namespace CCOMP::AST {

class AST {
   public:
    AST(uint32_t line, uint32_t column) : line(line), column(column) {
        AST_TRACE(line << ":" << column);
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
        AST_TRACE(line << ":" << column);
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
        AST_TRACE(line << ":" << column << " " << value);
    }

    AST_VISIT_METHODS()

   public:
    std::string value;
};

class Type : public virtual AST {
   public:
    Type(uint32_t line, uint32_t column) : AST(line, column) {
        AST_TRACE(line << ":" << column);
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

class Identifier : public AST {
   public:
    Identifier(uint32_t line, uint32_t column, std::string name)
        : AST(line, column),
          name(std::move(name)),
          type_owned(nullptr),
          type_ref(nullptr) {
        AST_TRACE(line << ":" << column << " " << name);
    }

    AST_VISIT_METHODS()

    void add_type(std::unique_ptr<Type> p_type) {
        this->type_owned = std::move(p_type);
        if (add_ptr_later) {
            this->type_owned->is_pointer = true;
        }
    }

    void add_type(Type *p_type) {
        this->type_ref = p_type;
        if (add_ptr_later) {
            p_type->is_pointer = true;
        }
    }

    Type *type() {
        if (type_ref) {
            return type_ref;
        }
        return type_owned.get();
    }

   public:
    std::string name;
    bool add_ptr_later = false;  // Add pointer to type once type is known

   private:
    std::unique_ptr<Type> type_owned;
    Type *type_ref;
};

class NamedType : public Type {
   public:
    NamedType(std::unique_ptr<Identifier> name)
        : Type(name->line, name->column), AST(name->line, name->column) {
        AST_TRACE(line << ":" << column << " " << name->name);
        this->name = name->name;
    }

    AST_VISIT_METHODS()

   public:
    std::string name;
};

class FunctionType : public Type {
   public:
    FunctionType(uint32_t line, uint32_t column,
                 std::unique_ptr<Type> return_type)
        : Type(line, column),
          AST(line, column),
          return_type(std::move(return_type)) {
        AST_TRACE(line << ":" << column);
    }

    AST_VISIT_METHODS()

    void add_parameter(std::unique_ptr<ParameterDeclaration> parameter) {
        parameters.push_back(std::move(parameter));
    }

   public:
    std::unique_ptr<Type> return_type;
    std::vector<std::unique_ptr<ParameterDeclaration>> parameters;
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
        AST_TRACE(line << ":" << column);
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
    Declaration(uint32_t line, uint32_t column,
                std::unique_ptr<Identifier> name)
        : AST(line, column), name(std::move(name)), m_type(nullptr) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    Declaration(uint32_t line, uint32_t column, std::unique_ptr<Type> type)
        : AST(line, column), name(nullptr), m_type(std::move(type)) {
        AST_TRACE(line << ":" << column);
    }

    bool is_public = true;

    virtual bool owns_type() {
        return m_type != nullptr;
    }

    virtual Type *type() {
        if (m_type) {
            return m_type.get();
        }
        return name->type();
    }

   public:
    std::unique_ptr<Identifier> name;
    std::unique_ptr<Type> m_type;
};

class TypeDef : public Declaration {
   public:
    TypeDef(uint32_t line, uint32_t column, std::unique_ptr<Identifier> id)
        : Declaration(line, column, std::move(id)), AST(line, column) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    AST_VISIT_METHODS();
};

class VariableDeclaration : public Declaration {
   public:
    VariableDeclaration(uint32_t line, uint32_t column,
                        std::unique_ptr<Identifier> name,
                        std::unique_ptr<AST> value)
        : Declaration(line, column, std::move(name)),
          AST(line, column),
          value(std::move(value)) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    AST_VISIT_METHODS()

   public:
    std::unique_ptr<AST> value;
    bool global = false;
};

class ParameterDeclaration : public Declaration {
   public:
    ParameterDeclaration(uint32_t line, uint32_t column,
                         std::unique_ptr<Identifier> name)
        : Declaration(line, column, std::move(name)), AST(line, column) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    ParameterDeclaration(uint32_t line, uint32_t column,
                         std::unique_ptr<Type> type)
        : Declaration(line, column, std::move(type)), AST(line, column) {
        AST_TRACE(line << ":" << column);
    }

    AST_VISIT_METHODS()
};

class ArrayInitializationList : public AST {
   public:
    ArrayInitializationList(uint32_t line, uint32_t column)
        : AST(line, column), values() {
        AST_TRACE(line << ":" << column);
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
        AST_TRACE(line << ":" << column << " " << this->name->name);
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
                       std::unique_ptr<Identifier> fn,
                       std::unique_ptr<Block> body)
        : Declaration(line, column, std::move(fn)),
          AST(line, column),
          body(std::move(body)) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    AST_VISIT_METHODS()

   public:
    std::unique_ptr<Block> body;
};

class FunctionDeclaration : public Declaration {
   public:
    FunctionDeclaration(uint32_t line, uint32_t column,
                        std::unique_ptr<Identifier> fn)
        : Declaration(line, column, std::move(fn)), AST(line, column) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    AST_VISIT_METHODS()
};

class Program : public AST {
   public:
    Program(uint32_t line, uint32_t column)
        : AST(line, column), declarations() {
        AST_TRACE(line << ":" << column);
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
        AST_TRACE(line << ":" << column << " " << op_to_str());
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
        AST_TRACE(line << ":" << column << " " << op_to_str());
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
        AST_TRACE(line << ":" << column);
    }

    AST_VISIT_METHODS()

   public:
    std::unique_ptr<AST> value;
};

class SizeOf : public AST {
   public:
    SizeOf(uint32_t line, uint32_t column, std::unique_ptr<Type> type)
        : AST(line, column), type(std::move(type)) {
        AST_TRACE(line << ":" << column);
    }

    AST_VISIT_METHODS()

   public:
    std::unique_ptr<Type> type;
};

class StructType : public Type, public Declaration {
   public:
    StructType(uint32_t line, uint32_t column, std::unique_ptr<Identifier> name,
               bool definition)
        : Type(line, column),
          AST(line, column),
          Declaration(line, column, std::move(name)),
          definition(definition) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    void add_member(std::unique_ptr<VariableDeclaration> member) {
        members.push_back(std::move(member));
    }

    AST_VISIT_METHODS()

    virtual bool owns_type() override {
        return false;
    }

    virtual Type *type() override {
        return this;
    }

   public:
    bool definition;
    std::vector<std::unique_ptr<VariableDeclaration>> members;
};

class UnionType : public Type, public Declaration {
   public:
    UnionType(uint32_t line, uint32_t column, std::unique_ptr<Identifier> name,
              bool definition)
        : Type(line, column),
          AST(line, column),
          Declaration(line, column, std::move(name)),
          definition(definition) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    void add_member(std::unique_ptr<VariableDeclaration> member) {
        members.push_back(std::move(member));
    }

    AST_VISIT_METHODS()

    virtual bool owns_type() override {
        return false;
    }

    virtual Type *type() override {
        return this;
    }

   public:
    bool definition;
    std::vector<std::unique_ptr<VariableDeclaration>> members;
};

#undef AST_VISIT_METHODS
}  // namespace CCOMP::AST
