%skeleton "lalr1.cc"
%require  "3.4"
%header

%defines

%define api.token.raw
%define api.value.type variant

%define api.namespace {dbuf::parser}
%define api.parser.class {Parser}

%code requires{
  #include <parser/ast.h>
  #include <parser/expression.h>

  namespace dbuf::parser {
    class Driver;
    class Lexer;
  }

}

%parse-param { Lexer &scanner }
%parse-param { Driver &driver }

%locations

%define parse.trace
%define parse.error detailed
%define parse.lac full

%code {
  #include <iostream>
  #include <cstdlib>
  #include <fstream>

  #include <parser/driver.hpp>
  #include <parser/ast.h>
  #include <parser/expression.h>

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
  BANG_EQUAL "!="
  GREATER_EQUAL ">="
  LESS_EQUAL "<="
  AND "&&"
  OR "||"
  LESS "<"
  EQUAL "="
  GREATER ">"
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
%token <long long> INT_LITERAL
%token <std::string> STRING_LITERAL

%right ":"
%left "!=" ">=" "<=" "<" "=" ">"
%left "||" "-" "+"
%left "&&" "*" "/"
%right "!"

%start schema

%%

schema : definitions { driver.saveAst(std::move($1)); }

%nterm <AST> definitions;
definitions
  : %empty { $$ = std::move(AST()); }
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
  ;

%nterm <Message> message_definition;
message_definition
  : MESSAGE type_identifier type_dependencies fields_block {
    $$ = Message{.name_=std::move($2)};
    for (auto &type_dependency : $3) {
      $$.AddDependency(std::move(type_dependency));
    }
    for (auto &field : $4) {
      $$.AddField(std::move(field));
    }
  }
  | MESSAGE type_identifier fields_block {
    $$ = Message{.name_=std::move($2)};
    for (auto &field : $3) {
      $$.AddField(std::move(field));
    }
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
    for (auto &type_dependency : $3) {
      $$.AddDependency(std::move(type_dependency));
    }
  }
  ;

%nterm <Enum> independent_enum;
independent_enum
  : ENUM type_identifier independent_enum_body {
    $$ = Enum{.name_=std::move($2)};
  }
  ;

%nterm <Enum> independent_enum_body;
independent_enum_body
  : constructors_block {
    $$ = Enum{};
    $$.AddOutput(std::move($1));
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
    $$ = Enum();
  }
  | dependent_blocks pattern_matching IMPL constructors_block {
    $$ = std::move($1);
    $$.AddInput(std::move($2));
    $$.AddOutput(std::move($4));
  }
  ;

%nterm <std::vector<std::unique_ptr<std::variant<Value, StarValue>>>> pattern_matching;
pattern_matching
  : pattern_match {
    $$ = std::vector<std::unique_ptr<std::variant<Value, StarValue>>>();
    $$.emplace_back(std::move($1));
  }
  | pattern_matching "," pattern_match {
    $$ = std::move($1);
    $$.emplace_back(std::move($3));
  }
  ;

%nterm <std::unique_ptr<std::variant<Value, StarValue>>> pattern_match;
pattern_match
  : STAR {
    $$ = std::make_unique<std::variant<Value, StarValue>>(StarValue{});
  }
  | value {
    $$ = std::make_unique<std::variant<Value, StarValue>>(std::move(*$1));
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
    $$.emplace_back(Constructor{.name_=std::move($2)});
    for (auto &field : $3) {
      $$.back().AddField(std::move(field));
    }
  }
  | constructor_declarations constructor_identifier NL {
    $$ = std::move($1);
    $$.emplace_back(Constructor{.name_=std::move($2)});
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
    $$ = TypeExpression(std::move($1));
  }
  | type_expr primary {
    $$ = std::move($1);
    $$.type_parameters_.push_back(std::move($2));
  }
  ;

%nterm <std::unique_ptr<Expression>> expression;
expression
  : expression PLUS expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kPlus, std::move($3));
  }
  | expression MINUS expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kMinus, std::move($3));
  }
  | expression STAR expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kStar, std::move($3));
  }
  | expression SLASH expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kSlash, std::move($3));
  }
  | expression BANG_EQUAL expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kBangEqual, std::move($3));
  }
  | expression GREATER_EQUAL expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kGreaterEqual, std::move($3));
  }
  | expression LESS_EQUAL expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kLessEqual, std::move($3));
  }
  | expression AND expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kAnd, std::move($3));
  }
  | expression OR expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kOr, std::move($3));
  }
  | expression LESS expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kLess, std::move($3));
  }
  | expression EQUAL expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kEqual, std::move($3));
  }
  | expression GREATER expression {
    $$ = std::make_unique<BinaryExpression>(std::move($1), BinaryExpressionType::kGreater, std::move($3));
  }
  | type_expr {
    $$ = std::make_unique<TypeExpression>(std::move($1));
  }
  | MINUS expression {
    $$ = std::make_unique<UnaryExpression>(UnaryExpressionType::kMinus, std::move($2));
  }
  | BANG expression {
    $$ = std::make_unique<UnaryExpression>(UnaryExpressionType::kBang, std::move($2));
  }
  | primary {
    $$ = std::move($1);
  }
  ;

