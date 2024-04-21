# Array
In DependoBuf, `array` is a collection type that allows the declaration of typed arrays with fixed sizes. These arrays enhance data integrity by ensuring all elements conform to the specified type and size is strictly adhered to.
```
message A (x Unsigned) {
  array Array (Int) x;
}

message B (a A 4u) {
}

message C (b B A{array : {1, 2, 3, 4}}) {
}
```

$$
\begin{align*}
  array\_access ::=&\ var\_identifier \texttt{[} expression \texttt{]}
\end{align*}
$$

The $array\_access$ operation is used to access an element within an array at a specific position determined by an index expression. 

```
message Number (x Int) {
}

enum D (array Array (Int) 3) {
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
| Operator | Description                 |
|----------|-----------------------------|
| `\|\|`   | Concatenation operator for arrays. It takes two arrays of the same type and size and merges them into a single array whose size is the sum of the sizes of the two input arrays. |


```
message T {
  array1 Array (Int) 1;
  array2 Array (Int) 2;
}

message K (array Array (Int) 3) {
}

message P (t T) {
  k K (t.array1 || t.array2);
}
```