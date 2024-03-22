#pragma once

#include <stdint.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "common.hpp"
#include "visitors/ASTVisitor.hpp"

/* #define DO_AST_TRACE */

#ifdef DO_AST_TRACE
#define AST_TRACE(p)                                              \
    do {                                                          \
        printf("Visiting %s at %d:%d (", __func__, line, column); \
        std::cout << p;                                           \
        printf(")\n");                                            \
    } while (0)
#else
#define AST_TRACE(p)
#endif

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
    virtual std::unique_ptr<AST> clone() = 0;

   public:
    uint32_t line, column;
};

#define AST_METHODS()                                        \
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

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto block = std::make_unique<Block>(line, column);
        for (auto &stmt : statements) {
            block->add_statement(stmt->clone());
        }
        return block;
    }

   public:
    std::vector<std::unique_ptr<AST>> statements;
};

class SwitchBlock : public AST {
   public:
    SwitchBlock(uint32_t line, uint32_t column)
        : AST(line, column), statements() {
        AST_TRACE(line << ":" << column);
    }

    void add_statement(std::unique_ptr<AST> statement) {
        statements.push_back(std::move(statement));
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto block = std::make_unique<SwitchBlock>(line, column);
        for (auto &stmt : statements) {
            block->add_statement(stmt->clone());
        }
        block->is_default = is_default;
        if (label) {
            block->label = label->clone();
        }
        return block;
    }

   public:
    std::vector<std::unique_ptr<AST>> statements;
    bool is_default = false;
    bool break_after = false;
    std::unique_ptr<AST> label;
};

class Switch : public AST {
   public:
    Switch(uint32_t line, uint32_t column, std::unique_ptr<AST> condition)
        : AST(line, column), condition(std::move(condition)), switch_blocks() {
        AST_TRACE(line << ":" << column);
    }

    void add_switch_block(std::unique_ptr<SwitchBlock> block) {
        switch_blocks.push_back(std::move(block));
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto sw = std::make_unique<Switch>(line, column, condition->clone());
        for (auto &block : switch_blocks) {
            auto b = block->clone().release();
            sw->add_switch_block(
                std::unique_ptr<SwitchBlock>(dynamic_cast<SwitchBlock *>(b)));
        }
        return sw;
    }

   public:
    std::unique_ptr<AST> condition;
    std::vector<std::unique_ptr<SwitchBlock>> switch_blocks;
};

class If : public AST {
   public:
    If(uint32_t line, uint32_t column, std::unique_ptr<AST> condition,
       std::unique_ptr<AST> then_block)
        : AST(line, column),
          condition(std::move(condition)),
          then_block(std::move(then_block)),
          else_block(nullptr) {
        AST_TRACE(line << ":" << column);
    }

    If(uint32_t line, uint32_t column, std::unique_ptr<AST> condition,
       std::unique_ptr<AST> then_block, std::unique_ptr<AST> else_block)
        : AST(line, column),
          condition(std::move(condition)),
          then_block(std::move(then_block)),
          else_block(std::move(else_block)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto if_node = std::make_unique<If>(line, column, condition->clone(),
                                            then_block->clone());
        if (else_block) {
            if_node->else_block = else_block->clone();
        }
        return if_node;
    }

   public:
    std::unique_ptr<AST> condition;
    std::unique_ptr<AST> then_block, else_block;
};

class For : public AST {
   public:
    For(uint32_t line, uint32_t column, std::unique_ptr<AST> body)
        : AST(line, column), body(std::move(body)) {
        AST_TRACE(line << ":" << column);
    }

    void set_init(std::unique_ptr<AST> init) {
        this->init = std::move(init);
    }

    void set_increment(std::unique_ptr<AST> increment) {
        this->increment = std::move(increment);
    }

    void set_condition(std::unique_ptr<AST> condition) {
        this->condition = std::move(condition);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto for_node = std::make_unique<For>(line, column, body->clone());
        if (init) {
            for_node->init = init->clone();
        }
        if (increment) {
            for_node->increment = increment->clone();
        }
        if (condition) {
            for_node->condition = condition->clone();
        }
        return for_node;
    }

   public:
    std::unique_ptr<AST> init, increment, condition;
    std::unique_ptr<AST> body;
};

