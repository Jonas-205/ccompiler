grammar C;

@parser::header {
#include "ast.hpp"
#include "memory"
#include "common.hpp"
using namespace CCOMP::AST;
}


// Parser

program returns [ std::unique_ptr<Program> ast ]
    : (decls += globalDeclaration)* EOF
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
    : variableDeclaration SEMICOLON
        {
            $ast = std::move($variableDeclaration.ast);
        }
    | returnStatement SEMICOLON
        {
            $ast = std::move($returnStatement.ast);
        }
    ;

returnStatement returns [ std::unique_ptr<Return> ast ]
    : RETURN (exp=expression)?
    {
        std::unique_ptr<AST> e = nullptr;
        if ($exp.ctx != nullptr) { e = std::move($exp.ast); }
        Token *symbol = $ctx->RETURN()->getSymbol();
        $ast = std::make_unique<Return>(symbol->getLine(), symbol->getCharPositionInLine(), std::move(e));
    }
    ;

globalDeclaration returns [ std::unique_ptr<Declaration> ast ]
    : (a1=attribute)? fn=functionDefinition (a11=attribute)?
    {
        $ast = std::move($fn.ast);
        if ($a1.ctx != nullptr) { $ast->add_attribute(std::move($a1.ast)); }
        if ($a11.ctx != nullptr) { $ast->add_attribute(std::move($a11.ast)); }
    }
    | (a2=attribute)? decl=functionDeclaration (a12=attribute)? SEMICOLON
    {
       $ast = std::move($decl.ast);
        if ($a2.ctx != nullptr) { $ast->add_attribute(std::move($a2.ast)); }
        if ($a12.ctx != nullptr) { $ast->add_attribute(std::move($a12.ast)); }
    }
    | (a0=attribute)? var=globalVariableDeclaration (a10=attribute)? SEMICOLON
    {
        $ast = std::move($var.ast);
        if ($a0.ctx != nullptr) { $ast->add_attribute(std::move($a0.ast)); }
        if ($a10.ctx != nullptr) { $ast->add_attribute(std::move($a10.ast)); }
    }
    | (a3=attribute)? sdc=structDeclaration (a13=attribute)? SEMICOLON
    {
        $ast = std::move($sdc.ast);
        if ($a3.ctx != nullptr) { $ast->add_attribute(std::move($a3.ast)); }
        if ($a13.ctx != nullptr) { $ast->add_attribute(std::move($a13.ast)); }
    }
    | (a4=attribute)? sdf=structDefinition (a14=attribute)? SEMICOLON
    {
        $ast = std::move($sdf.ast);
        if ($a4.ctx != nullptr) { $ast->add_attribute(std::move($a4.ast)); }
        if ($a14.ctx != nullptr) { $ast->add_attribute(std::move($a14.ast)); }
    }
    | (a5=attribute)? ud=unionDeclaration (a15=attribute)? SEMICOLON
    {
        $ast = std::move($ud.ast);
        if ($a5.ctx != nullptr) { $ast->add_attribute(std::move($a5.ast)); }
        if ($a15.ctx != nullptr) { $ast->add_attribute(std::move($a15.ast)); }
    }
    | (a6=attribute)? udf=unionDefinition (a16=attribute)? SEMICOLON
    {
        $ast = std::move($udf.ast);
        if ($a6.ctx != nullptr) { $ast->add_attribute(std::move($a6.ast)); }
        if ($a16.ctx != nullptr) { $ast->add_attribute(std::move($a16.ast)); }
    }
    | td=typedef SEMICOLON
        {
            $ast = std::move($td.ast);
        }
    ;

attribute returns [ std::unique_ptr<Attribute> ast ]
    : ATTRIBUTE LPAREN LPAREN s=attributeContent RPAREN RPAREN
    {
        Token *symbol = $ctx->ATTRIBUTE()->getSymbol();
        $ast = std::make_unique<Attribute>(symbol->getLine(), symbol->getCharPositionInLine(), $s.s);
    }
    ;

attributeContent returns [ std::string s ]
    : id=identifier
    {
        $s = $id.ast->name;
    }
    | c=constant
    {
        $s = $c.ast->value;
    }
    | a1=attributeContent a2=attributeContent
    {
        $s = $a1.s + " " + $a2.s;
    }
    | LPAREN a3=attributeContent RPAREN
    {
            $s = "(" + $a3.s + ")";
        }
    ;

