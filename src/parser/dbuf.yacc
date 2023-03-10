%union {
    int intNum;
    float floatNum;
    char *str;
}

%token<str> LC_IDENTIFIER UC_IDENTIFIER
%token NL
%token MESSAGE ENUM IMPL SERVICE RPC RETURNS
%token FALSE TRUE
%token PLUS MINUS STAR SLASH
%token BANG_EQUAL GREATER_EQUAL LESS_EQUAL AND OR LESS EQUAL GREATER BANG
%token LEFT_PAREN RIGHT_PAREN LEFT_BRACE RIGHT_BRACE
%token COMMA DOT COLON
%token<floatNum> FLOAT_LITERAL
%token<intNum> INT_LITERAL
%token<str> STRING_LITERAL

%right COLON
%left BANG_EQUAL GREATER_EQUAL LESS_EQUAL LESS EQUAL GREATER
%left OR MINUS PLUS
%left AND STAR SLASH
%right BANG

%start schema
%%

schema
  : %empty
  | message_definition schema
  | service_definition schema
  ;

message_definition
  : dependent_message 
  | independent_message
  ;

independent_message
  : MESSAGE type_identifier independent_message_body NL ;
independent_message_body
  : fields_block
  | constructors_block
  ;

dependent_message
  : MESSAGE type_identifier type_dependencies dependent_message_body NL;
type_dependencies
  : type_dependency
  | type_dependencies type_dependency
  ;
type_dependency : LEFT_PAREN var_identifier type_expr RIGHT_PAREN ;

dependent_message_body
  : LEFT_BRACE NL dependent_blocks RIGHT_BRACE NL
  ;
dependent_blocks
  : %empty
  | pattern_matching IMPL constructors_block
  | pattern_matching IMPL fields_block
  ;
pattern_matching
  : pattern_match
  | pattern_match COMMA pattern_matching
  ;
pattern_match
  : STAR
  | value
  | constructor_identifier
  | constructor_identifier LEFT_BRACE field_binding RIGHT_BRACE
  ;
field_binding
  : field_identifier
  | field_identifier COMMA field_binding
  | field_identifier COLON var_identifier
  | field_identifier COLON var_identifier COMMA field_binding
  ;

constructors_block
  : ENUM LEFT_BRACE NL
    constructor_declarations
    RIGHT_BRACE NL
  ;
constructor_declarations
  : %empty
  | constructor_identifier fields_block
  | constructors_block NL
  ;
fields_block : LEFT_BRACE NL field_declarations RIGHT_BRACE NL ;
field_declarations
  : %empty
  | field_identifier type_expr NL field_declarations
  ;

type_expr : type_identifier | type_expr type_param ;
type_param : primary ;

expression
  : expression PLUS
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
  | LEFT_PAREN expression RIGHT_PAREN
  ;
field_access
  : var_identifier DOT field_identifier
  | field_access DOT field_identifier
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

constructed_value : constructor_identifier LEFT_BRACE field_initialization RIGHT_BRACE ;
field_initialization
  : %empty
  | field_identifier EQUAL expression COMMA field_initialization
  | field_identifier EQUAL expression
  ;

type_identifier : UC_IDENTIFIER ;
constructor_identifier : UC_IDENTIFIER ;
service_identifier : UC_IDENTIFIER ;
var_identifier : LC_IDENTIFIER ;
field_identifier : LC_IDENTIFIER ;
rpc_identifier : LC_IDENTIFIER ;

service_definition
  : SERVICE service_identifier rpc_block
  ;
rpc_block
  : LEFT_BRACE NL rpc_declarations RIGHT_BRACE NL ;
rpc_declarations
  : %empty
  | RPC rpc_identifier LEFT_PAREN arguments RIGHT_PAREN
    RETURNS LEFT_PAREN type_expr RIGHT_PAREN NL
  ;
arguments
  : %empty
  | argument COMMA arguments
  | argument
  ;
argument : var_identifier type_expr ;

%%
