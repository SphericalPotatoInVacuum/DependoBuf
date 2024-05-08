/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
%skeleton "lalr1.cc"
%require  "3.4"
%header

%defines

%define api.token.raw
%define api.value.type variant

%define api.namespace {dbuf::parser}
%define api.parser.class {Parser}

%code requires{
  #include "core/ast/ast.h"
  #include "core/ast/expression.h"

  namespace dbuf::ast {
    class AST;
  }

  namespace dbuf::parser {
    class Lexer;
    class Parser;

    using ExprPtr = std::shared_ptr<const ast::Expression>;
  }
}

%code provides{
namespace dbuf::parser {

  class DbufParser : public Parser {
  public:
    DbufParser(Lexer *scanner, dbuf::ast::AST *ast) : Parser(scanner, ast), error_cnt_(0) {}

    void error(const location_type &l, const std::string &err_message) override {
      std::cerr << "Error: " << err_message << " at " << l << std::endl;
      error_cnt_++;
    }

    size_t GetErrorCnt() const { return error_cnt_; }
  private:
    size_t error_cnt_ = 0;
  };

}
}

%parse-param { Lexer *scanner }
%parse-param { ast::AST *ast }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
  #include "core/ast/ast.h"
  #include "core/ast/expression.h"

  #include "core/parser/lexer.h"

  #include <iostream>
  #include <cstdlib>
  #include <fstream>

#undef yylex
#define yylex scanner->yylex
}

%define api.token.prefix {TOK_}

%token END 0 "end of file"
%token SEMICOLON ";"
%token <std::string> LC_IDENTIFIER UC_IDENTIFIER
%token MESSAGE ENUM IMPL FUNC ARROW EQUAL
%token FALSE TRUE
%token
  PLUS "+"
  MINUS "-"
  STAR "*"
  SLASH "/"
;
%token
  AND "&"
  OR "|"
  BANG "!"
  DOUBLE_AND "&&"
  DOUBLE_OR "||"
  BACK_SLASH "\\"
  IN "in"
;
%token
  LEFT_PAREN "("
  RIGHT_PAREN ")"
  LEFT_BRACE "{"
  RIGHT_BRACE "}"
  LEFT_SQUARE "["
  RIGHT_SQUARE "]"
  LEFT_TRIANGLE "<"
  RIGHT_TRIANGLE ">"
;
%token
  COMMA ","
  DOT "."
  COLON ":"
;

%token <double> FLOAT_LITERAL
%token <int64_t> INT_LITERAL
%token <uint64_t> UINT_LITERAL
%token <std::string> STRING_LITERAL

%left "in"
%left "||"
%left "&&"
%left "\\"
%left "|" "-" "+"
%left "&" "*" "/"
%precedence "!"

%start schema

%%

schema
  : %empty
  | schema definition
  | schema error
  ;

definition
  : message_definition {
    if (ast->types.contains($1.identifier.name)) {
      throw syntax_error(@1, "Duplicate type definition: " + $1.identifier.name.GetString());
    }
    InternedString name($1.identifier.name);
    ast->types.insert(std::make_pair(name, std::move($1)));
    ast->constructor_to_type.emplace(name, name);
  }
  | enum_definition {
    if (ast->types.contains($1.identifier.name)) {
      throw syntax_error(@1, "Duplicate type definition: " + $1.identifier.name.GetString());
    }
    InternedString name($1.identifier.name);
    for (const auto &rule : $1.pattern_mapping) {
      for (const auto &constructor : rule.outputs) {
        ast->constructor_to_type.emplace(constructor.identifier.name, name);
      }
    }
    ast->types.insert(std::make_pair(name, std::move($1)));
  }
  | function_definition {
    if (ast->types.contains($1.identifier.name)) {
      throw syntax_error(@1, "Duplicate type definition: " + $1.identifier.name.GetString());
    }
    InternedString name($1.identifier.name);
    ast->types.insert(std::make_pair(name, std::move($1)));
    ast->constructor_to_type.emplace(name, name);
  }
  ;

