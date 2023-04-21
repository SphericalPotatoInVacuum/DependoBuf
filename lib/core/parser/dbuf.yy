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

  namespace dbuf {
    class Driver;
  }

  using namespace dbuf::ast;

  namespace dbuf::parser {
    class Lexer;

    using ExprPtr = std::unique_ptr<Expression>;
  }
}

%parse-param { Lexer &scanner }
%parse-param { Driver &driver }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
  #include "core/driver.h"

  #include "core/ast/ast.h"
  #include "core/ast/expression.h"

  #include <iostream>
  #include <cstdlib>
  #include <fstream>

#undef yylex
#define yylex scanner.yylex
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

schema : definitions { driver.saveAst(std::move($1)); }

%nterm <AST> definitions;
definitions
  : %empty { $$ = AST{}; }
  | definitions message_definition {
    $$ = std::move($1);
    $$.AddMessage(std::move($2));
  }
  | definitions enum_definition {
    $$ = std::move($1);
    $$.AddEnum(std::move($2));
  }
  | definitions service_definition {
    $$ = std::move($1);
  }
  | definitions NL {
    $$ = std::move($1);
  }
  ;

%nterm <Message> message_definition;
message_definition
  : MESSAGE type_identifier type_dependencies fields_block {
    $$ = Message{.name_ = $2, .type_dependencies_ = std::move($3), .fields_ = std::move($4)};
  }
  | MESSAGE type_identifier fields_block {
    $$ = Message{.name_ = $2, .fields_ = std::move($3)};
  }
  ;

%nterm <Enum> enum_definition;
enum_definition
  : dependent_enum { $$ = std::move($1); }
  | independent_enum { $$ = std::move($1); }
  ;

%nterm <Enum> dependent_enum;
dependent_enum
  : ENUM type_identifier type_dependencies dependent_enum_body {
    $$ = std::move($4);
    $$.name_ = $2;
    $$.type_dependencies_ = std::move($3);
  }
  ;

%nterm <Enum> independent_enum;
independent_enum
  : ENUM type_identifier independent_enum_body {
    $$ = Enum{.name_ = $2};
  }
  ;

%nterm <Enum> independent_enum_body;
independent_enum_body
  : constructors_block {
    $$ = Enum{};
    $$.outputs_.emplace_back(std::move($1));
  }

%nterm <std::vector<TypedVariable>> type_dependencies;
type_dependencies
  : type_dependency {
    $$ = std::vector<TypedVariable>();
    $$.emplace_back(std::move($1));
  }
  | type_dependencies type_dependency {
    $$ = std::move($1);
    $$.emplace_back(std::move($2));
  }
  ;

%nterm <TypedVariable> type_dependency;
type_dependency
  : "(" typed_variable ")" {
    $$ = std::move($2);
  }
  ;

%nterm <Enum> dependent_enum_body;
dependent_enum_body : "{" NL dependent_blocks "}" NL { $$ = std::move($3); };

%nterm <Enum> dependent_blocks;
dependent_blocks
  : %empty {
    $$ = Enum{};
  }
  | dependent_blocks pattern_matching IMPL constructors_block {
    $$ = std::move($1);
    $$.inputs_.emplace_back(std::move($2));
    $$.outputs_.emplace_back(std::move($4));
  }
  ;

%nterm <std::vector<std::variant<Value, StarValue>>> pattern_matching;
pattern_matching
  : pattern_match {
    $$ = std::vector<std::variant<Value, StarValue>>();
    $$.emplace_back(std::move($1));
  }
  | pattern_matching "," pattern_match {
    $$ = std::move($1);
    $$.emplace_back(std::move($3));
  }
  ;

%nterm <std::variant<Value, StarValue>> pattern_match;
pattern_match
  : STAR {
    $$ = StarValue{};
  }
  | value {
    $$ = std::move($1);
  }
  ;

%nterm <std::vector<Constructor>> constructors_block;
constructors_block
  : "{" NL constructor_declarations "}" NL { $$ = std::move($3); };

%nterm <std::vector<Constructor>> constructor_declarations;
constructor_declarations
  : %empty {
    $$ = std::vector<Constructor>();
  }
  | constructor_declarations constructor_identifier fields_block {
    $$ = std::move($1);
    $$.emplace_back(Constructor{.name_ = $2, .fields_ = std::move($3)});
  }
  | constructor_declarations constructor_identifier NL {
    $$ = std::move($1);
    $$.emplace_back(Constructor{.name_ = $2});
  }
  ;

%nterm <std::vector<TypedVariable>> fields_block;
fields_block : "{" NL field_declarations "}" NL { $$ = std::move($3); }; ;

%nterm <std::vector<TypedVariable>> field_declarations;
field_declarations
  : %empty {
    $$ = std::vector<TypedVariable>();
  }
  | field_declarations typed_variable NL {
    $$ = std::move($1);
    $$.emplace_back(std::move($2));
  }
  ;

