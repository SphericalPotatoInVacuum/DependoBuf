# Set

In DependoBuf, the `set` is a dynamic collection type that allows storage of unique elements. This type supports various operations to manipulate sets, such as union, intersection, and difference, along with membership testing. 

```
message A {
  set Set (Int);
}

message B (a A) {
}

message C (b B A{set = {1, 2, 3, 4}}) {
}
```

| Operator  | Description                  |
|----------|-------------------------------|
| `&&`     | set1 $\cap$ set2              |
| `\|\|`   | set1 $\cup$ set2              |
| `\`      | set1 \ set2                   |
| `in`     | Checks for the presence of an element in a set. |

This snippet demonstrates checking if the integer 2 is in the set, utilizing the in operator. The result is used to initialize a boolean dependence in message C.
```
message C (b Bool) {
}

message E (set Set (Int)) {
  c C (2 in set);
}
```
Here, the intersection of set1 and set2 from message T is calculated using && and used to initialize message K.
```
message T {
  set1 Set (Int);
  set2 Set (Int);
}

message K (set Set (Int)) {
}

message P (t T) {
  k K (t.array1 && t.array2);
}
```
