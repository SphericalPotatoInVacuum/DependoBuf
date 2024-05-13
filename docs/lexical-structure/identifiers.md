# Identifiers

## Basic building blocks

DependoBuf emposes strict rules on the names of identifiers. The following
productions are the basic building blocks of DependoBuf's syntax.

$$
\begin{align*}
lc\_letter &::= \texttt{a} \mid \texttt{b} \mid \ldots \mid \texttt{z} \\
uc\_letter &::= \texttt{A} \mid \texttt{B} \mid \ldots \mid \texttt{Z}\\
digit &::= \texttt{0} \mid \texttt{1} \mid \ldots \mid \texttt{9}\\
ident\_char &::= lc\_letter \mid uc\_letter \mid digit
\end{align*}
$$

## Identifier definitions

From these building blocks, we can construct identifiers:

$$
\begin{align*}
lc\_identifier &::= lc\_letter \lbrace ident\_char \rbrace \\
uc\_identifier &::= uc\_letter \lbrace ident\_char \rbrace \\
type\_identifier &::= uc\_identifier \\
constructor\_identifier &::= uc\_identifier \\
var\_identifier &::= lc\_identifier \\
collection\_identifier &::= Array \mid Set \\
\end{align*}
$$

In DependoBuf identifiers are case sensitive. This means that `foo` and `Foo`
are different identifiers. Moreover, the case of the identifier bears a semantic
meaning.

Every lowercase identifier is a function or variable (like a type dependency, which
can be understood as a parameter of the type; or a field of a constructed value).

Every uppercase identifier is a type (which are defined using `message` or `enum`
keywords) or a constructor of a type (which are defined inside `enum` body by
the user or implicitly by the compiler for `message` types).
