#pragma once

#include "ast.hpp"
namespace CCOMP::AST {

typedef class Program Program;
typedef class Block Block;
typedef class Constant Constant;
typedef class Identifier Identifier;
typedef class Type Type;
typedef class PrimitiveType PrimitiveType;
typedef class VariableDeclaration VariableDeclaration;
typedef class ParameterDeclaration ParameterDeclaration;
typedef class FunctionDefinition FunctionDefinition;
typedef class FunctionDeclaration FunctionDeclaration;
typedef class FunctionCall FunctionCall;
typedef class UnaryExpression UnaryExpression;
typedef class BinaryExpression BinaryExpression;
typedef class Return Return;
typedef class TypeDef TypeDef;
typedef class NamedType NamedType;
typedef class StructDefinition StructDefinition;
typedef class StructDeclaration StructDeclaration;
typedef class ArrayInitializationList ArrayInitializationList;
typedef class UnionDefinition UnionDefinition;
typedef class UnionDeclaration UnionDeclaration;
typedef class SizeOf SizeOf;

class ASTVisitor {
   public:
    virtual void *visit(Program &node, void *args) = 0;
    virtual void *visit(Block &node, void *args) = 0;
    virtual void *visit(Constant &node, void *args) = 0;
    virtual void *visit(Identifier &node, void *args) = 0;
    virtual void *visit(PrimitiveType &node, void *args) = 0;
    virtual void *visit(VariableDeclaration &node, void *args) = 0;
    virtual void *visit(ParameterDeclaration &node, void *args) = 0;
    virtual void *visit(FunctionDefinition &node, void *args) = 0;
    virtual void *visit(FunctionDeclaration &node, void *args) = 0;
    virtual void *visit(FunctionCall &node, void *args) = 0;
    virtual void *visit(UnaryExpression &node, void *args) = 0;
    virtual void *visit(BinaryExpression &node, void *args) = 0;
    virtual void *visit(Return &node, void *args) = 0;
    virtual void *visit(TypeDef &node, void *args) = 0;
    virtual void *visit(NamedType &node, void *args) = 0;
    virtual void *visit(StructDefinition &node, void *args) = 0;
    virtual void *visit(StructDeclaration &node, void *args) = 0;
    virtual void *visit(ArrayInitializationList &node, void *args) = 0;
    virtual void *visit(UnionDefinition &node, void *args) = 0;
    virtual void *visit(UnionDeclaration &node, void *args) = 0;
    virtual void *visit(SizeOf &node, void *args) = 0;
};

}  // namespace CCOMP::AST