%nterm <std::unique_ptr<Expression>> primary;
primary
  : value {
    $$ = std::move($1);
  }
  | var_access {
    $$ = std::move($1);
  }
  | "(" expression ")" {
    $$ = std::move($2);
  }
  ;

%nterm <std::unique_ptr<VarAccess>> var_access;
var_access
  : var_identifier {
    $$ = std::make_unique<VarAccess>($1);
  }
  | var_access "." var_identifier {
    $1->field_identifiers.push_back($3);
    $$ = std::move($1);
  }
  ;

%nterm <std::unique_ptr<Value>> value;
value
  : bool_literal { $$ = std::move($1); }
  | float_literal { $$ = std::move($1); }
  | int_literal { $$ = std::move($1); }
  | string_literal { $$ = std::move($1); }
  | constructed_value { $$ = std::move($1); }
  ;

%nterm <std::unique_ptr<BoolValue>> bool_literal;
bool_literal
  : FALSE { $$ = std::make_unique<BoolValue>(false); }
  | TRUE { $$ = std::make_unique<BoolValue>(true); }
  ;

%nterm <std::unique_ptr<FloatValue>> float_literal;
float_literal : FLOAT_LITERAL { $$ = std::make_unique<FloatValue>($1); } ;

%nterm <std::unique_ptr<IntValue>> int_literal;
int_literal : INT_LITERAL { $$ = std::make_unique<IntValue>($1); };

%nterm <std::unique_ptr<StringValue>> string_literal;
string_literal : STRING_LITERAL { $$ = std::make_unique<StringValue>(std::move($1)); };

%nterm <std::unique_ptr<ConstructedValue>> constructed_value;
constructed_value
  : constructor_identifier "{" field_initialization "}" {
    $$ = std::make_unique<ConstructedValue>(std::move($1), std::move($3));
  }
  ;
%nterm <FieldInitialization> field_initialization;
field_initialization
  : %empty {
    $$ = FieldInitialization();
  }
  | var_identifier COLON expression {
    $$ = FieldInitialization();
    $$.AddField($1, std::move($3));
  }
  | field_initialization "," var_identifier COLON expression {
    $$ = std::move($1);
    $$.AddField($3, std::move($5));
  }
  ;

%nterm <std::string>
  type_identifier
  constructor_identifier
  service_identifier
  var_identifier
  rpc_identifier
;
type_identifier : UC_IDENTIFIER { $$ = std::move($1); };
constructor_identifier : UC_IDENTIFIER { $$ = std::move($1); };
service_identifier : UC_IDENTIFIER { $$ = std::move($1); };
var_identifier : LC_IDENTIFIER { $$ = std::move($1); };
rpc_identifier : LC_IDENTIFIER { $$ = std::move($1); };

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
    $$ = TypedVariable{.name_=std::move($1), .type_expression_=std::move($2)};
  }

%%

void dbuf::parser::Parser::error(const location_type &l, const std::string &err_message)
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