%nterm <ast::Func> function_definition;
function_definition 
  : FUNC func_identifier arguments "->" "(" type_expr ")" "=" expression {
    $$ = ast::Func{{$2}, {std::move($3)}, std::make_shared<const ast::Expression>(std::move($6)), std::move($9)};
  }

%nterm <ast::FuncArguments> arguments;  
arguments
  : %empty {
    $$ = ast::FuncArguments{std::vector<ast::TypedVariable>()};
  }
  | type_dependencies {
    $$ = ast::FuncArguments{std::move($1)};
  } 
  ;

%nterm <ast::Message> message_definition;
message_definition
  : MESSAGE type_identifier type_dependencies fields_block {
    $$ = ast::Message{{$2}, {std::move($3)}, {std::move($4)}};
  }
  | MESSAGE type_identifier fields_block {
    $$ = ast::Message{{$2}, {}, {std::move($3)}};
  }
  ;

%nterm <ast::Enum> enum_definition;
enum_definition
  : dependent_enum { $$ = std::move($1); }
  | independent_enum { $$ = std::move($1); }
  ;

%nterm <ast::Enum> dependent_enum;
dependent_enum
  : ENUM type_identifier type_dependencies "{" mapping_rules "}" {
    $$ = ast::Enum{{$2}, {std::move($3)}, {std::move($5)}};
  }
  ;

%nterm <ast::Enum> independent_enum;
independent_enum
  : ENUM type_identifier constructors_block {
    std::vector<ast::Enum::Rule> one_rule;
    one_rule.emplace_back(ast::Enum::Rule{.outputs = std::move($3)});

    $$ = ast::Enum{{$2}, {}, {std::move(one_rule)}};
  }
  ;

%nterm <std::vector<ast::TypedVariable>> type_dependencies;
type_dependencies
  : "(" typed_variable ")" {
    $$ = std::vector<ast::TypedVariable>();
    $$.emplace_back(std::move($2));
  }
  | type_dependencies "(" typed_variable ")" {
    $$ = std::move($1);
    $$.emplace_back(std::move($3));
  }
  ;

%nterm <std::vector<ast::Enum::Rule>> mapping_rules;
mapping_rules
  : %empty {
    $$ = std::vector<ast::Enum::Rule>();
  }
  | mapping_rules input_patterns IMPL constructors_block {
    $$ = std::move($1);
    $$.emplace_back(ast::Enum::Rule{.inputs=std::move($2), .outputs=std::move($4)});
  }
  ;

%nterm <std::vector<ast::Enum::Rule::InputPattern>> input_patterns;
input_patterns
  : input_pattern {
    $$ = std::vector<ast::Enum::Rule::InputPattern>();
    $$.emplace_back(std::move($1));
  }
  | input_patterns "," input_pattern {
    $$ = std::move($1);
    $$.emplace_back(std::move($3));
  }
  ;

%nterm <ast::Enum::Rule::InputPattern> input_pattern;
input_pattern
  : STAR {
    $$ = ast::Star{{@1}};
  }
  | value {
    $$ = std::move($1);
  }
  ;

%nterm <std::vector<ast::Constructor>> constructors_block;
constructors_block
  : "{" constructor_declarations "}" { $$ = std::move($2); };

%nterm <std::vector<ast::Constructor>> constructor_declarations;
constructor_declarations
  : %empty {
    $$ = std::vector<ast::Constructor>();
  }
  | constructor_declarations constructor_identifier fields_block {
    $$ = std::move($1);
    $$.emplace_back(ast::Constructor{{$2}, {std::move($3)}});
  }
  | constructor_declarations constructor_identifier {
    $$ = std::move($1);
    $$.emplace_back(ast::Constructor{{$2}, {}});
  }
  ;

%nterm <std::vector<ast::TypedVariable>> fields_block;
fields_block : "{" field_declarations "}" { $$ = std::move($2); }; ;

%nterm <std::vector<ast::TypedVariable>> field_declarations;
field_declarations
  : %empty {
    $$ = std::vector<ast::TypedVariable>();
  }
  | field_declarations typed_variable SEMICOLON {
    $$ = std::move($1);
    $$.emplace_back(std::move($2));
  }
  ;

