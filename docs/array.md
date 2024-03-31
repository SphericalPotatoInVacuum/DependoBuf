# Array

...

$$
\begin{align*}
  type\_dependency ::=&\ \texttt{(}\ typed\_variable \texttt{)}\\
  size\_variable ::=&\ unsigned\_int\_literal \\
  array\_definition ::=&\ \texttt{array}\ type\_identifier \ <type\_expr, \ size\_variable>
\end{align*}
$$

```
message Num {
    x Int;
}

array List <Num, 2>
```

```
message Foo {
  l List;
}

message Bar (foo Foo) {}

message Tar (bar Bar Foo{l: List Num{x: 5} Num{x: 4}}) {}
```


