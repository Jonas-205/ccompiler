#pragma once

#include "ast.hpp"
#include "visitors/ASTVisitor.hpp"

namespace CCOMP::AST {

class ASTBaseVisitor : public ASTVisitor {
   public:
    void *visit(Program &node, void *args) override {
        for (auto &decl : node.declarations) {
            decl->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(Block &node, void *args) override {
        for (auto &stmt : node.statements) {
            stmt->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(Constant &node, void *args) override {
        return nullptr;
    }
    void *visit(Identifier &node, void *args) override {
        if (node.type()) {
            node.type()->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(PrimitiveType &node, void *args) override {
        for (auto &arr : node.array_sizes) {
            if (arr) {
                arr->accept(*this, args);
            }
        }
        return nullptr;
    }
    void *visit(VariableDeclaration &node, void *args) override {
        for (auto &ass : node.assembly) {
            ass->accept(*this, args);
        }
        for (auto &attr : node.attributes) {
            attr->accept(*this, args);
        }
        if (node.owns_type()) {
            node.type()->accept(*this, args);
        }
        node.name->accept(*this, args);
        if (node.value) {
            node.value->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(FunctionDefinition &node, void *args) override {
        for (auto &ass : node.assembly) {
            ass->accept(*this, args);
        }
        for (auto &attr : node.attributes) {
            attr->accept(*this, args);
        }
        if (node.owns_type()) {
            node.type()->accept(*this, args);
        }
        node.name->accept(*this, args);
        node.body->accept(*this, args);
        return nullptr;
    }
    void *visit(FunctionDeclaration &node, void *args) override {
        for (auto &ass : node.assembly) {
            ass->accept(*this, args);
        }
        for (auto &attr : node.attributes) {
            attr->accept(*this, args);
        }
        if (node.owns_type()) {
            node.type()->accept(*this, args);
        }
        node.name->accept(*this, args);
        return nullptr;
    }
    void *visit(FunctionCall &node, void *args) override {
        node.name->accept(*this, args);
        for (auto &arg : node.arguments) {
            arg->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(UnaryExpression &node, void *args) override {
        node.value->accept(*this, args);
        return nullptr;
    }
    void *visit(BinaryExpression &node, void *args) override {
        node.left->accept(*this, args);
        node.right->accept(*this, args);
        return nullptr;
    }
    void *visit(Return &node, void *args) override {
        if (node.value) {
            node.value->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(TypeDef &node, void *args) override {
        node.name->accept(*this, args);
        return nullptr;
    }
    void *visit(NamedType &node, void *args) override {
        for (auto &arr : node.array_sizes) {
            if (arr) {
                arr->accept(*this, args);
            }
        }
        return nullptr;
    }
    void *visit(ArrayInitializationList &node, void *args) override {
        for (auto &val : node.values) {
            val->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(FunctionType &node, void *args) override {
        node.return_type->accept(*this, args);
        for (auto &param : node.parameters) {
            param->accept(*this, args);
        }
        for (auto &arr : node.array_sizes) {
            if (arr) {
                arr->accept(*this, args);
            }
        }
        return nullptr;
    }
    void *visit(StructType &node, void *args) override {
        for (auto &ass : node.assembly) {
            ass->accept(*this, args);
        }
        for (auto &attr : node.attributes) {
            attr->accept(*this, args);
        }
        node.name->accept(*this, args);
        for (auto &member : node.members) {
            member->accept(*this, args);
        }
        for (auto &arr : node.array_sizes) {
            if (arr) {
                arr->accept(*this, args);
            }
        }
        return nullptr;
    }
    void *visit(EnumType &node, void *args) override {
        for (auto &ass : node.assembly) {
            ass->accept(*this, args);
        }
        for (auto &attr : node.attributes) {
            attr->accept(*this, args);
        }
        node.name->accept(*this, args);
        for (auto &member : node.values) {
            member->accept(*this, args);
        }
        for (auto &arr : node.array_sizes) {
            if (arr) {
                arr->accept(*this, args);
            }
        }
        return nullptr;
    }
    void *visit(EnumValue &node, void *args) override {
        node.name->accept(*this, args);
        if (node.value) {
            node.value->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(UnionType &node, void *args) override {
        for (auto &ass : node.assembly) {
            ass->accept(*this, args);
        }
        for (auto &attr : node.attributes) {
            attr->accept(*this, args);
        }
        node.name->accept(*this, args);
        for (auto &member : node.members) {
            member->accept(*this, args);
        }
        for (auto &arr : node.array_sizes) {
            if (arr) {
                arr->accept(*this, args);
            }
        }
        return nullptr;
    }
    void *visit(Attribute &node, void *args) override {
        return nullptr;
    }
    void *visit(Assembly &node, void *args) override {
        return nullptr;
    }
    void *visit(If &node, void *args) override {
        node.condition->accept(*this, args);
        node.then_block->accept(*this, args);
        if (node.else_block) {
            node.else_block->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(ArrayAccess &node, void *args) override {
        node.array->accept(*this, args);
        for (auto &idx : node.indices) {
            idx->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(StructAccess &node, void *args) override {
        node.struc->accept(*this, args);
        node.member->accept(*this, args);
        return nullptr;
    }
    void *visit(Assignment &node, void *args) override {
        node.left->accept(*this, args);
        node.right->accept(*this, args);
        return nullptr;
    }
    void *visit(For &node, void *args) override {
        if (node.init) {
            node.init->accept(*this, args);
        }
        if (node.condition) {
            node.condition->accept(*this, args);
        }
        if (node.increment) {
            node.increment->accept(*this, args);
        }
        node.body->accept(*this, args);
        return nullptr;
    }
    void *visit(TypeCast &node, void *args) override {
        node.type->accept(*this, args);
        node.value->accept(*this, args);
        return nullptr;
    }
    void *visit(TernaryExpression &node, void *args) override {
        node.condition->accept(*this, args);
        node.then_expr->accept(*this, args);
        node.else_expr->accept(*this, args);
        return nullptr;
    }
    void *visit(OperationAssignment &node, void *args) override {
        node.left->accept(*this, args);
        node.right->accept(*this, args);
        return nullptr;
    }
    void *visit(ExpressionList &node, void *args) override {
        for (auto &expr : node.expressions) {
            expr->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(While &node, void *args) override {
        node.condition->accept(*this, args);
        node.body->accept(*this, args);
        return nullptr;
    }
    void *visit(DoWhile &node, void *args) override {
        node.body->accept(*this, args);
        node.condition->accept(*this, args);
        return nullptr;
    }
    void *visit(Switch &node, void *args) override {
        node.condition->accept(*this, args);
        for (auto &block : node.switch_blocks) {
            block->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(SwitchBlock &node, void *args) override {
        if (!node.is_default) {
            node.label->accept(*this, args);
        }
        for (auto &s : node.statements) {
            s->accept(*this, args);
        }
        return nullptr;
    }
};

}  // namespace CCOMP::AST