%nterm <ast::TypeExpression> type_expr;
type_expr
  : type_identifier {
    $$ = ast::TypeExpression{{@1}, {$1}};
  }
  | type_expr primary {
    $$ = std::move($1);
    $$.parameters.emplace_back(std::move($2));
  }
  | func_type_expr {
    $$ = ast::TypeExpression{{@$}, {ast::Identifier{{@$}, {InternedString("func")}}}, std::move($1)};
  }
  ;

%nterm <std::vector<ExprPtr>> func_type_expr;
func_type_expr
  : type_expr "->" "(" type_expr ")" {
    if ($1.identifier.name.GetString() == "func") {
      throw syntax_error(@1, "Expected type expression, but got function: " + $1.identifier.name.GetString());
    }
    if ($4.identifier.name.GetString() == "func") {
      $$ = std::move($4.parameters);
      $$.insert($$.begin(), std::make_shared<const ast::Expression>(std::move($1)));
    } else {
      $$ = {std::make_shared<const ast::Expression>(std::move($1)), std::make_shared<const ast::Expression>(std::move($4))};
    }
  }
  ;

%nterm <ExprPtr> expression;
expression
  : expression PLUS expression {
    $$ = std::make_shared<const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::Plus,
      std::move($1),
      std::move($3)
    });
  }
  | expression MINUS expression {
    $$ = std::make_shared<const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::Minus,
      std::move($1),
      std::move($3)
    });
  }
  | expression STAR expression {
    $$ = std::make_shared<const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::Star,
      std::move($1),
      std::move($3)
    });
  }
  | expression SLASH expression {
    $$ = std::make_shared<const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::Slash,
      std::move($1),
      std::move($3)
    });
  }
  | expression AND expression {
    $$ = std::make_shared<const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::And,
      std::move($1),
      std::move($3)
    });
  }
  | expression OR expression {
    $$ = std::make_shared< const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::Or,
      std::move($1),
      std::move($3)
    });
  }
  | expression DOUBLE_AND expression {
    $$ = std::make_shared< const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::DoubleAnd,
      std::move($1),
      std::move($3)
    });
  }
  | expression DOUBLE_OR expression {
    $$ = std::make_shared< const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::DoubleOr,
      std::move($1),
      std::move($3)
    });
  }
  | expression BACK_SLASH expression {
    $$ = std::make_shared< const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::BackSlash,
      std::move($1),
      std::move($3)
    });
  }
  | expression IN expression {
    $$ = std::make_shared< const ast::Expression>(ast::BinaryExpression{
      {location(@$)},
      ast::BinaryExpressionType::In,
      std::move($1),
      std::move($3)
    });
  }
  | MINUS expression {
    $$ = std::make_shared<const ast::Expression>(ast::UnaryExpression{
      {location(@$)},
      ast::UnaryExpressionType::Minus,
      std::move($2)
    });
  }
  | BANG expression {
    $$ = std::make_shared<const ast::Expression>(ast::UnaryExpression{
      {location(@$)},
      ast::UnaryExpressionType::Bang,
      std::move($2)
    });
  }
  | type_expr {
    $$ = std::make_shared<const ast::Expression>(std::move($1));
  }
  | primary {
    $$ = std::move($1);
  }
  ;

%nterm <ExprPtr> primary;
primary
  : value {
    $$ = std::make_shared<const ast::Expression>(std::move($1));
  }
  | var_access {
    $$ = std::make_shared<const ast::Expression>(std::move($1));
  }
  | array_access {
    $$ = std::make_shared<const ast::Expression>(std::move($1));
  }
  | "(" expression ")" {
    $$ = std::move($2);
  }
  ;

%nterm <ast::VarAccess> var_access;
var_access
  : var_identifier {
    $$ = ast::VarAccess{{$1}};
  }
  | var_access "." var_identifier {
    $$ = std::move($1);
    $$.field_identifiers.emplace_back($3);
  }
  ;

%nterm <ast::ArrayAccess> array_access;
array_access
  : primary "[" expression "]" {
    $$ = ast::ArrayAccess{std::move($1), std::move($3)};
  }
  ;