class While : public AST {
   public:
    While(uint32_t line, uint32_t column, std::unique_ptr<AST> condition,
          std::unique_ptr<AST> body)
        : AST(line, column),
          condition(std::move(condition)),
          body(std::move(body)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<While>(line, column, condition->clone(),
                                       body->clone());
    }

   public:
    std::unique_ptr<AST> condition, body;
};

class DoWhile : public AST {
   public:
    DoWhile(uint32_t line, uint32_t column, std::unique_ptr<AST> condition,
            std::unique_ptr<AST> body)
        : AST(line, column),
          condition(std::move(condition)),
          body(std::move(body)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<DoWhile>(line, column, condition->clone(),
                                         body->clone());
    }

   public:
    std::unique_ptr<AST> condition, body;
};

class Attribute : public AST {
   public:
    Attribute(uint32_t line, uint32_t column, std::string name)
        : AST(line, column), name(std::move(name)) {
        AST_TRACE(line << ":" << column << " " << name);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<Attribute>(line, column, name);
    }

   public:
    std::string name;
};

class Assembly : public AST {
   public:
    Assembly(uint32_t line, uint32_t column, std::vector<std::string> assembly)
        : AST(line, column), assembly(std::move(assembly)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<Assembly>(line, column, assembly);
    }

   public:
    std::vector<std::string> assembly;
};

class Constant : public AST {
   public:
    Constant(uint32_t line, uint32_t column, std::string value)
        : AST(line, column), value(std::move(value)) {
        AST_TRACE(line << ":" << column << " " << value);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<Constant>(line, column, value);
    }

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

    void add_array_dimension(std::unique_ptr<AST> dimension) {
        array_dimensions++;
        array_sizes.push_back(std::move(dimension));
    }

    void add_array_dimension() {
        array_dimensions++;
        array_sizes.push_back(nullptr);
    }

   public:
    int pointer_count = 0;
    bool is_const = false;
    bool is_restrict = false;
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

    AST_METHODS()

    void add_type(std::unique_ptr<Type> p_type) {
        this->type_owned = std::move(p_type);
    }

    void add_type(Type *p_type) {
        this->type_ref = p_type;
    }

    Type *type() {
        if (type_ref) {
            return type_ref;
        }
        return type_owned.get();
    }

    std::unique_ptr<AST> clone() override {
        auto id = std::make_unique<Identifier>(line, column, name);
        if (type_owned) {
            auto type = type_owned->clone().release();
            id->add_type(std::unique_ptr<Type>(dynamic_cast<Type *>(type)));
        } else if (type_ref) {
            id->add_type(type_ref);
        }
        return id;
    }

   public:
    std::string name;

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

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<NamedType>(
            std::make_unique<Identifier>(line, column, name));
    }

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

    AST_METHODS()

    void add_parameter(std::unique_ptr<Identifier> parameter) {
        parameters.push_back(std::move(parameter));
    }

    std::unique_ptr<AST> clone() override {
        auto return_type = this->return_type->clone().release();
        auto fn_type = std::make_unique<FunctionType>(
            line, column,
            std::unique_ptr<Type>(dynamic_cast<Type *>(return_type)));
        for (auto &param : parameters) {
            fn_type->add_parameter(std::make_unique<Identifier>(
                param->line, param->column, param->name));
        }
        return fn_type;
    }

   public:
    bool varargs = false;
    std::unique_ptr<Type> return_type;
    std::vector<std::unique_ptr<Identifier>> parameters;
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
        FLOAT,
        DOUBLE,
    };

    PrimitiveType(uint32_t line, uint32_t column)
        : Type(line, column), AST(line, column) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

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
                case KeyWords::FLOAT:
                    s << "float ";
                    break;
                case KeyWords::DOUBLE:
                    s << "double ";
                    break;
            }
        }
        return s.str();
    }

    std::unique_ptr<AST> clone() override {
        auto prim = std::make_unique<PrimitiveType>(line, column);
        for (auto k : keywords) {
            prim->add_keyword(k);
        }
        return prim;
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

    void add_attribute(std::vector<std::unique_ptr<Attribute>> &attribute) {
        for (auto &attr : attribute) {
            attributes.push_back(std::move(attr));
        }
    }

    void add_assembly(std::unique_ptr<Assembly> line) {
        assembly.push_back(std::move(line));
    }

   public:
    std::unique_ptr<Identifier> name;
    std::unique_ptr<Type> m_type;
    std::vector<std::unique_ptr<Attribute>> attributes;
    std::vector<std::unique_ptr<Assembly>> assembly;
};