%nterm <TypeExpression> type_expr;
type_expr
  : type_identifier {
    $$ = TypeExpression{.type_name_ = $1};
  }
  | type_expr primary {
    $$ = std::move($1);
    $$.type_parameters_.emplace_back(std::move($2));
  }
  ;

%nterm <ExprPtr> expression;
expression
  : expression PLUS expression {
    $$ = std::make_unique<Expression>(BinaryExpression{
      .left_=std::move($1),
      .type_=BinaryExpressionType::kPlus,
      .right_=std::move($3)
    });
  }
  | expression MINUS expression {
    $$ = std::make_unique<Expression>(BinaryExpression{
      .left_=std::move($1),
      .type_=BinaryExpressionType::kMinus,
      .right_=std::move($3)
    });
  }
  | expression STAR expression {
    $$ = std::make_unique<Expression>(BinaryExpression{
      .left_=std::move($1),
      .type_=BinaryExpressionType::kStar,
      .right_=std::move($3)
    });
  }
  | expression SLASH expression {
    $$ = std::make_unique<Expression>(BinaryExpression{
      .left_=std::move($1),
      .type_=BinaryExpressionType::kSlash,
      .right_=std::move($3)
    });
  }
  | expression AND expression {
    $$ = std::make_unique<Expression>(BinaryExpression{
      .left_=std::move($1),
      .type_=BinaryExpressionType::kAnd,
      .right_=std::move($3)
    });
  }
  | expression OR expression {
    $$ = std::make_unique<Expression>(BinaryExpression{
      .left_=std::move($1),
      .type_=BinaryExpressionType::kOr,
      .right_=std::move($3)
    });
  }
  | MINUS expression {
    $$ = std::make_unique<Expression>(UnaryExpression{
      .type_=UnaryExpressionType::kMinus,
      .expression_=std::move($2)
    });
  }
  | BANG expression {
    $$ = std::make_unique<Expression>(UnaryExpression{
      .type_=UnaryExpressionType::kBang,
      .expression_=std::move($2)
    });
  }
  | type_expr {
    $$ = std::make_unique<Expression>(std::move($1));
  }
  | primary {
    $$ = std::move($1);
  }
  ;

%nterm <ExprPtr> primary;
primary
  : value {
    $$ = std::make_unique<Expression>(std::move($1));
  }
  | var_access {
    $$ = std::make_unique<Expression>(std::move($1));
  }
  | "(" expression ")" {
    $$ = std::move($2);
  }
  ;

%nterm <VarAccess> var_access;
var_access
  : var_identifier {
    $$ = VarAccess{.var_identifier_ = $1};
  }
  | var_access "." var_identifier {
    $$ = std::move($1);
    $$.field_identifiers_.emplace_back($3);
  }
  ;

%nterm <Value> value;
value
  : bool_literal { $$ = std::move($1); }
  | float_literal { $$ = std::move($1); }
  | int_literal { $$ = std::move($1); }
  | uint_literal { $$ = std::move($1); }
  | string_literal { $$ = std::move($1); }
  | constructed_value { $$ = std::move($1); }
  ;

%nterm <Value> bool_literal;
bool_literal
  : FALSE { $$ = Value(ScalarValue<bool>{false}); }
  | TRUE { $$ = Value(ScalarValue<bool>{true}); }
  ;

%nterm <Value> float_literal;
float_literal : FLOAT_LITERAL { $$ = Value(ScalarValue<double>{$1}); } ;

%nterm <Value> int_literal;
int_literal : INT_LITERAL { $$ = Value(ScalarValue<int64_t>{$1}); };

%nterm <Value> uint_literal;
uint_literal : UINT_LITERAL { $$ = Value(ScalarValue<uint64_t>{$1}); };

%nterm <Value> string_literal;
string_literal : STRING_LITERAL { $$ = Value(ScalarValue<std::string>{$1}); };

%nterm <Value> constructed_value;
constructed_value
  : constructor_identifier "{" field_initialization "}" {
    $$ = ConstructedValue {.constructor_identifier_ = $1, .fields_ = std::move($3)};
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
type_identifier : UC_IDENTIFIER { $$ = driver.GetInterning(std::move($1)); };
constructor_identifier : UC_IDENTIFIER { $$ = driver.GetInterning(std::move($1)); };
service_identifier : UC_IDENTIFIER { $$ = driver.GetInterning(std::move($1)); };
var_identifier : LC_IDENTIFIER { $$ = driver.GetInterning(std::move($1)); };
rpc_identifier : LC_IDENTIFIER { $$ = driver.GetInterning(std::move($1)); };

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

%nterm <TypedVariable> typed_variable;
typed_variable
  : var_identifier type_expr {
    $$ = TypedVariable{.name_ = $1, .type_expression_= std::move($2)};
  }

%%

void dbuf::parser::Parser::error(const location_type &l, const std::string &err_message)
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
