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
    void *visit(FunctionDefinition &node, void *args) override;
    void *visit(FunctionDeclaration &node, void *args) override;
    void *visit(FunctionCall &node, void *args) override;
    void *visit(UnaryExpression &node, void *args) override;
    void *visit(BinaryExpression &node, void *args) override;
    void *visit(Return &node, void *args) override;
    void *visit(TypeDef &node, void *args) override;
    void *visit(NamedType &node, void *args) override;
    void *visit(ArrayInitializationList &node, void *args) override;
    void *visit(FunctionType &node, void *args) override;
    void *visit(StructType &node, void *args) override;
    void *visit(UnionType &node, void *args) override;
    void *visit(Attribute &node, void *args) override;
    void *visit(Assembly &node, void *args) override;
    void *visit(If &node, void *args) override;
    void *visit(ArrayAccess &node, void *args) override;
    void *visit(StructAccess &node, void *args) override;
    void *visit(Assignment &node, void *args) override;
    void *visit(For &node, void *args) override;
    void *visit(TypeCast &node, void *args) override;
    void *visit(TernaryExpression &node, void *args) override;
    void *visit(OperationAssignment &node, void *args) override;
    void *visit(ExpressionList &node, void *args) override;
    void *visit(EnumType &node, void *args) override;
    void *visit(EnumValue &node, void *args) override;
    void *visit(While &node, void *args) override;
    void *visit(DoWhile &node, void *args) override;
    void *visit(Switch &node, void *args) override;
    void *visit(SwitchBlock &node, void *args) override;
};
}  // namespace CCOMP::AST