class TypeDef : public Declaration {
   public:
    TypeDef(uint32_t line, uint32_t column, std::unique_ptr<Identifier> id)
        : Declaration(line, column, std::move(id)), AST(line, column) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    AST_METHODS();

    std::unique_ptr<AST> clone() override {
        return std::make_unique<TypeDef>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name));
    }
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

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto var = std::make_unique<VariableDeclaration>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name),
            nullptr);
        if (value) {
            var->value = value->clone();
        }
        return var;
    }

   public:
    std::unique_ptr<AST> value;
    bool global = false;
};

class ArrayInitializationList : public AST {
   public:
    ArrayInitializationList(uint32_t line, uint32_t column)
        : AST(line, column), values() {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    void add_value(std::unique_ptr<AST> value) {
        values.push_back(std::move(value));
    }

    std::unique_ptr<AST> clone() override {
        auto list = std::make_unique<ArrayInitializationList>(line, column);
        for (auto &val : values) {
            list->add_value(val->clone());
        }
        return list;
    }

   public:
    std::vector<std::unique_ptr<AST>> values;
};

class ArrayAccess : public AST {
   public:
    ArrayAccess(uint32_t line, uint32_t column, std::unique_ptr<AST> array)
        : AST(line, column), array(std::move(array)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    void add_index(std::unique_ptr<AST> index) {
        indices.push_back(std::move(index));
    }

    std::unique_ptr<AST> clone() override {
        auto access =
            std::make_unique<ArrayAccess>(line, column, array->clone());
        for (auto &idx : indices) {
            access->add_index(idx->clone());
        }
        return access;
    }

   public:
    std::unique_ptr<AST> array;
    std::vector<std::unique_ptr<AST>> indices;
};

class StructAccess : public AST {
   public:
    StructAccess(uint32_t line, uint32_t column, std::unique_ptr<AST> struc,
                 std::unique_ptr<Identifier> member, bool through_pointer)
        : AST(line, column),
          struc(std::move(struc)),
          member(std::move(member)),
          through_pointer(through_pointer) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<StructAccess>(
            line, column, struc->clone(),
            std::make_unique<Identifier>(member->line, member->column,
                                         member->name),
            through_pointer);
    }

   public:
    bool through_pointer;
    std::unique_ptr<AST> struc;
    std::unique_ptr<Identifier> member;
};

class Assignment : public AST {
   public:
    Assignment(uint32_t line, uint32_t column, std::unique_ptr<AST> left,
               std::unique_ptr<AST> right)
        : AST(line, column), left(std::move(left)), right(std::move(right)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<Assignment>(line, column, left->clone(),
                                            right->clone());
    }

   public:
    std::unique_ptr<AST> left, right;
};

class OperationAssignment : public AST {
   public:
    enum class Operator {
        NONE = 0,
        PLUS,
        MINUS,
        MUL,
        DIV,
        REM,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        SHIFT_LEFT,
        SHIFT_RIGHT,
    };

    OperationAssignment(uint32_t line, uint32_t column,
                        std::unique_ptr<AST> left, std::unique_ptr<AST> right,
                        Operator op)
        : AST(line, column),
          left(std::move(left)),
          right(std::move(right)),
          op(op) {
        AST_TRACE(line << ":" << column << " " << op_to_str());
    }

    AST_METHODS()

    std::string op_to_str() const {
        switch (op) {
            case Operator::PLUS:
                return "+=";
            case Operator::MINUS:
                return "-=";
            case Operator::MUL:
                return "*=";
            case Operator::DIV:
                return "/=";
            case Operator::REM:
                return "%=";
            case Operator::BITWISE_AND:
                return "&=";
            case Operator::BITWISE_OR:
                return "|=";
            case Operator::BITWISE_XOR:
                return "^=";
            case Operator::SHIFT_LEFT:
                return "<<=";
            case Operator::SHIFT_RIGHT:
                return ">>=";
            default:
                die("Invalid operator");
                return "";
        }
    }

