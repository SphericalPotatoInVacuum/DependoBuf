Matching and recursion are prohibited for functions, that is, a function can take some arguments and return an expression.
Func declaration
consists of a keyword `func`, followed by a func identifier, followed by
optional type dependencies, followed by a type expression of returned value and expression.
$$
\begin{align*}
  type\_dependency ::=&\ \texttt{(}\ type\_variable \ \texttt{)}\\
  func\_definition ::=&\ \texttt{func}\ func\_identifier \{type\_dependency\} \xrightarrow{} \texttt{(}\ type\_expr \ \texttt{)} = expression
\end{align*}
$$

```title="Example func declaration"
func f (x Unsigned) (y Unsigned) (z Unsigned) -> (Unsigned) = x + y + z
message Kek {
    array Array (Int) (f (1u, 2u, 3u));
}
```
Functions support currying, that is, we transform a function from many arguments into a set of functions from one argument, the function from the example below will take an argument of type Unsigned and return a function, and so on along the chain until we return the value.
```
func f (x Unsigned) (y Unsigned) (z Unsigned) -> Unsigned = x + y + z

message DepOnFunc (g Unsigned -> Unsigned) {
    array Array (Int) (g 5);
}

message Foo {
    dep DepOnFunc (f 1 3) 
}
```