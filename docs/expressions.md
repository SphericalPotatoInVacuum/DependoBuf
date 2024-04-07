# Expressions

## Value expressions

Value expressions are used to compute values. They can be used wherever a value
is expected, for example in field initializations, in pattern matching, or in
type parametrization.

From this point forward we will omit "value" and just say "expression",
implying that it is a value expression, unless otherwise specified.

$$
\begin{align*}
  expression ::=&
  \ expression
  \left(\ \texttt{+} \mid \texttt{-} \mid \texttt{*} \mid \texttt{/} \mid \texttt{\&} \mid \texttt{|}\ \right)
  expression \\
  \mid&\ \texttt{-} expression  \\
  \mid&\ \texttt{!} expression  \\
  \mid&\ primary
\end{align*}
$$

### Precedence and Associativity

DependoBuf uses the same precedence and associativity rules as any other
language, which are also used in mathematics. The following table lists
operators in order of decreasing precedence:

| Operator | Description         |
|----------|---------------------|
| `!`      | Logical NOT         |
| `-`      | Arithmetic negation |
| `*`      | Multiplication      |
| `/`      | Division            |
| `&`      | Logical AND         |
| `\|`     | Logical OR          |
| `+`      | Addition            |
| `-`      | Subtraction         |

### Primary expressions

Primary is like an atomic expression. It can be a value, a variable (with
optional field access), or a parenthesized expression.

$$
\begin{align*}
  var\_access ::=&\ var\_identifier \lbrace \ \texttt{.}\ var\_identifier \rbrace \\
  primary ::=&\ value \mid var\_access
  \mid \texttt{(} expression \texttt{)}
\end{align*}
$$

## Type expressions

Type expressions are used to compute types. They can be used wherever a type is
expected: in declarations of parameters (be it type parameters or rpc parameters),
or in field declarations. Such a variable declaration is called a typed variable
in our grammar. It is simply a variable identifier followed by a type expression.

Type expressions consist of type name and optional type parameters. Type
parameters need to be primary expressions to avoid ambiguity.

$$
\begin{align*}
  type\_expr ::=&\ type\_identifier \lbrace primary \rbrace \\
  typed\_variable ::=&\ var\_identifier\ type\_expr
\end{align*}
$$