fnTypeWithoutReturn returns [ std::unique_ptr<Identifier> ast ]
    : id=identifier LPAREN (args+=parameterDeclaration (COMMA args+=parameterDeclaration)*)? (COMMA va=VA_ARGS)? RPAREN
    {
        $ast = std::move($identifier.ast);
        auto fn = std::make_unique<FunctionType>($ast->get_line(), $ast->get_column(), nullptr);
        if ($va != nullptr) { fn->varargs = true; }
        for (int i = 0; i < $args.size(); i++) {
           fn->add_parameter(std::move($args[i]->ast));
        }
        $ast->add_type(std::move(fn));
    }
    | s=LPAREN STAR RPAREN LPAREN (args+=parameterDeclaration (COMMA args+=parameterDeclaration)*)? (COMMA var=VA_ARGS)? RPAREN
         {
             Token *symbol = $ctx->s;
             $ast = std::make_unique<Identifier>(symbol->getLine(), symbol->getCharPositionInLine(), "");

             auto fn = std::make_unique<FunctionType>($ast->get_line(), $ast->get_column(), nullptr);
             if ($var != nullptr) { fn->varargs = true; }

             for (int i = 0; i < $args.size(); i++) {
                fn->add_parameter(std::move($args[i]->ast));
             }
             fn->is_pointer = true;
             $ast->add_type(std::move(fn));
         }

    ;

fnType returns [ std::unique_ptr<Identifier> ast ]
    : type fnTypeWithoutReturn
         {
             $ast = std::move($fnTypeWithoutReturn.ast);
             dynamic_cast<FunctionType*>($ast->type())->return_type = std::move($type.ast);
         }
    | type LPAREN STAR id=fnTypeWithoutReturn RPAREN LPAREN (args+=parameterDeclaration (COMMA args+=parameterDeclaration)*)? (COMMA va=VA_ARGS)? RPAREN
    {
             $ast = std::move($id.ast);
             auto fn = std::make_unique<FunctionType>($ast->get_line(), $ast->get_column(), std::move($type.ast));
             if ($va != nullptr) { fn->varargs = true; }

             for (int i = 0; i < $args.size(); i++) {
                 fn->add_parameter(std::move($args[i]->ast));
             }

             fn->is_pointer = true;
             dynamic_cast<FunctionType*>($ast->type())->return_type = std::move(fn);
    }
    ;

identifier_with_type returns [ std::unique_ptr<Identifier> ast ]
    : type identifier
    {
        $ast = std::move($identifier.ast);
        $ast->add_type(std::move($type.ast));
    }
    | type STAR identifier
    {
        $ast = std::move($identifier.ast);
        $type.ast->is_pointer = true;
        $ast->add_type(std::move($type.ast));
    }
    | fnType
    {
        $ast = std::move($fnType.ast);
    }
    ;

typedef returns [ std::unique_ptr<TypeDef> ast ]
    : TYPEDEF id=identifier_with_type
    {
        Token *symbol = $ctx->TYPEDEF()->getSymbol();
        $ast = std::make_unique<TypeDef>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($id.ast));
    }
    ;

expression returns [ std::unique_ptr<AST> ast ]
    : add=additiveExpression
    {
        $ast = std::move($add.ast);
    }
    | ail=arrayInitializerList
    {
        $ast = std::move($ail.ast);
    }
    ;

arrayInitializerList returns [ std::unique_ptr<ArrayInitializationList> ast ]
    : LBRACE (item+=additiveExpression (COMMA item+=additiveExpression)*)? RBRACE
    {
        Token *symbol = $ctx->LBRACE()->getSymbol();
        $ast = std::make_unique<ArrayInitializationList>(symbol->getLine(), symbol->getCharPositionInLine());

        for (int i = 0; i < $item.size(); i++) {
            $ast->add_value(std::move($item[i]->ast));
        }
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
    : operands+=unary
        ( operators+=( STAR | SLASH ) operands+=unary )*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);

        }
    }
    ;

