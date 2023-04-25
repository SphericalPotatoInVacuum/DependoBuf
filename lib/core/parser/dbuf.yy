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

    using ExprPtr = std::unique_ptr<ast::Expression>;
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
%token NL
%token <std::string> LC_IDENTIFIER UC_IDENTIFIER
%token MESSAGE ENUM IMPL SERVICE RPC RETURNS
%token FALSE TRUE
%token
  PLUS "+"
  MINUS "-"
  STAR "*"
  SLASH "/"
;
%token
  AND "&&"
  OR "||"
  BANG "!"
;
%token
  LEFT_PAREN "("
  RIGHT_PAREN ")"
  LEFT_BRACE "{"
  RIGHT_BRACE "}"
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

%left "||" "-" "+"
%left "&&" "*" "/"
%precedence "!"

%start schema

%%

schema
  : %empty
  | schema message_definition {
    InternedString name($2.name);
    ast->messages.insert(std::make_pair(name, std::move($2)));
  }
  | schema enum_definition {
    InternedString name($2.name);
    ast->enums.insert(std::make_pair(name, std::move($2)));
  }
  | schema service_definition
  | schema NL
  ;

%nterm <ast::Message> message_definition;
message_definition
  : MESSAGE type_identifier type_dependencies fields_block {
    $$ = ast::Message{.name = $2, .type_dependencies = std::move($3), .fields = std::move($4)};
  }
  | MESSAGE type_identifier fields_block {
    $$ = ast::Message{.name = $2, .fields = std::move($3)};
  }
  ;

%nterm <ast::Enum> enum_definition;
enum_definition
  : dependent_enum { $$ = std::move($1); }
  | independent_enum { $$ = std::move($1); }
  ;

%nterm <ast::Enum> dependent_enum;
dependent_enum
  : ENUM type_identifier type_dependencies "{" NL mapping_rules "}" NL {
    $$ = ast::Enum{.name = $2, .type_dependencies = std::move($3), .pattern_mapping = std::move($6)};
  }
  ;

%nterm <ast::Enum> independent_enum;
independent_enum
  : ENUM type_identifier constructors_block {
    std::vector<ast::Enum::Rule> one_rule;
    one_rule.emplace_back(ast::Enum::Rule{.outputs = std::move($3)});

    $$ = ast::Enum{.name = $2, .pattern_mapping = std::move(one_rule)};
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
    $$ = ast::Star{};
  }
  | value {
    $$ = std::move($1);
  }
  ;

%nterm <std::vector<ast::Constructor>> constructors_block;
constructors_block
  : "{" NL constructor_declarations "}" NL { $$ = std::move($3); };

%nterm <std::vector<ast::Constructor>> constructor_declarations;
constructor_declarations
  : %empty {
    $$ = std::vector<ast::Constructor>();
  }
  | constructor_declarations constructor_identifier fields_block {
    $$ = std::move($1);
    $$.emplace_back(ast::Constructor{.name = $2, .fields = std::move($3)});
  }
  | constructor_declarations constructor_identifier NL {
    $$ = std::move($1);
    $$.emplace_back(ast::Constructor{.name = $2});
  }
  ;

%nterm <std::vector<ast::TypedVariable>> fields_block;
fields_block : "{" NL field_declarations "}" NL { $$ = std::move($3); }; ;

%nterm <std::vector<ast::TypedVariable>> field_declarations;
field_declarations
  : %empty {
    $$ = std::vector<ast::TypedVariable>();
  }
  | field_declarations typed_variable NL {
    $$ = std::move($1);
    $$.emplace_back(std::move($2));
  }
  ;

%nterm <ast::TypeExpression> type_expr;
type_expr
  : type_identifier {
    $$ = ast::TypeExpression{.name = $1};
  }
  | type_expr primary {
    $$ = std::move($1);
    $$.parameters.emplace_back(std::move($2));
  }
  ;

