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
typedef class FunctionDefinition FunctionDefinition;
typedef class FunctionDeclaration FunctionDeclaration;
typedef class FunctionCall FunctionCall;
typedef class UnaryExpression UnaryExpression;
typedef class BinaryExpression BinaryExpression;
typedef class Return Return;
typedef class TypeDef TypeDef;
typedef class NamedType NamedType;
typedef class ArrayInitializationList ArrayInitializationList;
typedef class FunctionType FunctionType;
typedef class StructType StructType;
typedef class UnionType UnionType;
typedef class Attribute Attribute;
typedef class Assembly Assembly;
typedef class If If;
typedef class ArrayAccess ArrayAccess;
typedef class StructAccess StructAccess;
typedef class Assignment Assignment;
typedef class OperationAssignment OperationAssignment;
typedef class For For;
typedef class While While;
typedef class DoWhile DoWhile;
typedef class TypeCast TypeCast;
typedef class TernaryExpression TernaryExpression;
typedef class ExpressionList ExpressionList;
typedef class EnumType EnumType;
typedef class EnumValue EnumValue;
typedef class Switch Switch;
typedef class SwitchBlock SwitchBlock;

class ASTVisitor {
   public:
    virtual void *visit(Program &node, void *args) = 0;
    virtual void *visit(Block &node, void *args) = 0;
    virtual void *visit(Constant &node, void *args) = 0;
    virtual void *visit(Identifier &node, void *args) = 0;
    virtual void *visit(PrimitiveType &node, void *args) = 0;
    virtual void *visit(VariableDeclaration &node, void *args) = 0;
    virtual void *visit(FunctionDefinition &node, void *args) = 0;
    virtual void *visit(FunctionDeclaration &node, void *args) = 0;
    virtual void *visit(FunctionCall &node, void *args) = 0;
    virtual void *visit(UnaryExpression &node, void *args) = 0;
    virtual void *visit(BinaryExpression &node, void *args) = 0;
    virtual void *visit(Return &node, void *args) = 0;
    virtual void *visit(TypeDef &node, void *args) = 0;
    virtual void *visit(NamedType &node, void *args) = 0;
    virtual void *visit(ArrayInitializationList &node, void *args) = 0;
    virtual void *visit(FunctionType &node, void *args) = 0;
    virtual void *visit(StructType &node, void *args) = 0;
    virtual void *visit(UnionType &node, void *args) = 0;
    virtual void *visit(Attribute &node, void *args) = 0;
    virtual void *visit(Assembly &node, void *args) = 0;
    virtual void *visit(If &node, void *args) = 0;
    virtual void *visit(ArrayAccess &node, void *args) = 0;
    virtual void *visit(StructAccess &node, void *args) = 0;
    virtual void *visit(Assignment &node, void *args) = 0;
    virtual void *visit(OperationAssignment &node, void *args) = 0;
    virtual void *visit(For &node, void *args) = 0;
    virtual void *visit(While &node, void *args) = 0;
    virtual void *visit(DoWhile &node, void *args) = 0;
    virtual void *visit(TypeCast &node, void *args) = 0;
    virtual void *visit(TernaryExpression &node, void *args) = 0;
    virtual void *visit(ExpressionList &node, void *args) = 0;
    virtual void *visit(EnumType &node, void *args) = 0;
    virtual void *visit(EnumValue &node, void *args) = 0;
    virtual void *visit(Switch &node, void *args) = 0;
    virtual void *visit(SwitchBlock &node, void *args) = 0;
};

}  // namespace CCOMP::AST
