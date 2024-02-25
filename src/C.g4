grammar C;

@parser::header {
#include "ast.hpp"
#include "memory"
using namespace CCOMP::AST;
}


// Parser

program returns [ std::unique_ptr<Program> ast ]
    : (decls += declaration)*
    {
        $ast = std::make_unique<Program>(0, 0);
        for (int i = 0; i < $decls.size(); i++) {
            $ast->add_declaration(std::move($decls[i]->ast));
        }
    }
    ;

block returns [ std::unique_ptr<Block> ast ]
    : LBRACE (s+=statement)* RBRACE
    {
        Token *symbol = $ctx->LBRACE()->getSymbol();
        $ast = std::make_unique<Block>(symbol->getLine(), symbol->getCharPositionInLine());
        for (int i = 0; i < $s.size(); i++) {
            $ast->add_statement(std::move($s[i]->ast));
        }
    }
    ;


statement returns [ std::unique_ptr<AST> ast ]
    : variableDeclaration
        {
            $ast = std::move($variableDeclaration.ast);
        }
    | returnStatement
        {
            $ast = std::move($returnStatement.ast);
        }
    ;


returnStatement returns [ std::unique_ptr<Return> ast ]
    : RETURN (exp=expression)? SEMICOLON
    {
        std::unique_ptr<AST> e = nullptr;
        if ($exp.ctx != nullptr) { e = std::move($exp.ast); }
        Token *symbol = $ctx->RETURN()->getSymbol();
        $ast = std::make_unique<Return>(symbol->getLine(), symbol->getCharPositionInLine(), std::move(e));
    }
    ;


declaration returns [ std::unique_ptr<Declaration> ast ]
    : var=globalVariableDeclaration
    {
        $ast = std::move($var.ast);
    }
    | fn=functionDeclaration
        {
            $ast = std::move($fn.ast);
        }
    ;


expression returns [ std::unique_ptr<AST> ast ]
    : add=additiveExpression
    {
        $ast = std::move($add.ast);
    }
    ;

additiveExpression returns [ std::unique_ptr<AST> ast ]
    : operands+=multiplicativeExpression
        ( operators+=( PLUS | MINUS ) operands+=multiplicativeExpression )*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

multiplicativeExpression returns [ std::unique_ptr<AST> ast ]
    : operands+=factor
        ( operators+=( STAR | SLASH ) operands+=factor )*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);

        }
    }
    ;

factor returns [ std::unique_ptr<AST> ast ]
    : con=constant
    {
        $ast = std::move($con.ast);
    }
    | id=identifier
    {
        $ast = std::move($id.ast);
    }
    | fn=functionCall
    {
        $ast = std::move($fn.ast);
    }
    ;

constant returns [ std::unique_ptr<Constant> ast ]
    : NUMBER
    {
        Token *symbol = $ctx->NUMBER()->getSymbol();
        $ast = std::make_unique<Constant>(symbol->getLine(), symbol->getCharPositionInLine(), symbol->getText());
    }
    ;

identifier returns [ std::unique_ptr<Identifier> ast ]
    : IDENTIFIER
    {
        Token *symbol = $ctx->IDENTIFIER()->getSymbol();
        $ast = std::make_unique<Identifier>(symbol->getLine(), symbol->getCharPositionInLine(), symbol->getText());
    }
    ;

visibility returns [ bool is_public ]
    : EXTERN { $is_public = true; }
    | STATIC { $is_public = false; }
    ;

parameterDeclaration returns [ std::unique_ptr<ParameterDeclaration> ast ]
    : t=type (id=identifier)?
    {
        std::unique_ptr<Identifier> id = nullptr;
        if ($id.ctx != nullptr) { id = std::move($id.ast); }
        $ast = std::make_unique<ParameterDeclaration>($t.ast->get_line(), $t.ast->get_column(), std::move(id), std::move($t.ast));
    }
    ;

functionDeclaration returns [ std::unique_ptr<FunctionDeclaration> ast ]
    : (vis=visibility)? type name=identifier LPAREN (args+=parameterDeclaration (COMMA args+=parameterDeclaration)*)? RPAREN block
    {
        $ast = std::make_unique<FunctionDeclaration>($type.ast->get_line(), $type.ast->get_column(), std::move($name.ast), std::move($type.ast), std::move($block.ast));
        for (int i = 0; i < $args.size(); i++) {
            $ast->add_parameter(std::move($args[i]->ast));
        }
        if ($vis.ctx != nullptr) { $ast->is_public = $vis.is_public; }
    }
    ;

functionCall returns [ std::unique_ptr<FunctionCall> ast ]
    : name=identifier LPAREN (args+=expression (COMMA args+=expression)*)? RPAREN
    {
        $ast = std::make_unique<FunctionCall>($name.ast->get_line(), $name.ast->get_column(), std::move($name.ast));
        for (int i = 0; i < $args.size(); i++) {
            $ast->add_argument(std::move($args[i]->ast));
        }
    }
    ;

variableDeclaration returns [ std::unique_ptr<VariableDeclaration> ast ]
    : type name=identifier SEMICOLON
    {
        $ast = std::make_unique<VariableDeclaration>($name.ast->get_line(), $name.ast->get_column(), std::move($name.ast), std::move($type.ast), nullptr);
    }
    | type name=identifier EQUAL expression SEMICOLON
        {
            $ast = std::make_unique<VariableDeclaration>($name.ast->get_line(), $name.ast->get_column(), std::move($name.ast), std::move($type.ast), std::move($expression.ast));
        }
    ;

globalVariableDeclaration returns [ std::unique_ptr<VariableDeclaration> ast ]
    : (vis=visibility)? var=variableDeclaration
    {
        $ast = std::move($var.ast);
        $ast->global = true;
        if ($vis.ctx != nullptr) { $ast->is_public = $vis.is_public; }
    }
    ;

type returns  [ std::unique_ptr<Type> ast ]
    : INT
    {
        Token *symbol = $ctx->INT()->getSymbol();
        $ast = std::make_unique<Type>(symbol->getLine(), symbol->getCharPositionInLine(), Type::PrimitiveType::INT);
    }
    | VOID
    {
        Token *symbol = $ctx->VOID()->getSymbol();
        $ast = std::make_unique<Type>(symbol->getLine(), symbol->getCharPositionInLine(), Type::PrimitiveType::VOID);
    }
    | name=identifier
        {
            $ast = std::make_unique<Type>($name.ast->get_line(), $name.ast->get_column(), std::move($name.ast));
        }
    ;


// Lexer

WHITESPACE: (' ' | '\t' | '\r' | '\n') -> skip;
COMMENT: '#' ~('\n'|'\r')* '\r'? '\n' -> skip;

EXTERN: 'extern';
STATIC: 'static';

INT: 'int';
VOID: 'void';

RETURN: 'return';

LPAREN: '(';
RPAREN: ')';

LBRACK: '[';
RBRACK: ']';

LBRACE: '{';
RBRACE: '}';

PLUS: '+';
MINUS: '-';
STAR: '*';
SLASH: '/';

EQUAL: '=';

COMMA: ',';
SEMICOLON: ';';

NUMBER: [0-9]+ ('.' [0-9]*)?;
IDENTIFIER: [a-zA-Z_]+[a-zA-Z0-9_]*;
