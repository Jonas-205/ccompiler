grammar C;

@parser::header {
#include "ast.hpp"
#include "memory"
#include "common.hpp"
using namespace CCOMP::AST;

#include <sstream>

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
    | fn=functionCall SEMICOLON
    {
        $ast = std::move($fn.ast);
    }
    | b=block
    {
        $ast = std::move($b.ast);
    }
    | i=ifStatement
    {
        $ast = std::move($i.ast);
    }
    | f=forStatement
    {
        $ast = std::move($f.ast);
    }
    | w=whileStatement
    {
        $ast = std::move($w.ast);
    }
    | dw=doWhileStatement SEMICOLON
    {
        $ast = std::move($dw.ast);
    }
    | sw=switchStatement
    {
        $ast = std::move($sw.ast);
    }
    | exp=expression SEMICOLON
    {
        $ast = std::move($exp.ast);
    }
    ;

ifStatement returns [ std::unique_ptr<If> ast ]
    : IF LPAREN cond=expression RPAREN b=statement (ELSE b2=statement)?
    {
        Token *symbol = $ctx->IF()->getSymbol();
        if ($b2.ctx != nullptr) {
            $ast = std::make_unique<If>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($cond.ast), std::move($b.ast), std::move($b2.ast));
        } else {
            $ast = std::make_unique<If>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($cond.ast), std::move($b.ast));
        }
    }
    ;


forStatement returns [ std::unique_ptr<For> ast ]
    : FOR LPAREN (init=expression)? SEMICOLON (cond=expression)? SEMICOLON (inc=expression)? RPAREN s=statement
    {
        Token *symbol = $ctx->FOR()->getSymbol();
        auto f = std::make_unique<For>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($s.ast));
        if ($init.ctx != nullptr) { f->set_init(std::move($init.ast)); }
        if ($cond.ctx != nullptr) { f->set_condition(std::move($cond.ast)); }
        if ($inc.ctx != nullptr) { f->set_increment(std::move($inc.ast)); }
        $ast = std::move(f);
    }
    ;

whileStatement returns [ std::unique_ptr<While> ast ]
    : WHILE LPAREN cond=expression RPAREN s=statement
    {
        Token *symbol = $ctx->WHILE()->getSymbol();
        $ast = std::make_unique<While>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($cond.ast), std::move($s.ast));
    }
    ;


doWhileStatement returns [ std::unique_ptr<DoWhile> ast ]
    : DO s=statement WHILE LPAREN cond=expression RPAREN
    {
        Token *symbol = $ctx->WHILE()->getSymbol();
        $ast = std::make_unique<DoWhile>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($cond.ast), std::move($s.ast));
    }
    ;

switchStatement returns [ std::unique_ptr<Switch> ast ]
    : SWITCH LPAREN e=expression RPAREN LBRACE (s+=switchBlock)* RBRACE
    {
        Token *symbol = $ctx->SWITCH()->getSymbol();
        $ast = std::make_unique<Switch>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($e.ast));
        for (int i = 0; i < $s.size(); i++) {
            $ast->add_switch_block(std::move($s[i]->ast));
        }
    }
    ;

