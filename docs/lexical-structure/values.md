# Values

You can use values in your schema to parametrize types, initialize fields, and
match patterns. Values can be of two kinds: literals and constructed values.

$$
\begin{align*}
value ::= literal \mid constructed\_value
\end{align*}
$$

## Literals

Literals are used to represent concrete values of basic types like integers,
floats, strings, and booleans.

$$
\begin{align*}
any &::= \text{any single character except a newline} \\
string\_literal &::= \texttt{"}any_{\langle\texttt{"}\texttt{\\}\rangle}\texttt{"} \\
int\_literal &::= [\texttt{+}\mid\texttt{-}] digit \lbrace digit \rbrace \\
unsigned\_int\_literal &::= digit \lbrace digit \rbrace \texttt{u} \\
float\_literal &::= [\texttt{+}\mid\texttt{-}] digit \lbrace digit \rbrace \texttt{.} digit \lbrace digit \rbrace \\
bool\_literal &::= \texttt{true} \mid \texttt{false} \\
literal &::= string\_literal \mid int\_literal \mid unsigned\_int\_literal \mid float\_literal \mid bool\_literal
\end{align*}
$$

## Array and Set Literals

$$
\begin{align*}
array\_literal &::= \texttt{\{} \ \lbrace value \lbrace ~\texttt{,}~ value \rbrace \rbrace \ \texttt{\}} \\
set\_literal &::= \texttt{\{} \ \lbrace value \lbrace ~\texttt{,}~ value \rbrace \rbrace \ \texttt{\}} \\
\end{align*}
$$

## Constructed Values

Constructed values are used to represent values of complex types like messages
and enums. They are created using a type constructor and field initializations.
Field initialization must initialize all fields of the type in the order they
are declared.

Notice that here we use the $expression$ production. It will be defined in the
[next chapter](../expressions.md).

$$
\begin{align*}
constructed\_value &::= constructor\_identifier ~\texttt{\{}~ field\_inits ~\texttt{\}}~ \\
field\_inits &::= \lbrace field\_init \lbrace ~\texttt{,}~ field\_init \rbrace \rbrace \\
field\_init &::= var\_identifier ~\texttt{:}~ expression
\end{align*}
$$