unary returns [ std::unique_ptr<AST> ast ]
    : factor
    {
        $ast = std::move($factor.ast);
    }
    | AND addr=factor  // addr of
    {
        $ast = std::make_unique<UnaryExpression>($addr.ast->get_line(), $addr.ast->get_column(), std::move($addr.ast), UnaryExpression::Operator::ADDRESS);
    }
    | STAR deref=factor // deref
    {
        $ast = std::make_unique<UnaryExpression>($deref.ast->get_line(), $deref.ast->get_column(), std::move($deref.ast), UnaryExpression::Operator::DEREFERENCE);
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
    | so=sizeof
    {
        $ast = std::move($so.ast);
    }
    ;

sizeof  returns [ std::unique_ptr<SizeOf> ast ]
    : SIZEOF_KEY LPAREN t=type RPAREN
    {
        Token *symbol = $ctx->SIZEOF_KEY()->getSymbol();
        $ast = std::make_unique<SizeOf>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($t.ast));
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
    | STAR identifier
    {
        $ast = std::move($identifier.ast);
        $ast->add_ptr_later = true;
    }
    | LPAREN identifier RPAREN
    {
        $ast = std::move($identifier.ast);
    }
    ;

visibility returns [ bool is_public ]
    : EXTERN { $is_public = true; }
    | STATIC { $is_public = false; }
    ;

parameterDeclaration returns [ std::unique_ptr<Identifier> ast ]
    : t=type
    {
        $ast = std::make_unique<Identifier>($t.ast->get_line(), $t.ast->get_column(), "");
        $ast->add_type(std::move($t.ast));
    }
    | id=identifier_with_type
    {
        $ast = std::move($id.ast);
    }
    ;

functionDefinition returns [ std::unique_ptr<FunctionDefinition> ast ]
    : (vis=visibility)? fnType block
    {
        $ast = std::make_unique<FunctionDefinition>($fnType.ast->get_line(), $fnType.ast->get_column(), std::move($fnType.ast), std::move($block.ast));
        if ($vis.ctx != nullptr) { $ast->is_public = $vis.is_public; }
    }
    ;

functionDeclaration returns [ std::unique_ptr<FunctionDeclaration> ast ]
    : (vis=visibility)? fnType
    {
        $ast = std::make_unique<FunctionDeclaration>($fnType.ast->get_line(), $fnType.ast->get_column(), std::move($fnType.ast));
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
    : id=identifier_with_type (l+=LBRACK (exp+=expression)? RBRACK)*
    {
        $ast = std::make_unique<VariableDeclaration>($id.ast->get_line(), $id.ast->get_column(), std::move($id.ast), nullptr);

        if ($l.size() > 0) {
            $ast->type()->set_array_dimensions($l.size());
                for (int i = 0; i < $exp.size(); i++) {
                    $ast->type()->set_array_dimension(i, std::move($exp[i]->ast));
                }
        }

    }
    | id=identifier_with_type (lb+=LBRACK (ex+=expression)? RBRACK)* EQUAL expression
        {
            $ast = std::make_unique<VariableDeclaration>($id.ast->get_line(), $id.ast->get_column(), std::move($id.ast), std::move($expression.ast));

            if ($lb.size() > 0) {
                $ast->type()->set_array_dimensions($lb.size());
                for (int i = 0; i < $ex.size(); i++) {
                    $ast->type()->set_array_dimension(i, std::move($ex[i]->ast));
                }
            }
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

type returns [ std::unique_ptr<Type> ast ]
    : pt=primitiveType
    {
        $ast = std::move($pt.ast);
    }
    | id=identifier
    {
        $ast = std::make_unique<NamedType>(std::move($id.ast));
    }
    | sdc=structDeclaration
    {
        $ast = std::move($sdc.ast);
    }
    | sdf=structDefinition
    {
        $ast = std::move($sdf.ast);
    }
    | ud=unionDeclaration
    {
        $ast = std::move($ud.ast);
    }
    | udf=unionDefinition
    {
        $ast = std::move($udf.ast);
    }
    | t=type STAR
    {
        $ast = std::move($t.ast);
        if ($ast->is_pointer) {
            Token *symbol = $ctx->STAR()->getSymbol();
            die("Two Stars ('*') in Type %d:%d", symbol->getLine(), symbol->getCharPositionInLine());
        }
        $ast->is_pointer = true;
    }
    | CONST ca=type
    {
        $ast = std::move($ca.ast);
        $ast->is_const = true;
    }
    | cb=type CONST
    {
        $ast = std::move($cb.ast);
        $ast->is_const = true;
    }
    | RESTRICT ra=type
    {
        $ast = std::move($ra.ast);
        $ast->is_restrict = true;
    }
    | rb=type RESTRICT
    {
        $ast = std::move($rb.ast);
        $ast->is_restrict = true;
    }
    ;

structDeclaration returns [ std::unique_ptr<StructType> ast ]
    : STRUCT name=identifier
    {
        Token *symbol = $ctx->STRUCT()->getSymbol();
        $ast = std::make_unique<StructType>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($name.ast), false);
    }
    ;

structDefinition returns [ std::unique_ptr<StructType> ast ]
    : STRUCT (name=identifier)? LBRACE (var+=variableDeclaration SEMICOLON)* RBRACE
    {
        Token *symbol = $ctx->STRUCT()->getSymbol();
        std::unique_ptr<Identifier> name;
        if ($name.ctx != nullptr) {
            name = std::move($name.ast);
        } else {
            name = std::make_unique<Identifier>(symbol->getLine(), symbol->getCharPositionInLine(), "");
        }
        $ast = std::make_unique<StructType>(symbol->getLine(), symbol->getCharPositionInLine(), std::move(name), true);
        for (int i = 0; i < $var.size(); i++) {
            $ast->add_member(std::move($var[i]->ast));
        }
    }
    ;

unionDeclaration returns [ std::unique_ptr<UnionType> ast ]
    : UNION name=identifier
    {
        Token *symbol = $ctx->UNION()->getSymbol();
        $ast = std::make_unique<UnionType>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($name.ast), false);
    }
    ;

unionDefinition returns [ std::unique_ptr<UnionType> ast ]
    : UNION (name=identifier)? LBRACE (var+=variableDeclaration SEMICOLON)* RBRACE
    {
        Token *symbol = $ctx->UNION()->getSymbol();
        std::unique_ptr<Identifier> name;
        if ($name.ctx != nullptr) {
            name = std::move($name.ast);
        } else {
            name = std::make_unique<Identifier>(symbol->getLine(), symbol->getCharPositionInLine(), "");
        }
        $ast = std::make_unique<UnionType>(symbol->getLine(), symbol->getCharPositionInLine(), std::move(name), true);
        for (int i = 0; i < $var.size(); i++) {
            $ast->add_member(std::move($var[i]->ast));
        }
    }
    ;

primitiveType returns [ std::unique_ptr <PrimitiveType> ast ]
    : p=primitiveTypeHelper
    {
        $ast = std::move($p.ast);
    }
    | t=primitiveType h=primitiveTypeHelper
    {
        $ast = std::move($t.ast);
        $ast->add_keyword($h.ast->keywords[0]);
    }
    ;
    
primitiveTypeHelper returns [ std::unique_ptr <PrimitiveType> ast ]
    : INT
    {
        Token *symbol = $ctx->INT()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::INT);
    }
    | VOID
    {
        Token *symbol = $ctx->VOID()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::VOID);
    }
    | SIGNED
    {
        Token *symbol = $ctx->SIGNED()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::SIGNED);
    }
    | UNSIGNED
    {
        Token *symbol = $ctx->UNSIGNED()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::UNSIGNED);
    }
    | CHAR
    {
        Token *symbol = $ctx->CHAR()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::CHAR);
    }
    | SHORT
    {
        Token *symbol = $ctx->SHORT()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::SHORT);
    }
    | LONG
    {
        Token *symbol = $ctx->LONG()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::LONG);
    }
    | BUILTIN_VA_LIST
    {
        Token *symbol = $ctx->BUILTIN_VA_LIST()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::VA_LIST);
    }

    ;