    static OperationAssignment::Operator str_to_op(const std::string &s) {
        if (s == "+=") {
            return Operator::PLUS;
        } else if (s == "-=") {
            return Operator::MINUS;
        } else if (s == "*=") {
            return Operator::MUL;
        } else if (s == "/=") {
            return Operator::DIV;
        } else if (s == "%=") {
            return Operator::REM;
        } else if (s == "&=") {
            return Operator::BITWISE_AND;
        } else if (s == "|=") {
            return Operator::BITWISE_OR;
        } else if (s == "^=") {
            return Operator::BITWISE_XOR;
        } else if (s == "<<=") {
            return Operator::SHIFT_LEFT;
        } else if (s == ">>=") {
            return Operator::SHIFT_RIGHT;
        }
        die("Invalid operator: %s", s.c_str());
        return Operator::NONE;
    }

    std::unique_ptr<AST> clone() override {
        return std::make_unique<OperationAssignment>(
            line, column, left->clone(), right->clone(), op);
    }

   public:
    std::unique_ptr<AST> left, right;
    Operator op;
};

class ExpressionList : public AST {
   public:
    ExpressionList(uint32_t line, uint32_t column)
        : AST(line, column), expressions() {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    void add_expression(std::unique_ptr<AST> expression) {
        expressions.push_back(std::move(expression));
    }

    std::unique_ptr<AST> clone() override {
        auto list = std::make_unique<ExpressionList>(line, column);
        for (auto &expr : expressions) {
            list->add_expression(expr->clone());
        }
        return list;
    }

   public:
    std::vector<std::unique_ptr<AST>> expressions;
};

class FunctionCall : public AST {
   public:
    FunctionCall(uint32_t line, uint32_t column,
                 std::unique_ptr<Identifier> name)
        : AST(line, column), name(std::move(name)), arguments() {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto call = std::make_unique<FunctionCall>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name));
        for (auto &arg : arguments) {
            call->add_argument(arg->clone());
        }
        return call;
    }

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

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto block = body->clone().release();
        return std::make_unique<FunctionDefinition>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name),
            std::unique_ptr<Block>(dynamic_cast<Block *>(block)));
    }

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

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<FunctionDeclaration>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name));
    }
};

