#include "visitors/dotVisitor.hpp"

#include <fstream>
#include <stack>

namespace CCOMP::AST {

static std::ofstream file;

static std::stack<int> node_stack;
static int node_counter = 0;

static void declare_node(int id, const std::string &name) {
    file << "  node_" << id << " [label=\"" << name << "\"];\n";
    file.flush();
}

static void connect_nodes(int a, int b) {
    file << "  node_" << a << " -> node_" << b << ";\n";
    file.flush();
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
    declare_node(id, node.file_location);

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
    GENERATE("Function Decl");
}

void *DotVisitor::visit(FunctionDefinition &node, void *args) {
    GENERATE("Function Def");
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

void *DotVisitor::visit(PrimitiveType &node, void *args) {
    std::string arr = "";
    for (int i = 0; i < node.array_dimensions; i++) {
        arr += "[]";
    }
    GENERATE(node.to_string() + (node.is_pointer ? "*" : "") + arr);
}

void *DotVisitor::visit(NamedType &node, void *args) {
    std::string arr = "";
    for (int i = 0; i < node.array_dimensions; i++) {
        arr += "[]";
    }
    GENERATE(node.name + (node.is_pointer ? "*" : "") + arr);
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

void *DotVisitor::visit(UnaryExpression &node, void *args) {
    GENERATE(node.op_to_str());
}

void *DotVisitor::visit(BinaryExpression &node, void *args) {
    GENERATE(node.op_to_str());
}

void *DotVisitor::visit(Return &node, void *args) {
    GENERATE("Return");
}

void *DotVisitor::visit(TypeDef &node, void *args) {
    GENERATE("TypeDef");
}

void *DotVisitor::visit(StructDefinition &node, void *args) {
    std::string arr = "";
    for (int i = 0; i < node.array_dimensions; i++) {
        arr += "[]";
    }
    GENERATE("Struct Def " + arr);
};

void *DotVisitor::visit(StructDeclaration &node, void *args) {
    std::string arr = "";
    for (int i = 0; i < node.array_dimensions; i++) {
        arr += "[]";
    }
    GENERATE("Struct Decl " + arr);
};

void *DotVisitor::visit(ArrayInitializationList &node, void *args) {
    GENERATE("Array Init");
};

void *DotVisitor::visit(UnionDefinition &node, void *args) {
    std::string arr = "";
    for (int i = 0; i < node.array_dimensions; i++) {
        arr += "[]";
    }
    GENERATE("Union Def " + arr);
};

void *DotVisitor::visit(UnionDeclaration &node, void *args) {
    std::string arr = "";
    for (int i = 0; i < node.array_dimensions; i++) {
        arr += "[]";
    }
    GENERATE("Union Decl " + arr);
};

void *DotVisitor::visit(SizeOf &node, void *args) {
    GENERATE("SizeOf");
};

}  // namespace CCOMP::AST
