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

#define GENERATE(name)                 \
    int id = node_counter++;           \
    declare_node(id, name);            \
    int parent = node_stack.top();     \
    connect_nodes(parent, id);         \
    node_stack.push(id);               \
    ASTBaseVisitor::visit(node, args); \
    node_stack.pop();                  \
    return nullptr;

#define GENERATE_TYPE(name)                                \
    std::string arr = "";                                  \
    for (int i = 0; i < node.array_dimensions; i++) {      \
        arr += "[]";                                       \
    }                                                      \
    int id = node_counter++;                               \
    std::string r = (node.is_restrict ? "restrict " : ""); \
    std::string c = (node.is_const ? "const " : "");       \
    std::string p = "";                                    \
    for (int i = 0; i < node.pointer_count; i++) {         \
        p += "*";                                          \
    }                                                      \
    declare_node(id, c + r + std::string(name) + p + arr); \
    int parent = node_stack.top();                         \
    connect_nodes(parent, id);                             \
    node_stack.push(id);                                   \
    ASTBaseVisitor::visit(node, args);                     \
    node_stack.pop();                                      \
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
    if (node.name == "") {
        GENERATE("Anonymous");
    }
    GENERATE(node.name);
}

void *DotVisitor::visit(PrimitiveType &node, void *args) {
    GENERATE_TYPE(node.to_string());
}

void *DotVisitor::visit(NamedType &node, void *args) {
    GENERATE_TYPE(node.name);
}

void *DotVisitor::visit(VariableDeclaration &node, void *args) {
    GENERATE("Variable");
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

void *DotVisitor::visit(ArrayInitializationList &node, void *args) {
    GENERATE("Array Init");
};

void *DotVisitor::visit(FunctionType &node, void *args) {
    GENERATE_TYPE(std::string("FunctionType") + (node.varargs ? "..." : ""));
};

void *DotVisitor::visit(StructType &node, void *args) {
    GENERATE_TYPE("StructType");
};

void *DotVisitor::visit(UnionType &node, void *args) {
    GENERATE_TYPE("UnionType");
};
void *DotVisitor::visit(Attribute &node, void *args) {
    GENERATE("Attribute: " + node.name);
};
void *DotVisitor::visit(Assembly &node, void *args) {
    std::stringstream s;
    for (auto &i : node.assembly) {
        s << "\\\"" << i << "\\\""
          << "\n";
    }
    GENERATE("Asm: " + s.str());
};
void *DotVisitor::visit(If &node, void *args) {
    GENERATE("If");
};
void *DotVisitor::visit(ArrayAccess &node, void *args) {
    GENERATE("ArrayAccess");
};
void *DotVisitor::visit(StructAccess &node, void *args) {
    GENERATE(std::string("StructAccess") + (node.through_pointer ? " (ptr)" : ""));
};
void *DotVisitor::visit(Assignment &node, void *args) {
    GENERATE("=");
};
void *DotVisitor::visit(For &node, void *args) {
    GENERATE("For");
};
void *DotVisitor::visit(TypeCast &node, void *args) {
    GENERATE("TypeCast");
};
void *DotVisitor::visit(TernaryExpression &node, void *args) {
    GENERATE("Ternary");
};
void *DotVisitor::visit(OperationAssignment &node, void *args) {
    GENERATE(node.op_to_str());
};
void *DotVisitor::visit(ExpressionList &node, void *args) {
    GENERATE(",");
};
void *DotVisitor::visit(EnumType &node, void *args) {
    GENERATE_TYPE("enum");
};
void *DotVisitor::visit(EnumValue &node, void *args) {
    GENERATE("EnumValue");
};
void *DotVisitor::visit(While &node, void *args) {
    GENERATE("While");
};
void *DotVisitor::visit(DoWhile &node, void *args) {
    GENERATE("DoWhile");
};
void *DotVisitor::visit(Switch &node, void *args) {
    GENERATE("Switch");
};
void *DotVisitor::visit(SwitchBlock &node, void *args) {
    GENERATE(std::string(node.is_default ? "default" : "case") + std::string(node.break_after ? " (break)" : ""));
};

}  // namespace CCOMP::AST