class Program : public AST {
   public:
    Program(uint32_t line, uint32_t column)
        : AST(line, column), declarations() {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    void add_declaration(std::unique_ptr<AST> declaration) {
        declarations.push_back(std::move(declaration));
    }

    std::unique_ptr<AST> clone() override {
        auto program = std::make_unique<Program>(line, column);
        for (auto &decl : declarations) {
            program->add_declaration(decl->clone());
        }
        return program;
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
        PLUS,
        MINUS,
        LOGICAL_NOT,
        BITWISE_NOT,
        SIZEOF,
        INC_POSTFIX,
        INC_PREFIX,
        DEC_POSTFIX,
        DEC_PREFIX,
    };

    UnaryExpression(uint32_t line, uint32_t column, std::unique_ptr<AST> value,
                    Operator op)
        : AST(line, column), value(std::move(value)), op(op) {
        AST_TRACE(line << ":" << column << " " << op_to_str());
    }

    AST_METHODS()

    std::string op_to_str() {
        switch (op) {
            case Operator::DEREFERENCE:
                return "deref";
            case Operator::ADDRESS:
                return "addr of";
            case Operator::INC_POSTFIX:
                return "inc postfix";
            case Operator::INC_PREFIX:
                return "inc prefix";
            case Operator::DEC_POSTFIX:
                return "dec postfix";
            case Operator::DEC_PREFIX:
                return "dec prefix";
            case Operator::PLUS:
                return "+";
            case Operator::MINUS:
                return "-";
            case Operator::LOGICAL_NOT:
                return "!";
            case Operator::BITWISE_NOT:
                return "~";
            case Operator::SIZEOF:
                return "sizeof";
            default:
                die("Invalid operator");
                return "";
        }
    }

    static inline Operator postfix_str_to_op(const std::string &s) {
        if (s == "++") {
            return Operator::INC_POSTFIX;
        } else if (s == "--") {
            return Operator::DEC_POSTFIX;
        }
        die("Invalid operator: %s", s.c_str());
        return Operator::NONE;
    }

    static inline Operator prefix_str_to_op(const std::string &s) {
        if (s == "++") {
            return Operator::INC_PREFIX;
        } else if (s == "--") {
            return Operator::DEC_PREFIX;
        } else if (s == "sizeof") {
            return Operator::SIZEOF;
        }
        switch (s[0]) {
            case '*':
                return Operator::DEREFERENCE;
            case '&':
                return Operator::ADDRESS;
            case '+':
                return Operator::PLUS;
            case '-':
                return Operator::MINUS;
            case '!':
                return Operator::LOGICAL_NOT;
            case '~':
                return Operator::BITWISE_NOT;
            default:
                die("Invalid operator: %s", s.c_str());
                return Operator::NONE;
        }
    }

    std::unique_ptr<AST> clone() override {
        return std::make_unique<UnaryExpression>(line, column, value->clone(),
                                                 op);
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
        LESS,
        GREATER,
        LESS_EQUAL,
        GREATER_EQUAL,
        EQUAL,
        BITWISE_AND,
        BITWISE_OR,
        BITWISE_XOR,
        LOGICAL_AND,
        LOGICAL_OR,
        NOT_EQUAL,
        SHIFT_LEFT,
        SHIFT_RIGHT,
    };

    BinaryExpression(uint32_t line, uint32_t column, std::unique_ptr<AST> left,
                     std::unique_ptr<AST> right, Operator op)
        : AST(line, column),
          left(std::move(left)),
          right(std::move(right)),
          op(op) {
        AST_TRACE(line << ":" << column << " " << op_to_str());
    }

    AST_METHODS()

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
            case Operator::SHIFT_LEFT:
                return "<<";
            case Operator::SHIFT_RIGHT:
                return ">>";
            case Operator::LESS:
                return "<";
            case Operator::GREATER:
                return ">";
            case Operator::LESS_EQUAL:
                return "<=";
            case Operator::GREATER_EQUAL:
                return ">=";
            case Operator::EQUAL:
                return "==";
            case Operator::NOT_EQUAL:
                return "!=";
            case Operator::BITWISE_AND:
                return "&";
            case Operator::BITWISE_OR:
                return "|";
            case Operator::BITWISE_XOR:
                return "^";
            case Operator::LOGICAL_AND:
                return "&&";
            case Operator::LOGICAL_OR:
                return "||";
            default:
                die("Invalid operator");
                return "";
        }
    }

    static inline Operator str_to_op(const std::string &s) {
        if (s == "<<") {
            return Operator::SHIFT_LEFT;
        } else if (s == ">>") {
            return Operator::SHIFT_RIGHT;
        } else if (s == "<=") {
            return Operator::LESS_EQUAL;
        } else if (s == ">=") {
            return Operator::GREATER_EQUAL;
        } else if (s == "==") {
            return Operator::EQUAL;
        } else if (s == "!=") {
            return Operator::NOT_EQUAL;
        } else if (s == "&&") {
            return Operator::LOGICAL_AND;
        } else if (s == "||") {
            return Operator::LOGICAL_OR;
        }
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
            case '<':
                return Operator::LESS;
            case '>':
                return Operator::GREATER;
            case '&':
                return Operator::BITWISE_AND;
            case '|':
                return Operator::BITWISE_OR;
            case '^':
                return Operator::BITWISE_XOR;
            default:
                die("Invalid operator: %s", s.c_str());
                return Operator::NONE;
        }
    }

    std::unique_ptr<AST> clone() override {
        return std::make_unique<BinaryExpression>(line, column, left->clone(),
                                                  right->clone(), op);
    }

   public:
    std::unique_ptr<AST> left, right;
    Operator op;
};

class TernaryExpression : public AST {
   public:
    TernaryExpression(uint32_t line, uint32_t column,
                      std::unique_ptr<AST> condition,
                      std::unique_ptr<AST> then_expr,
                      std::unique_ptr<AST> else_expr)
        : AST(line, column),
          condition(std::move(condition)),
          then_expr(std::move(then_expr)),
          else_expr(std::move(else_expr)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        return std::make_unique<TernaryExpression>(
            line, column, condition->clone(), then_expr->clone(),
            else_expr->clone());
    }

