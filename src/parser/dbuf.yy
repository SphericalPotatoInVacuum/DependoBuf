%skeleton "lalr1.cc"
%require  "3.4"
%header

%defines 

%define api.token.raw
%define api.value.type variant

%define api.namespace {dbuf}
%define api.parser.class {Parser}

%code requires{
  namespace dbuf {
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
  #include <stdio.h>

  #include <parser/driver.hpp>

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

schema
  : END
  | message_definition schema
  | service_definition schema
  ;

message_definition
  : dependent_message 
  | independent_message
  ;

independent_message : MESSAGE type_identifier independent_message_body ;
independent_message_body
  : constructors_block
  | fields_block
  ;

dependent_message 
  : MESSAGE type_identifier type_dependencies dependent_message_body ;
type_dependencies
  : type_dependency
  | type_dependencies type_dependency
  ;
type_dependency : "(" var_identifier type_expr ")" ;

dependent_message_body : "{" NL dependent_blocks "}" NL ;
dependent_blocks
  : %empty
  | pattern_matching IMPL constructors_block dependent_blocks
  | pattern_matching IMPL fields_block dependent_blocks
  ;
pattern_matching
  : pattern_match
  | pattern_match "," pattern_matching
  ;
pattern_match
  : STAR
  | value
  | constructor_identifier
  | constructor_identifier "{" field_binding "}"
  ;
field_binding
  : field_identifier
  | field_identifier "," field_binding
  | field_identifier ":" var_identifier
  | field_identifier ":" var_identifier "," field_binding
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
  | field_identifier type_expr NL field_declarations
  ;

type_expr : type_identifier | type_expr type_param ;
type_param : primary ;

expression
  : expression PLUS expression
  | expression MINUS expression
  | expression STAR expression
  | expression SLASH expression
  | expression BANG_EQUAL expression
  | expression GREATER_EQUAL expression
  | expression LESS_EQUAL expression
  | expression AND expression
  | expression OR expression
  | expression LESS expression
  | expression EQUAL expression
  | expression GREATER expression
  | type_expr
  | MINUS expression
  | BANG expression
  | primary
  ;
primary
  : value
  | var_identifier
  | field_access
  | "(" expression ")"
  ;
field_access
  : var_identifier "." field_identifier
  | field_access "." field_identifier
  ;
value
  : literal_value
  | constructed_value
  ;
literal_value
  : bool_literal
  | float_literal
  | int_literal
  | string_literal
  ;

bool_literal
  : FALSE
  | TRUE
  ;
float_literal : FLOAT_LITERAL ;
int_literal : INT_LITERAL ;
string_literal : STRING_LITERAL ;

constructed_value : constructor_identifier "{" field_initialization "}" ;
field_initialization
  : %empty
  | field_identifier EQUAL expression "," field_initialization
  | field_identifier EQUAL expression
  ;

type_identifier : UC_IDENTIFIER;
constructor_identifier : UC_IDENTIFIER ;
service_identifier : UC_IDENTIFIER ;
var_identifier : LC_IDENTIFIER ;
field_identifier : LC_IDENTIFIER ;
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
  | argument "," arguments
  | argument
  ;
argument : var_identifier type_expr ;

%%

void dbuf::Parser::error(const location_type &l, const std::string &err_message)
{
   std::cerr << "Error: " << err_message << " at " << l << "\n";
}