%nterm <ExprPtr> expression;
expression
  : expression PLUS expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .type=ast::BinaryExpressionType::Plus,
      .left=std::move($1),
      .right=std::move($3)
    });
  }
  | expression MINUS expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .type=ast::BinaryExpressionType::Minus,
      .left=std::move($1),
      .right=std::move($3)
    });
  }
  | expression STAR expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .type=ast::BinaryExpressionType::Star,
      .left=std::move($1),
      .right=std::move($3)
    });
  }
  | expression SLASH expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .type=ast::BinaryExpressionType::Slash,
      .left=std::move($1),
      .right=std::move($3)
    });
  }
  | expression AND expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .type=ast::BinaryExpressionType::And,
      .left=std::move($1),
      .right=std::move($3)
    });
  }
  | expression OR expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .type=ast::BinaryExpressionType::Or,
      .left=std::move($1),
      .right=std::move($3)
    });
  }
  | MINUS expression {
    $$ = std::make_unique<ast::Expression>(ast::UnaryExpression{
      .type=ast::UnaryExpressionType::Minus,
      .expression=std::move($2)
    });
  }
  | BANG expression {
    $$ = std::make_unique<ast::Expression>(ast::UnaryExpression{
      .type=ast::UnaryExpressionType::Bang,
      .expression=std::move($2)
    });
  }
  | type_expr {
    $$ = std::make_unique<ast::Expression>(std::move($1));
  }
  | primary {
    $$ = std::move($1);
  }
  ;

%nterm <ExprPtr> primary;
primary
  : value {
    $$ = std::make_unique<ast::Expression>(std::move($1));
  }
  | var_access {
    $$ = std::make_unique<ast::Expression>(std::move($1));
  }
  | "(" expression ")" {
    $$ = std::move($2);
  }
  ;

%nterm <ast::VarAccess> var_access;
var_access
  : var_identifier {
    $$ = ast::VarAccess{.var_identifier = $1};
  }
  | var_access "." var_identifier {
    $$ = std::move($1);
    $$.field_identifiers.emplace_back($3);
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
  ;

%nterm <ast::Value> bool_literal;
bool_literal
  : FALSE { $$ = ast::Value(ast::ScalarValue<bool>{false}); }
  | TRUE { $$ = ast::Value(ast::ScalarValue<bool>{true}); }
  ;

%nterm <ast::Value> float_literal;
float_literal : FLOAT_LITERAL { $$ = ast::Value(ast::ScalarValue<double>{$1}); } ;

%nterm <ast::Value> int_literal;
int_literal : INT_LITERAL { $$ = ast::Value(ast::ScalarValue<int64_t>{$1}); };

%nterm <ast::Value> uint_literal;
uint_literal : UINT_LITERAL { $$ = ast::Value(ast::ScalarValue<uint64_t>{$1}); };

%nterm <ast::Value> string_literal;
string_literal : STRING_LITERAL { $$ = ast::Value(ast::ScalarValue<std::string>{$1}); };

%nterm <ast::Value> constructed_value;
constructed_value
  : constructor_identifier "{" field_initialization "}" {
    $$ = ast::ConstructedValue{.constructor_identifier = $1, .fields = std::move($3)};
  }
  ;

%nterm <std::vector<std::pair<InternedString, ExprPtr>>> field_initialization;
field_initialization
  : %empty {
    $$ = std::vector<std::pair<InternedString, ExprPtr>>();
  }
  | field_initialization_list {
    $$ = std::move($1);
  }
  ;

%nterm <std::vector<std::pair<InternedString, ExprPtr>>> field_initialization_list;
field_initialization_list
  : var_identifier COLON expression {
    $$.emplace_back($1, std::move($3));
  }
  | field_initialization "," var_identifier COLON expression {
    $$ = std::move($1);
    $$.emplace_back($3, std::move($5));
  }

%nterm <InternedString>
  type_identifier
  constructor_identifier
  service_identifier
  var_identifier
  rpc_identifier
;
type_identifier : UC_IDENTIFIER { $$ = InternedString(std::move($1)); };
constructor_identifier : UC_IDENTIFIER { $$ = InternedString(std::move($1)); };
service_identifier : UC_IDENTIFIER { $$ = InternedString(std::move($1)); };
var_identifier : LC_IDENTIFIER { $$ = InternedString(std::move($1)); };
rpc_identifier : LC_IDENTIFIER { $$ = InternedString(std::move($1)); };

service_definition
  : SERVICE service_identifier rpc_block
  ;
rpc_block
  : "{" NL rpc_declarations "}" NL ;
rpc_declarations
  : %empty
  | RPC rpc_identifier "(" arguments ")"
    RETURNS "(" type_expr ")" NL
  ;
arguments
  : %empty
  | typed_variable
  | typed_variable "," arguments
  ;

%nterm <ast::TypedVariable> typed_variable;
typed_variable
  : var_identifier type_expr {
    $$ = ast::TypedVariable{.name = $1, .type_expression= std::move($2)};
  }

%%

void dbuf::parser::Parser::error(const location_type &l, const std::string &err_message)
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
