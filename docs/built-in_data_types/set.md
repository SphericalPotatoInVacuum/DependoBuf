# Set

$$
\begin{align*}
  set ::=&\ \texttt{Set}\ type\_identifier
\end{align*}
$$

```
message A () {
  set Set Int;
}

message B (a A) {
}

message C (b B A{set = {1, 2, 3, 4}}) {
}
```

 Operator | Description         |
|----------|---------------------|
| `in`      | checks if element x is in Set |
| `\|`      | set1 $\cup$ set2 |
| `&`      | set1 $\cap$ set2 |
| `\`      | set1 \ set2 |

```
message C (b Bool) {
}

message E (set Set Int) {
  c C (2 in set);
}
```

```
message T {
  set1 Set Int;
  set2 Set Int;
}

message K (set Set Int) {
}

message P (t T) {
  k K (t.array1 & t.array2);
}
```
