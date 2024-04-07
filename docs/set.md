# Set

$$
\begin{align*}
  size\_variable ::=&\ unsigned\_int\_literal \\
  set ::=&\ \texttt{Set}\ type\_identifier \ size\_variable \\
\end{align*}
$$

```
message A (x Unsigned) {
  set Set Int x;
}

message B (a A 4) {
}

message C (b B A{set = {1, 2, 3, 4}}) {
}
```

 Operator | Description         |
|----------|---------------------|
| `in`      | checks if element x is in Set |
| `+`      | set1 $\cup$ set2 |
| `-`      | set1 \ set2 |

```
message C (b Bool) {
}

message E (set Set Int 3) {
  c C (2 in set);
}
```

```
message T {
  set1 Set Int 1;
  set2 Set Int 2;
}

message K (array Array) {
}

message P (t T) {
  k K (t.array1 - t.array2);
}
```
