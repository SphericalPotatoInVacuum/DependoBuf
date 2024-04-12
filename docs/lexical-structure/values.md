# Values

You can use values in your schema to parametrize types, initialize fields, and
match patterns. Values can be of two kinds: literals, constructed values or collection values.

$$
\begin{align*}
value ::= literal \mid constructed\_value \mid collection\_value
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

## Constructed Values

Constructed values are used to represent values of complex types like messages
and enums. They are created using a type constructor and field initializations.
Field initialization must initialize all fields of the type in the order they
are declared.

Notice that here we use $expression$. It is defined in the [expressions.md](../expressions.md).

$$
\begin{align*}
constructed\_value &::= constructor\_identifier ~\texttt{\{}~ field\_inits ~\texttt{\}}~ \\
field\_inits &::= \lbrace field\_init \ \lbrace ~\texttt{,}~ field\_init \rbrace \rbrace \\
field\_init &::= var\_identifier ~\texttt{:}~ expression
\end{align*}
$$

## Collection Values

Collection values are used to represent values of Arrays and Sets. They are created using $collection\_elements$. The elements of the collection must be of the same type as the Array or Set stores

$$
\begin{align*}
collection\_value &::= collection\_identifier ~\texttt{\{}  \ collection\_elements \ \texttt{\}} \\
collection\_elements &::= \lbrace expression \ \lbrace ~\texttt{,}~ expression \rbrace \rbrace \\
\end{align*}
$$


The collections themselves are defined in the [array.md](../collection_types/array.md) and [set.md](../collection_types/set.md)