   public:
    std::unique_ptr<AST> condition, then_expr, else_expr;
};

class Return : public AST {
   public:
    Return(uint32_t line, uint32_t column, std::unique_ptr<AST> value)
        : AST(line, column), value(std::move(value)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        if (value) {
            return std::make_unique<Return>(line, column, value->clone());
        }
        return std::make_unique<Return>(line, column, nullptr);
    }

   public:
    std::unique_ptr<AST> value;
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

    AST_METHODS()

    virtual bool owns_type() override {
        return false;
    }

    virtual Type *type() override {
        return this;
    }

    std::unique_ptr<AST> clone() override {
        auto struc = std::make_unique<StructType>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name),
            definition);
        for (auto &member : members) {
            struc->add_member(std::make_unique<VariableDeclaration>(
                member->line, member->column,
                std::make_unique<Identifier>(member->name->line,
                                             member->name->column,
                                             member->name->name),
                nullptr));
        }
        return struc;
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

    AST_METHODS()

    virtual bool owns_type() override {
        return false;
    }

    virtual Type *type() override {
        return this;
    }

    std::unique_ptr<AST> clone() override {
        auto union_type = std::make_unique<UnionType>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name),
            definition);
        for (auto &member : members) {
            union_type->add_member(std::make_unique<VariableDeclaration>(
                member->line, member->column,
                std::make_unique<Identifier>(member->name->line,
                                             member->name->column,
                                             member->name->name),
                nullptr));
        }
        return union_type;
    }

   public:
    bool definition;
    std::vector<std::unique_ptr<VariableDeclaration>> members;
};

class EnumValue : public AST {
   public:
    EnumValue(uint32_t line, uint32_t column, std::unique_ptr<Identifier> name)
        : AST(line, column), name(std::move(name)), value(nullptr) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto id =
            std::make_unique<Identifier>(name->line, name->column, name->name);
        auto e = std::make_unique<EnumValue>(line, column, std::move(id));
        if (value) {
            e->value = value->clone();
        }
        return e;
    }

    void set_value(std::unique_ptr<AST> value) {
        this->value = std::move(value);
    }

   public:
    std::unique_ptr<Identifier> name;
    std::unique_ptr<AST> value;
};

class EnumType : public Type, public Declaration {
   public:
    EnumType(uint32_t line, uint32_t column, std::unique_ptr<Identifier> name,
             bool definition)
        : Type(line, column),
          AST(line, column),
          Declaration(line, column, std::move(name)),
          definition(definition) {
        AST_TRACE(line << ":" << column << " " << this->name->name);
    }

    void add_value(std::unique_ptr<EnumValue> value) {
        values.push_back(std::move(value));
    }

    AST_METHODS()

    virtual bool owns_type() override {
        return false;
    }

    virtual Type *type() override {
        return this;
    }

    std::unique_ptr<AST> clone() override {
        auto enum_type = std::make_unique<EnumType>(
            line, column,
            std::make_unique<Identifier>(name->line, name->column, name->name),
            definition);
        for (auto &value : values) {
            auto val = value->clone().release();
            auto val_ptr =
                std::unique_ptr<EnumValue>(dynamic_cast<EnumValue *>(val));
            enum_type->add_value(std::move(val_ptr));
        }
        return enum_type;
    }

   public:
    std::vector<std::unique_ptr<EnumValue>> values;
    bool definition;
};

class TypeCast : public AST {
   public:
    TypeCast(uint32_t line, uint32_t column, std::unique_ptr<Type> type,
             std::unique_ptr<AST> value)
        : AST(line, column), type(std::move(type)), value(std::move(value)) {
        AST_TRACE(line << ":" << column);
    }

    AST_METHODS()

    std::unique_ptr<AST> clone() override {
        auto type = this->type->clone().release();
        return std::make_unique<TypeCast>(
            line, column, std::unique_ptr<Type>(dynamic_cast<Type *>(type)),
            value->clone());
    }

   public:
    std::unique_ptr<Type> type;
    std::unique_ptr<AST> value;
};

#undef AST_METHODS
}  // namespace CCOMP::AST
