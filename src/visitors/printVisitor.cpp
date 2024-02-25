#include "visitors/printVisitor.hpp"

namespace CCOMP::AST {

static int S_INDENT;

#define INDENT() for (int i = 0; i < S_INDENT; i++) printf("  ")

void *PrintVisitor::visit(Program &node, void *args) {
    S_INDENT = 0;
    info("Program: %s", node.file_location.c_str());

    return ASTBaseVisitor::visit(node, args);
}
void *PrintVisitor::visit(Block &node, void *args) {
    INDENT();
    printf("Block:\n");

    S_INDENT++;
    ASTBaseVisitor::visit(node, args);

    S_INDENT--;

    return nullptr;
}
void *PrintVisitor::visit(Constant &node, void *args) {
    INDENT();
    printf("Constant: %s\n", node.value.c_str());
    return ASTBaseVisitor::visit(node, args);
}
void *PrintVisitor::visit(Identifier &node, void *args) {
    INDENT();
    printf("Identifier: %s\n", node.name.c_str());

    return ASTBaseVisitor::visit(node, args);
}
void *PrintVisitor::visit(Type &node, void *args) {
    INDENT();
    printf("Type: %s\n", node.name().c_str());
    return ASTBaseVisitor::visit(node, args);
}
void *PrintVisitor::visit(VariableDeclaration &node, void *args) {
    INDENT();
    printf("VariableDeclaration:\n");

    S_INDENT++;

    INDENT();
    printf("Visibility: %s\n", node.is_public ? "public" : "private");

    ASTBaseVisitor::visit(node, args);
    S_INDENT--;

    return nullptr;
}
void *PrintVisitor::visit(ParameterDeclaration &node, void *args) {
    INDENT();
    printf("ParameterDeclaration:\n");

    S_INDENT++;
    ASTBaseVisitor::visit(node, args);
    S_INDENT--;

    return nullptr;
}
void *PrintVisitor::visit(FunctionDeclaration &node, void *args) {
    INDENT();
    printf("FunctionDeclaration:\n");

    S_INDENT++;
    INDENT();
    printf("Visibility: %s\n", node.is_public ? "public" : "private");


    ASTBaseVisitor::visit(node, args);
    S_INDENT--;

    return nullptr;
}
void *PrintVisitor::visit(FunctionCall &node, void *args) {
    INDENT();
    printf("FunctionCall: \n");

    S_INDENT++;
    ASTBaseVisitor::visit(node, args);
    S_INDENT--;

    return nullptr;
}
void *PrintVisitor::visit(BinaryExpression &node, void *args) {
    INDENT();
    printf("BinaryExpression: %s\n", node.op_to_str().c_str());

    S_INDENT++;
    ASTBaseVisitor::visit(node, args);
    S_INDENT--;

    return nullptr;
}
void *PrintVisitor::visit(Return &node, void *args) {
    INDENT();
    printf("Return:\n");

    S_INDENT++;
    ASTBaseVisitor::visit(node, args);
    S_INDENT--;

    return nullptr;
}

}  // namespace CCOMP::AST
