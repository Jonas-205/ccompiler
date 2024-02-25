#pragma once

#include "visitors/ASTBaseVisitor.hpp"

namespace CCOMP::AST {

class PrintVisitor : public ASTBaseVisitor {
   public:
    void *visit(Program &node, void *args) override;
    void *visit(Block &node, void *args) override;
    void *visit(Constant &node, void *args) override;
    void *visit(Identifier &node, void *args) override;
    void *visit(Type &node, void *args) override;
    void *visit(VariableDeclaration &node, void *args) override;
    void *visit(ParameterDeclaration &node, void *args) override;
    void *visit(FunctionDeclaration &node, void *args) override;
    void *visit(FunctionCall &node, void *args) override;
    void *visit(BinaryExpression &node, void *args) override;
    void *visit(Return &node, void *args) override;
};
}  // namespace CCOMP::AST
