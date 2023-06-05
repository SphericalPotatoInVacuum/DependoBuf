# Notation

Throughout this section we will be using an extended version of the
[Backus-Naur Form](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form)
(BNF) to describe the syntax of DependoBuf. The following table describes the
symbols used in the notation:

|           Notation           |                         Examples                         | Description                                                              |
|:----------------------------:|:--------------------------------------------------------:|--------------------------------------------------------------------------|
|            $pat$             |                       $lc\_letter$                       | A non-terminal production.                                               |
|            $::=$             |                    $a ::= \texttt{a}$                    | Definition. Represents the definition of a non-terminal.                 |
|      $\texttt{literal}$      |                    $\texttt{message}$                    | Terminal syntax. Represents exactly the characters you see.              |
|            $\mid$            |               $\texttt{a} \mid \texttt{b}$               | Alternation. Represents a choice between two or more alternatives.       |
|           $[pat]$            |                      $[lc\_letter]$                      | Optional. Represents zero or one occurrences of the enclosed pattern.    |
|     $\lbrace pat\rbrace$     |               $\lbrace lc\_letter\rbrace$                | Repetition. Represents zero or more occurrences of the enclosed pattern. |
|     $\lgroup pat\rgroup$     |               $\lgroup lc\_letter\rgroup$                | Grouping. Represents a group of patterns.                                |
| $pat_{\langle excl \rangle}$ | $lc\_letter_{\langle \texttt{a} \mid \texttt{b}\rangle}$ | Exclusion. Represents the pattern except for the excluded characters.    |

It is important to note the distinction between $\mid$ and $\texttt{|}$ and
between $\lbrace \rbrace$ and $\texttt{\{} \texttt{\}}$, however
it should be clear from the context which one is being used.

schema
  : %empty
  | schema definition
  | schema error
  ;

definition
  : message_definition
  | enum_definition
  | service_definition
  ;

message_definition
  : MESSAGE type_identifier type_dependencies fields_block
  | MESSAGE type_identifier fields_block
  ;

enum_definition
  : dependent_enum
  | independent_enum
  ;

dependent_enum
  : ENUM type_identifier type_dependencies "{" mapping_rules "}"
  ;

independent_enum
  : ENUM type_identifier constructors_block
  ;

type_dependencies
  : "(" typed_variable ")"
  | type_dependencies "(" typed_variable ")"
  ;

mapping_rules
  : %empty
  | mapping_rules input_patterns IMPL constructors_block
  ;

input_patterns
  : input_pattern
  | input_patterns "," input_pattern
  ;

input_pattern
  : STAR
  | value
  ;

constructors_block
  : "{" constructor_declarations "}";

constructor_declarations
  : %empty
  | constructor_declarations constructor_identifier fields_block
  | constructor_declarations constructor_identifier
  ;

fields_block : "{" field_declarations "}";

field_declarations
  : %empty
  | field_declarations typed_variable ";"
  ;

type_expr
  : type_identifier
  | type_expr primary
  ;

expression
  : expression PLUS expression
  | expression MINUS expression
  | expression STAR expression
  | expression SLASH expression
  | expression AND expression
  | expression OR expression
  | MINUS expression
  | BANG expression
  | type_expr
  | primary
  ;

primary
  : value
  | var_access
  | "(" expression ")"
  ;

var_access
  : var_identifier
  | var_access "." var_identifier
  ;

value
  : bool_literal
  | float_literal
  | int_literal
  | uint_literal
  | string_literal
  | constructed_value
  ;

bool_literal : FALSE | TRUE ;
float_literal : FLOAT_LITERAL
int_literal : INT_LITERAL
uint_literal : UINT_LITERAL
string_literal : STRING_LITERAL

constructed_value : constructor_identifier "{" field_initialization "}"

field_initialization
  : %empty
  | field_initialization_list

field_initialization_list
  : var_identifier COLON expression
  | field_initialization "," var_identifier COLON expression

type_identifier : UC_IDENTIFIER
constructor_identifier : UC_IDENTIFIER
var_identifier : LC_IDENTIFIER

typed_variable : var_identifier type_expr
