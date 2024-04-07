# Array

```
message A (x Unsigned) {
  array Array Int x;
}

message B (a A 4) {
}

message C (b B A{array = {1, 2, 3, 4}}) {
}
```

$$
\begin{align*}
  array\_access ::=&\ var\_identifier \texttt{[} unsigned\_int\_literal \texttt{]}
\end{align*}
$$

```
message Number (x Int) {
}

enum D (array Array Int 3) {
  {1, 1, 1} => {
    Constuctor1 {
      num Number array[0];
    }
  }
  * => {
    Constuctor1 {
      num1 Number array[0];
      num2 Number array[1];
      num3 Number array[2];
    }
  }
}
```
 Operator | Description         |
|----------|---------------------|
| `\|`      | concatenation of two arrays |


```
message T {
  array1 Array Int 1;
  array2 Array Int 2;
}

message K (array Array Int 3) {
}

message P (t T) {
  k K (t.array1 & t.array2);
}
```