switchBlock returns [ std::unique_ptr<SwitchBlock> ast ]
    : (CASE e=expression | DEFAULT) COLON (s+=statement)* (BREAK SEMICOLON)?
    {
        Token *symbol = $ctx->COLON()->getSymbol();
        $ast = std::make_unique<SwitchBlock>(symbol->getLine(), symbol->getCharPositionInLine());
        for (int i = 0; i < $s.size(); i++) {
            $ast->add_statement(std::move($s[i]->ast));
        }
        if ($e.ctx != nullptr) {
            $ast->label = std::move($e.ast);
        } else {
            $ast->is_default = true;
        }

        if ($ctx->BREAK() != nullptr) {
            $ast->break_after = true;
        }
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

globalDeclarationHelperSemi returns [ std::unique_ptr<Declaration> ast ]
    : a=attribute g=globalDeclarationHelperSemi
    {
        $ast = std::move($g.ast);
        $ast->add_attribute($a.ast);
    }
    | gl=globalDeclarationHelperSemi at=attribute
    {
        $ast = std::move($gl.ast);
        $ast->add_attribute($at.ast);
    }
    | ass=assembly glo=globalDeclarationHelperSemi
    {
        $ast = std::move($glo.ast);
        $ast->add_assembly(std::move($ass.ast));
    }
    | glob=globalDeclarationHelperSemi as=assembly
    {
        $ast = std::move($glob.ast);
        $ast->add_assembly(std::move($as.ast));
    }
    |  decl=functionDeclaration
    {
       $ast = std::move($decl.ast);
    }
    | var=globalVariableDeclaration
    {
        $ast = std::move($var.ast);
    }
    | sdc=structDeclaration
    {
        $ast = std::move($sdc.ast);
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
    | en=enumDeclaration
    {
        $ast = std::move($en.ast);
    }
    | enu=enumDefinition
    {
        $ast = std::move($enu.ast);
    }
    | td=typedef
    {
       $ast = std::move($td.ast);
    }
    ;


globalDeclarationHelperNoSemi returns [ std::unique_ptr<Declaration> ast ]
    : a=attribute g=globalDeclarationHelperNoSemi
    {
        $ast = std::move($g.ast);
        $ast->add_attribute($a.ast);
    }
    | gl=globalDeclarationHelperNoSemi at=attribute
    {
        $ast = std::move($gl.ast);
        $ast->add_attribute($at.ast);
    }
    | ass=assembly glo=globalDeclarationHelperNoSemi
    {
        $ast = std::move($glo.ast);
        $ast->add_assembly(std::move($ass.ast));
    }
    | glob=globalDeclarationHelperNoSemi as=assembly
    {
        $ast = std::move($glob.ast);
        $ast->add_assembly(std::move($as.ast));
    }
    | fn=functionDefinition
    {
        $ast = std::move($fn.ast);
    }
    ;

globalDeclaration returns [ std::unique_ptr<Declaration> ast ]
    : g=globalDeclarationHelperSemi SEMICOLON
    {
        $ast = std::move($g.ast);
    }
    | gl=globalDeclarationHelperNoSemi
    {
       $ast = std::move($gl.ast);
    }
    ;

attribute returns [ std::vector<std::unique_ptr<Attribute>> ast ]
    : ATTRIBUTE LPAREN LPAREN s=attributeContent RPAREN RPAREN
    {
        Token *symbol = $ctx->ATTRIBUTE()->getSymbol();
        std::vector<std::unique_ptr<Attribute>> erg;
        for (auto s: $s.v) {
            erg.push_back(std::move(std::make_unique<Attribute>(symbol->getLine(), symbol->getCharPositionInLine(), s)));
        }
        $ast = std::move(erg);
    }
    ;

assembly returns [ std::unique_ptr<Assembly> ast ]
    : ASSEMBLY LPAREN s=assemblyContent RPAREN
    {
        Token *symbol = $ctx->ASSEMBLY()->getSymbol();
        $ast = std::make_unique<Assembly>(symbol->getLine(), symbol->getCharPositionInLine(), $s.s);
    }
    ;

assemblyContent returns [ std::vector<std::string> s ]
    : (c+=string)*
    {
        $s.reserve($c.size());
        for (int i = 0; i < $c.size(); i++) {
            $s.push_back($c[i]->s);
        }
    }
    ;

attributeContent returns [ std::vector<std::string> v ]
    : id=identifier
    {
        $v = { $id.ast->name };
    }
    | c=constant
    {
        $v = { $c.ast->value };
    }
    | a1=attributeContent COMMA a2=attributeContent
    {
        $a1.v.insert($a1.v.end(), $a2.v.begin(), $a2.v.end());
        $v = $a1.v;
    }
    |  a3=attributeContent LPAREN a4=attributeContent RPAREN
    {
        $a3.v.back() += "(";
        bool first = true;
        for (auto s: $a4.v) {
            if (first) {
                first = false;
            } else {
                $a3.v.back() += ',';
            }
            $a3.v.back() += s;
        }
        $a3.v.back() += ")";
        $v = $a3.v;
    }
    ;

fnTypeWithoutReturn returns [ std::unique_ptr<Identifier> ast ]
    : (l+=LPAREN STAR? (id=identifier)? RPAREN | STAR? (id=identifier)?) l+=LPAREN (args+=parameterDeclaration (COMMA args+=parameterDeclaration)*)? (COMMA var=VA_ARGS)? RPAREN
         {
             Token *symbol = $ctx->l[0];
             if ($id.ctx != nullptr) {
                $ast = std::move($id.ast);
              } else {
                $ast = std::make_unique<Identifier>(symbol->getLine(), symbol->getCharPositionInLine(), "");
              }

             auto fn = std::make_unique<FunctionType>($ast->get_line(), $ast->get_column(), nullptr);
             if ($var != nullptr) { fn->varargs = true; }

             for (int i = 0; i < $args.size(); i++) {
                fn->add_parameter(std::move($args[i]->ast));
             }
             if ($ctx->STAR() != nullptr) {
                fn->pointer_count++;
             }
             $ast->add_type(std::move(fn));
         }

    ;

fnType returns [ std::unique_ptr<Identifier> ast ]
    : type fnTypeWithoutReturn
    {
             $ast = std::move($fnTypeWithoutReturn.ast);
             dynamic_cast<FunctionType*>($ast->type())->return_type = std::move($type.ast);
    }
    | type (l+=LPAREN STAR? (id=fnTypeWithoutReturn)? RPAREN | STAR? (id=fnTypeWithoutReturn)?) l+=LPAREN (args+=parameterDeclaration (COMMA args+=parameterDeclaration)*)? (COMMA va=VA_ARGS)? RPAREN
        {
            Token *symbol = $ctx->l[0];
             if ($id.ctx != nullptr) {
                $ast = std::move($id.ast);
             } else {
                $ast = std::make_unique<Identifier>(symbol->getLine(), symbol->getCharPositionInLine(), "");
             }

             auto fn = std::make_unique<FunctionType>($ast->get_line(), $ast->get_column(), std::move($type.ast));
             if ($va != nullptr) { fn->varargs = true; }

             for (int i = 0; i < $args.size(); i++) {
                 fn->add_parameter(std::move($args[i]->ast));
             }
             if ($ctx->STAR() != nullptr) {
                fn->pointer_count++;
             }

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
        $type.ast->pointer_count++;
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

arrayInitializerList returns [ std::unique_ptr<ArrayInitializationList> ast ]
    : LBRACE (item+=expression (COMMA item+=expression)*)? RBRACE
    {
        Token *symbol = $ctx->LBRACE()->getSymbol();
        $ast = std::make_unique<ArrayInitializationList>(symbol->getLine(), symbol->getCharPositionInLine());

        for (int i = 0; i < $item.size(); i++) {
            $ast->add_value(std::move($item[i]->ast));
        }
    }
    ;

expression returns [ std::unique_ptr<AST> ast ]
    : p=presedence_15
    {
        $ast = std::move($p.ast);
    }
    ;

presedence_15 returns [ std::unique_ptr<AST> ast ]
    : p1=presedence_14
    {
        $ast = std::move($p1.ast);
    }
    | p+=presedence_14 (COMMA p+=presedence_14)+
    {
        auto l = std::make_unique<ExpressionList>($p[0]->ast->get_line(), $p[0]->ast->get_column());
        for (int i = 0; i < $p.size(); i++) {
            l->add_expression(std::move($p[i]->ast));
        }
        $ast = std::move(l);
    }
    ;

presedence_14 returns [ std::unique_ptr<AST> ast ]
    : a=presedence_2 EQUAL b=presedence_14
    {
        $ast = std::make_unique<Assignment>($a.ast->get_line(), $a.ast->get_column(), std::move($a.ast), std::move($b.ast));
    }
    | a1=presedence_2 op=( MINUSEQUAL | PLUSEQUAL )  b1=presedence_13
    {
        $ast = std::make_unique<OperationAssignment>($a1.ast->get_line(), $a1.ast->get_column(), std::move($a1.ast), std::move($b1.ast), OperationAssignment::str_to_op($op->getText()));
    }
    | p=presedence_13
    {
        $ast = std::move($p.ast);
    }
    ;

presedence_13 returns [ std::unique_ptr<AST> ast ]
    : p=presedence_12
    {
        $ast = std::move($p.ast);
    }
    | cond=presedence_12 QUESTION exp0=expression COLON exp1=presedence_12
    {
        $ast = std::make_unique<TernaryExpression>($cond.ast->get_line(), $cond.ast->get_column(), std::move($cond.ast), std::move($exp0.ast), std::move($exp1.ast));
    }
    ;


presedence_12 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_11
        ( operators+=OROR operands+=presedence_11)*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_11 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_10
        ( operators+=ANDAND operands+=presedence_10)*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_10 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_9
        ( operators+=OR operands+=presedence_9)*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_9 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_8
        ( operators+=XOR operands+=presedence_8)*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_8 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_7
        ( operators+=AND operands+=presedence_7)*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_7 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_6
        ( operators+=( EQUALS | NOT_EQUALS ) operands+=presedence_6)*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_6 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_5
        ( operators+=( LESS | LESS_EQUAL | GREATER | GREATER_EQUAL ) operands+=presedence_5)*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_5 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_4
        ( operators+=( LESSLESS | GREATERGREATER ) operands+=presedence_4 )*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_4 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_3
        ( operators+=( PLUS | MINUS ) operands+=presedence_3 )*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_3 returns [ std::unique_ptr<AST> ast ]
    : operands+=presedence_2
        ( operators+=( STAR | SLASH | PERCENT) operands+=presedence_2 )*
    {
        $ast = std::move($operands[0]->ast);

        for (int i = 1; i < $operands.size(); i++) {
            auto op = BinaryExpression::str_to_op($operators[i - 1]->getText());
            auto right = std::move($operands[i]->ast);

            $ast = std::make_unique<BinaryExpression>($ast->get_line(), $ast->get_column(), std::move($ast), std::move(right), op);
        }
    }
    ;

presedence_2 returns [ std::unique_ptr<AST> ast ]
    : p=presedence_1
    {
        $ast = std::move($p.ast);
    }
    | op=(SIZEOF | PLUSPLUS | MINUSMINUS | AND | STAR | PLUS | MINUS | TILDE | NOT) p3=presedence_2
    {
        $ast = std::make_unique<UnaryExpression>($p3.ast->get_line(), $p3.ast->get_column(), std::move($p3.ast), UnaryExpression::prefix_str_to_op($op->getText()));
    }
    | s=SIZEOF LPAREN ty=type RPAREN
    {
        $ast = std::make_unique<UnaryExpression>($ty.ast->get_line(), $ty.ast->get_column(), std::move($ty.ast), UnaryExpression::prefix_str_to_op($s->getText()));
    }
    | LPAREN t=type RPAREN p1=presedence_2
    {
        $ast = std::make_unique<TypeCast>($p1.ast->get_line(), $p1.ast->get_column(), std::move($t.ast), std::move($p1.ast));
    }
    ;

presedence_1 returns [ std::unique_ptr<AST> ast ]
    : f=factor
    {
        $ast = std::move($f.ast);
    }
    | p0=presedence_1 op=(PLUSPLUS | MINUSMINUS)
    {
        $ast = std::make_unique<UnaryExpression>($p0.ast->get_line(), $p0.ast->get_column(), std::move($p0.ast), UnaryExpression::postfix_str_to_op($op->getText()));
    }
    | fn=functionCall
    {
        $ast = std::move($fn.ast);
    }
    | p4=presedence_1 (LBRACK exp+=expression RBRACK)+
    {
        auto arr = std::make_unique<ArrayAccess>($p4.ast->get_line(), $p4.ast->get_column(), std::move($p4.ast));
        for (int i = 0; i < $exp.size(); i++) {
            arr->add_index(std::move($exp[i]->ast));
        }
        $ast = std::move(arr);
    }
    | p5=presedence_1 DOT id=identifier
    {
        $ast = std::make_unique<StructAccess>($p5.ast->get_line(), $p5.ast->get_column(), std::move($p5.ast), std::move($id.ast), false);
    }
    | p6=presedence_1 MINUSGREATER ide=identifier
    {
        $ast = std::make_unique<StructAccess>($p6.ast->get_line(), $p6.ast->get_column(), std::move($p6.ast), std::move($ide.ast), true);
    }
    | ail=arrayInitializerList
    {
        $ast = std::move($ail.ast);
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
    | LPAREN exp=expression RPAREN
    {
        $ast = std::move($exp.ast);
    }
    ;

constant returns [ std::unique_ptr<Constant> ast ]
    : NUMBER
    {
        Token *symbol = $ctx->NUMBER()->getSymbol();
        $ast = std::make_unique<Constant>(symbol->getLine(), symbol->getCharPositionInLine(), symbol->getText());
    }
    | HEX_NUMBER
    {
        Token *symbol = $ctx->HEX_NUMBER()->getSymbol();
        $ast = std::make_unique<Constant>(symbol->getLine(), symbol->getCharPositionInLine(), symbol->getText());
    }
    | OCT_NUMBER
    {
        Token *symbol = $ctx->OCT_NUMBER()->getSymbol();
        $ast = std::make_unique<Constant>(symbol->getLine(), symbol->getCharPositionInLine(), symbol->getText());
    }
    | BIN_NUMBER
    {
        Token *symbol = $ctx->BIN_NUMBER()->getSymbol();
        $ast = std::make_unique<Constant>(symbol->getLine(), symbol->getCharPositionInLine(), symbol->getText());
    }
    | string
    {
        $ast = std::make_unique<Constant>(0, 0, $string.s);
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
    | p=parameterDeclaration LBRACK (exp=expression)? RBRACK
    {
        $ast = std::move($p.ast);

        if ($exp.ctx != nullptr) {
            $ast->type()->add_array_dimension(std::move($exp.ast));
        } else {
            $ast->type()->add_array_dimension();
        }
    }
    ;

functionDefinition returns [ std::unique_ptr<FunctionDefinition> ast ]
    : (vis=visibility)? (a+=attribute)* fnType (a+=attribute)* block
    {
        $ast = std::make_unique<FunctionDefinition>($fnType.ast->get_line(), $fnType.ast->get_column(), std::move($fnType.ast), std::move($block.ast));
        if ($vis.ctx != nullptr) { $ast->is_public = $vis.is_public; }

        for (int i = 0; i < $a.size(); i++) {
            $ast->add_attribute($a[i]->ast);
        }
    }
    ;

functionDeclaration returns [ std::unique_ptr<FunctionDeclaration> ast ]
    : (vis=visibility)? (a+=attribute)* fnType
    {
        $ast = std::make_unique<FunctionDeclaration>($fnType.ast->get_line(), $fnType.ast->get_column(), std::move($fnType.ast));
        if ($vis.ctx != nullptr) { $ast->is_public = $vis.is_public; }

        for (int i = 0; i < $a.size(); i++) {
            $ast->add_attribute($a[i]->ast);
        }
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
    | en=enumDeclaration
    {
        $ast = std::move($en.ast);
    }
    | enu=enumDefinition
    {
        $ast = std::move($enu.ast);
    }
    | t=type STAR
    {
        $ast = std::move($t.ast);
        $ast->pointer_count++;
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

enumDeclaration returns [ std::unique_ptr<EnumType> ast ]
    : ENUM name=identifier
    {
        Token *symbol = $ctx->ENUM()->getSymbol();
        $ast = std::make_unique<EnumType>(symbol->getLine(), symbol->getCharPositionInLine(), std::move($name.ast), false);
    }
    ;

enumDefinition returns [ std::unique_ptr<EnumType> ast ]
    : ENUM (name=identifier)? LBRACE (var+=enumValue COMMA)* (var+=enumValue)? RBRACE
    {
        Token *symbol = $ctx->ENUM()->getSymbol();
        std::unique_ptr<Identifier> name;
        if ($name.ctx != nullptr) {
            name = std::move($name.ast);
        } else {
            name = std::make_unique<Identifier>(symbol->getLine(), symbol->getCharPositionInLine(), "");
        }
        $ast = std::make_unique<EnumType>(symbol->getLine(), symbol->getCharPositionInLine(), std::move(name), true);
        for (int i = 0; i < $var.size(); i++) {
            $ast->add_value(std::move($var[i]->ast));
        }
    }
    ;

enumValue returns [ std::unique_ptr<EnumValue> ast ]
    : id=identifier (EQUAL c=expression)?
    {
        auto e = std::make_unique<EnumValue>($id.ast->get_line(), $id.ast->get_column(), std::move($id.ast));
        if ($c.ctx != nullptr) {
            e->set_value(std::move($c.ast));
        }
        $ast = std::move(e);
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
    | FLOAT
    {
        Token *symbol = $ctx->FLOAT()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::FLOAT);
    }
    | DOUBLE
    {
        Token *symbol = $ctx->DOUBLE()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::DOUBLE);
    }
    | BUILTIN_VA_LIST
    {
        Token *symbol = $ctx->BUILTIN_VA_LIST()->getSymbol();
        $ast = std::make_unique<PrimitiveType>(symbol->getLine(), symbol->getCharPositionInLine());
        $ast->add_keyword(PrimitiveType::KeyWords::VA_LIST);
    }
    ;


string returns [ std::string s ]
    : c=STRING
    {
        std::string erg = $c->getText();
        $s = erg.substr(1, erg.size() - 2);
    }
    ;

// Lexer

WHITESPACE: (' ' | '\t' | '\r' | '\n') -> skip;
PRE_PROCESSOR_OUTPUT: '#' ~('\n'|'\r')* '\r'? '\n' -> skip;
COMMENT: '//' ~('\n'|'\r')* '\r'? '\n' -> skip;
COMMENT1: '/*' .*? '*/' -> skip;

INLINE: ('__inline' | 'inline') -> skip;
EXTENSION: '__extension__' -> skip;
VOLATILE: 'volatile' -> skip;
NORETURN: '_Noreturn' -> skip;

STRING: '"' (~'"' | '\\"')* '"';

EXTERN: 'extern';
STATIC: 'static';

TYPEDEF: 'typedef';

INT: 'int';
SIGNED: 'signed';
UNSIGNED: 'unsigned';
CHAR: 'char';
SHORT: 'short';
LONG: 'long';
FLOAT: 'float';
DOUBLE: 'double';
VOID: 'void';
BUILTIN_VA_LIST: '__builtin_va_list';
STRUCT: 'struct';
UNION: 'union';
ENUM: 'enum';

IF: 'if';
ELSE: 'else';
WHILE: 'while';
DO: 'do';
FOR: 'for';
SWITCH: 'switch';
CASE: 'case';
BREAK: 'break';
DEFAULT: 'default';

RETURN: 'return';

CONST: 'const';
RESTRICT: 'restrict' | '__restrict' | '__restrict__';
ATTRIBUTE: '__attribute__';
ASSEMBLY: '__asm__';

SIZEOF: 'sizeof';

VA_ARGS: '...';
DOT: '.';

QUOTES: '"';

LPAREN: '(';
RPAREN: ')';

LBRACK: '[';
RBRACK: ']';

LBRACE: '{';
RBRACE: '}';

PLUSPLUS: '++';
MINUSMINUS: '--';

MINUSGREATER: '->';

PLUS: '+';
MINUS: '-';
STAR: '*';
SLASH: '/';
PERCENT: '%';

PLUSEQUAL: '+=';
MINUSEQUAL: '-=';
STAREQUAL: '*=';
SLASHEQUAL: '/=';
PERCENTEQUAL: '%=';
LESSLESSEQUAL: '<<=';
GREATERGREATEREQUAL: '>>=';
ANDEQUAL: '&=';
XOREQUAL: '^=';
OREQUAL: '|=';

EQUALS: '==';
NOT_EQUALS: '!=';
GREATER_EQUAL: '>=';
LESS_EQUAL: '<=';
LESSLESS: '<<';
GREATERGREATER: '>>';
GREATER: '>';
LESS: '<';
EQUAL: '=';

ANDAND: '&&';
AND: '&';

XOR: '^';

OROR: '||';
OR: '|';

NOT: '!';
TILDE: '~';

COMMA: ',';
SEMICOLON: ';';

QUESTION: '?';
COLON: ':';

HEX_NUMBER: '0x' [0-9a-f]+ [ul]*;
BIN_NUMBER: '0b' [0-1]+ [ul]*;
OCT_NUMBER: '0' [0-7]+ [ul]*;
NUMBER: [0-9]+ ('.' [0-9]*)? [ul]*;
IDENTIFIER: [a-zA-Z_]+[a-zA-Z0-9_]*;
