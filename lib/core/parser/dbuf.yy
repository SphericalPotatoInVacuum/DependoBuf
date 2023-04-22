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
    ast->AddMessage(std::move($2));
  }
  | schema enum_definition {
    ast->AddEnum(std::move($2));
  }
  | schema service_definition
  | schema NL
  ;

%nterm <ast::Message> message_definition;
message_definition
  : MESSAGE type_identifier type_dependencies fields_block {
    $$ = ast::Message{.name_ = $2, .type_dependencies_ = std::move($3), .fields_ = std::move($4)};
  }
  | MESSAGE type_identifier fields_block {
    $$ = ast::Message{.name_ = $2, .fields_ = std::move($3)};
  }
  ;

%nterm <ast::Enum> enum_definition;
enum_definition
  : dependent_enum { $$ = std::move($1); }
  | independent_enum { $$ = std::move($1); }
  ;

%nterm <ast::Enum> dependent_enum;
dependent_enum
  : ENUM type_identifier type_dependencies dependent_enum_body {
    $$ = std::move($4);
    $$.name_ = $2;
    $$.type_dependencies_ = std::move($3);
  }
  ;

%nterm <ast::Enum> independent_enum;
independent_enum
  : ENUM type_identifier independent_enum_body {
    $$ = ast::Enum{.name_ = $2};
  }
  ;

%nterm <ast::Enum> independent_enum_body;
independent_enum_body
  : constructors_block {
    $$ = ast::Enum{};
    $$.outputs_.emplace_back(std::move($1));
  }

%nterm <std::vector<ast::TypedVariable>> type_dependencies;
type_dependencies
  : type_dependency {
    $$ = std::vector<ast::TypedVariable>();
    $$.emplace_back(std::move($1));
  }
  | type_dependencies type_dependency {
    $$ = std::move($1);
    $$.emplace_back(std::move($2));
  }
  ;

%nterm <ast::TypedVariable> type_dependency;
type_dependency
  : "(" typed_variable ")" {
    $$ = std::move($2);
  }
  ;

%nterm <ast::Enum> dependent_enum_body;
dependent_enum_body : "{" NL dependent_blocks "}" NL { $$ = std::move($3); };

%nterm <ast::Enum> dependent_blocks;
dependent_blocks
  : %empty {
    $$ = ast::Enum{};
  }
  | dependent_blocks pattern_matching IMPL constructors_block {
    $$ = std::move($1);
    $$.inputs_.emplace_back(std::move($2));
    $$.outputs_.emplace_back(std::move($4));
  }
  ;

%nterm <std::vector<std::variant<ast::Value, ast::StarValue>>> pattern_matching;
pattern_matching
  : pattern_match {
    $$ = std::vector<std::variant<ast::Value, ast::StarValue>>();
    $$.emplace_back(std::move($1));
  }
  | pattern_matching "," pattern_match {
    $$ = std::move($1);
    $$.emplace_back(std::move($3));
  }
  ;

%nterm <std::variant<ast::Value, ast::StarValue>> pattern_match;
pattern_match
  : STAR {
    $$ = ast::StarValue{};
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
    $$.emplace_back(ast::Constructor{.name_ = $2, .fields_ = std::move($3)});
  }
  | constructor_declarations constructor_identifier NL {
    $$ = std::move($1);
    $$.emplace_back(ast::Constructor{.name_ = $2});
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
    $$ = ast::TypeExpression{.type_name_ = $1};
  }
  | type_expr primary {
    $$ = std::move($1);
    $$.type_parameters_.emplace_back(std::move($2));
  }
  ;

%nterm <ExprPtr> expression;
expression
  : expression PLUS expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .left_=std::move($1),
      .type_=ast::BinaryExpressionType::kPlus,
      .right_=std::move($3)
    });
  }
  | expression MINUS expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .left_=std::move($1),
      .type_=ast::BinaryExpressionType::kMinus,
      .right_=std::move($3)
    });
  }
  | expression STAR expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .left_=std::move($1),
      .type_=ast::BinaryExpressionType::kStar,
      .right_=std::move($3)
    });
  }
  | expression SLASH expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .left_=std::move($1),
      .type_=ast::BinaryExpressionType::kSlash,
      .right_=std::move($3)
    });
  }
  | expression AND expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .left_=std::move($1),
      .type_=ast::BinaryExpressionType::kAnd,
      .right_=std::move($3)
    });
  }
  | expression OR expression {
    $$ = std::make_unique<ast::Expression>(ast::BinaryExpression{
      .left_=std::move($1),
      .type_=ast::BinaryExpressionType::kOr,
      .right_=std::move($3)
    });
  }
  | MINUS expression {
    $$ = std::make_unique<ast::Expression>(ast::UnaryExpression{
      .type_=ast::UnaryExpressionType::kMinus,
      .expression_=std::move($2)
    });
  }
  | BANG expression {
    $$ = std::make_unique<ast::Expression>(ast::UnaryExpression{
      .type_=ast::UnaryExpressionType::kBang,
      .expression_=std::move($2)
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
    $$ = ast::VarAccess{.var_identifier_ = $1};
  }
  | var_access "." var_identifier {
    $$ = std::move($1);
    $$.field_identifiers_.emplace_back($3);
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
    $$ = ast::ConstructedValue{.constructor_identifier_ = $1, .fields_ = std::move($3)};
  }
  ;

%nterm <std::vector<std::pair<uint64_t, ExprPtr>>> field_initialization;
field_initialization
  : %empty {
    $$ = std::vector<std::pair<uint64_t, ExprPtr>>();
  }
  | field_initialization_list {
    $$ = std::move($1);
  }
  ;

%nterm <std::vector<std::pair<uint64_t, ExprPtr>>> field_initialization_list;
field_initialization_list
  : var_identifier COLON expression {
    $$ = std::vector<std::pair<uint64_t, ExprPtr>>();
    $$.emplace_back($1, std::move($3));
  }
  | field_initialization "," var_identifier COLON expression {
    $$ = std::move($1);
    $$.emplace_back($3, std::move($5));
  }

%nterm <uint64_t>
  type_identifier
  constructor_identifier
  service_identifier
  var_identifier
  rpc_identifier
;
type_identifier : UC_IDENTIFIER { $$ = ast->GetInterning(std::move($1)); };
constructor_identifier : UC_IDENTIFIER { $$ = ast->GetInterning(std::move($1)); };
service_identifier : UC_IDENTIFIER { $$ = ast->GetInterning(std::move($1)); };
var_identifier : LC_IDENTIFIER { $$ = ast->GetInterning(std::move($1)); };
rpc_identifier : LC_IDENTIFIER { $$ = ast->GetInterning(std::move($1)); };

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
    $$ = ast::TypedVariable{.name_ = $1, .type_expression_= std::move($2)};
  }

%%

void dbuf::parser::Parser::error(const location_type &l, const std::string &err_message)
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
