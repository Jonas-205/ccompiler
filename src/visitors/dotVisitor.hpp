#pragma once

#include "visitors/ASTBaseVisitor.hpp"

namespace CCOMP::AST {

class DotVisitor : public ASTBaseVisitor {
   public:
    static void generate(Program &node, const std::string &output_file);

    void *visit(Program &node, void *args) override;
    void *visit(Block &node, void *args) override;
    void *visit(Constant &node, void *args) override;
    void *visit(Identifier &node, void *args) override;
    void *visit(PrimitiveType &node, void *args) override;
    void *visit(VariableDeclaration &node, void *args) override;
    void *visit(ParameterDeclaration &node, void *args) override;
    void *visit(FunctionDefinition &node, void *args) override;
    void *visit(FunctionDeclaration &node, void *args) override;
    void *visit(FunctionCall &node, void *args) override;
    void *visit(BinaryExpression &node, void *args) override;
    void *visit(Return &node, void *args) override;
};
}  // namespace CCOMP::AST
