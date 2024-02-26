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
        return nullptr;
    }
    void *visit(PrimitiveType &node, void *args) override {
        return nullptr;
    }
    void *visit(VariableDeclaration &node, void *args) override {
        node.type->accept(*this, args);
        node.name->accept(*this, args);
        if (node.value) {
            node.value->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(ParameterDeclaration &node, void *args) override {
        node.type->accept(*this, args);
        if (node.name) {
            node.name->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(FunctionDefinition &node, void *args) override {
        node.type->accept(*this, args);
        node.name->accept(*this, args);
        for (auto &param : node.parameters) {
            param->accept(*this, args);
        }
        node.body->accept(*this, args);
        return nullptr;
    }
    void *visit(FunctionDeclaration &node, void *args) override {
        node.type->accept(*this, args);
        node.name->accept(*this, args);
        for (auto &param : node.parameters) {
            param->accept(*this, args);
        }
        return nullptr;
    }
    void *visit(FunctionCall &node, void *args) override {
        node.name->accept(*this, args);
        for (auto &arg : node.arguments) {
            arg->accept(*this, args);
        }
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
};

}  // namespace CCOMP::AST