%nterm <ast::Value> value;
value
  : bool_literal { $$ = std::move($1); }
  | float_literal { $$ = std::move($1); }
  | int_literal { $$ = std::move($1); }
  | uint_literal { $$ = std::move($1); }
  | string_literal { $$ = std::move($1); }
  | constructed_value { $$ = std::move($1); }
  | collection_value { $$ = std::move($1); }
  | function_value { $$ = std::move($1); }
  ;

%nterm <ast::Value> bool_literal;
bool_literal
  : FALSE { $$ = ast::Value(ast::ScalarValue<bool>{{@1}, false}); }
  | TRUE { $$ = ast::Value(ast::ScalarValue<bool>{{@1}, true}); }
  ;

%nterm <ast::Value> float_literal;
float_literal : FLOAT_LITERAL { $$ = ast::Value(ast::ScalarValue<double>{{@1}, $1}); };

%nterm <ast::Value> int_literal;
int_literal : INT_LITERAL { $$ = ast::Value(ast::ScalarValue<int64_t>{{@1}, $1}); };

%nterm <ast::Value> uint_literal;
uint_literal : UINT_LITERAL { $$ = ast::Value(ast::ScalarValue<uint64_t>{{@1}, $1}); };

%nterm <ast::Value> string_literal;
string_literal : STRING_LITERAL { $$ = ast::Value(ast::ScalarValue<std::string>{{@1}, $1}); };

%nterm <ast::FunctionValue> function_value;
function_value
  : "(" func_identifier "(" parameters ")" ")" {
    $$ = ast::FunctionValue{{@$}, $2, std::move($4)};
  }
  ;

%nterm <ast::Value> collection_value;
collection_value
  : "<" parameters ">" {
    $$ = ast::CollectionValue{{@$}, std::move($2)};
  }
  ;

%nterm <std::vector<ExprPtr>> parameters;
parameters 
  : %empty {
    $$ = std::vector<ExprPtr>();
  } 
  | parameters_initialization_list {
    $$ = std::move($1);
  }
  ;

%nterm <std::vector<ExprPtr>> parameters_initialization_list;
parameters_initialization_list
  : expression {
    $$.emplace_back(std::move($1));
  }
  | parameters_initialization_list "," expression {
    $$ = std::move($1);
    $$.emplace_back(std::move($3));
  }
  ;

%nterm <ast::Value> constructed_value;
constructed_value
  : constructor_identifier "{" field_initialization "}" {
    $$ = ast::ConstructedValue{{@$}, $1, std::move($3)};
  }
  ;

%nterm <std::vector<std::pair<ast::Identifier, ExprPtr>>> field_initialization;
field_initialization
  : %empty {
    $$ = std::vector<std::pair<ast::Identifier, ExprPtr>>();
  }
  | field_initialization_list {
    $$ = std::move($1);
  }
  ;

%nterm <std::vector<std::pair<ast::Identifier, ExprPtr>>> field_initialization_list;
field_initialization_list
  : var_identifier COLON expression {
    $$.emplace_back($1, std::move($3));
  }
  | field_initialization "," var_identifier COLON expression {
    $$ = std::move($1);
    $$.emplace_back($3, std::move($5));
  }
  ;

%nterm <ast::Identifier>
  type_identifier
  constructor_identifier
  var_identifier
  func_identifier
;

type_identifier : UC_IDENTIFIER { $$ = ast::Identifier{{@1}, {InternedString(std::move($1))}}; };
constructor_identifier : UC_IDENTIFIER { $$ = ast::Identifier{{@1}, {InternedString(std::move($1))}}; };
var_identifier : LC_IDENTIFIER { $$ = ast::Identifier{{@1}, {InternedString(std::move($1))}}; };
func_identifier : LC_IDENTIFIER { $$ = ast::Identifier{{@1}, {InternedString(std::move($1))}}; };

%nterm <ast::TypedVariable> typed_variable;
typed_variable
  : var_identifier type_expr {
    $$ = ast::TypedVariable{{$1}, std::move($2)};
  }
  ;

%%

void dbuf::parser::Parser::error(const location_type &l, const std::string &err_message)
{
   std::cerr << "Error: " << err_message << " at " << l << std::endl;
}
