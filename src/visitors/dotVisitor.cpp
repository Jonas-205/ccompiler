#include "visitors/dotVisitor.hpp"

#include <fstream>
#include <stack>

namespace CCOMP::AST {

static std::ofstream file;

static std::stack<int> node_stack;
static int node_counter = 0;

static void declare_node(int id, const std::string &name) {
    file << "  node_" << id << " [label=\"" << name << "\"];\n";
}

static void connect_nodes(int a, int b) {
    file << "  node_" << a << " -> node_" << b << ";\n";
}

void DotVisitor::generate(Program &node, const std::string &output_file) {
    std::ofstream l_file(output_file);
    file = std::move(l_file);

    file << "digraph G {\n";
    file << "  graph [ordering=\"out\"];\n";

    DotVisitor visitor;

    node.accept(visitor, nullptr);

    file << "}";
    file.close();
}

void *DotVisitor::visit(Program &node, void *args) {
    int id = node_counter++;
    declare_node(id, "Program");

    node_stack.push(id);
    return ASTBaseVisitor::visit(node, args);
}

#define GENERATE(name) \
    int id = node_counter++; \
    declare_node(id, name); \
    int parent = node_stack.top(); \
    connect_nodes(parent, id); \
    node_stack.push(id); \
    ASTBaseVisitor::visit(node, args); \
    node_stack.pop(); \
    return nullptr;


void *DotVisitor::visit(FunctionDeclaration &node, void *args) {
    GENERATE("Function");
}

void *DotVisitor::visit(Block &node, void *args) {
    GENERATE("Block");
}

void *DotVisitor::visit(Constant &node, void *args) {
    GENERATE(node.value);
}

void *DotVisitor::visit(Identifier &node, void *args) {
    GENERATE(node.name);
}

void *DotVisitor::visit(Type &node, void *args) {
    GENERATE(node.name());
}

void *DotVisitor::visit(VariableDeclaration &node, void *args) {
    GENERATE("Variable");
}

void *DotVisitor::visit(ParameterDeclaration &node, void *args) {
    GENERATE("Parameter");
}

void *DotVisitor::visit(FunctionCall &node, void *args) {
    GENERATE("FunctionCall");
}

void *DotVisitor::visit(BinaryExpression &node, void *args) {
    GENERATE(node.op_to_str());
}

void *DotVisitor::visit(Return &node, void *args) {
    GENERATE("Return");
}

}  // namespace CCOMP::AST