// Lexer

WHITESPACE: (' ' | '\t' | '\r' | '\n') -> skip;
PRE_PROCESSOR_OUTPUT: '#' ~('\n'|'\r')* '\r'? '\n' -> skip;
COMMENT: '//' ~('\n'|'\r')* '\r'? '\n' -> skip;
COMMENT1: '/*' .*? '*/' -> skip;

EXTERN: 'extern';
STATIC: 'static';

TYPEDEF: 'typedef';

SIZEOF_KEY: 'sizeof';
INT: 'int';
SIGNED: 'signed';
UNSIGNED: 'unsigned';
CHAR: 'char';
SHORT: 'short';
LONG: 'long';
VOID: 'void';
BUILTIN_VA_LIST: '__builtin_va_list';
STRUCT: 'struct';
UNION: 'union';

RETURN: 'return';

CONST: 'const';
RESTRICT: 'restrict' | '__restrict' | '__restrict__';
ATTRIBUTE: '__attribute__';

VA_ARGS: '...';

LPAREN: '(';
RPAREN: ')';

LBRACK: '[';
RBRACK: ']';

LBRACE: '{';
RBRACE: '}';

AND: '&';

PLUS: '+';
MINUS: '-';
STAR: '*';
SLASH: '/';

EQUAL: '=';

COMMA: ',';
SEMICOLON: ';';

NUMBER: [0-9]+ ('.' [0-9]*)?;
IDENTIFIER: [a-zA-Z_]+[a-zA-Z0-9_]*;
