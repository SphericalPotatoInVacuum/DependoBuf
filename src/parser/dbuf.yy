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

// The following definitions is missing when %locations isn't used
# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

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
    $1.AddMessage(std::move($2));
    $$ = std::move($1);
  }
  | definitions service_definition {
    $$ = std::move($1);
  }
  ;

%nterm <std::unique_ptr<Message>> message_definition;
message_definition
  : dependent_message {
    $$ = std::move($1);
  }
  | independent_message {
    $$ = std::move($1);
  }
  ;

%nterm <std::unique_ptr<Message>> independent_message;
independent_message
  : MESSAGE type_identifier independent_message_body {
    $$ = std::make_unique<Message>($2);
  }
  ;
independent_message_body
  : constructors_block
  | fields_block
  ;

%nterm <std::unique_ptr<Message>> dependent_message;
dependent_message
  : MESSAGE type_identifier type_dependencies dependent_message_body {
    $$ = std::make_unique<Message>($2);
  }
  ;

%nterm <std::vector<std::unique_ptr<TypedVariable>>> type_dependencies;
type_dependencies
  : type_dependency {
    $$ = std::vector<std::unique_ptr<TypedVariable>>();
    $$.emplace_back(std::move($1));
  }
  | type_dependencies type_dependency {
    $1.emplace_back(std::move($2));
    $$ = std::move($1);
  }
  ;

%nterm <std::unique_ptr<TypedVariable>> type_dependency;
type_dependency
  : "(" typed_variable ")" {
    $$ = std::move($2);
  }
  ;

dependent_message_body : "{" NL dependent_blocks "}" NL ;
dependent_blocks
  : %empty
  | pattern_matching IMPL constructors_block dependent_blocks
  | pattern_matching IMPL fields_block dependent_blocks
  ;
pattern_matching
  : pattern_match
  | pattern_matching "," pattern_match
  ;
pattern_match
  : STAR
  | value
  ;

constructors_block
  : ENUM "{" NL constructor_declarations "}" NL ;
constructor_declarations
  : %empty
  | constructor_identifier fields_block constructor_declarations
  | constructor_identifier NL constructor_declarations
  ;
fields_block : "{" NL field_declarations "}" NL ;
field_declarations
  : %empty
  | typed_variable NL field_declarations
  ;

%nterm <std::unique_ptr<TypeExpression>> type_expr;
type_expr
  : type_identifier {
    $$ = std::make_unique<TypeExpression>($1);
  }
  | type_expr type_param {
    $1->type_parameters.push_back(std::move($2));
    $$ = std::move($1);
  }
  ;

%nterm <std::unique_ptr<Expression>> type_param;
type_param : primary { $$ = std::move($1); };

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
    $$ = std::move($1);
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
string_literal : STRING_LITERAL { $$ = std::make_unique<StringValue>($1); };

%nterm <std::unique_ptr<ConstructedValue>> constructed_value;
constructed_value
  : constructor_identifier "{" field_initialization "}" {
    $$ = std::make_unique<ConstructedValue>($1, std::move($3));
  }
  ;
%nterm <std::unique_ptr<FieldInitialization>> field_initialization;
field_initialization
  : %empty {
    $$ = std::make_unique<FieldInitialization>();
  }
  | var_identifier COLON expression {
    $$ = std::make_unique<FieldInitialization>();
    $$->AddField($1, std::move($3));
  }
  | field_initialization "," var_identifier COLON expression {
    $1->AddField($3, std::move($5));
    $$ = std::move($1);
  }
  ;

%nterm <std::string>
  type_identifier
  constructor_identifier
  service_identifier
  var_identifier
  rpc_identifier
;
type_identifier : UC_IDENTIFIER { $$ = $1; };
constructor_identifier : UC_IDENTIFIER ;
service_identifier : UC_IDENTIFIER ;
var_identifier : LC_IDENTIFIER ;
rpc_identifier : LC_IDENTIFIER ;

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

%nterm <std::unique_ptr<TypedVariable>> typed_variable;
typed_variable
  : var_identifier type_expr {
    $$ = std::make_unique<TypedVariable>($1, std::move($2));
  }

%%

void dbuf::parser::Parser::error(const location_type &l, const std::string &err_message)
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
