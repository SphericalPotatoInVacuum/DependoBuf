# Messages

One of the two main types in DependoBuf is a message type. Message declaration
consists of a keyword `message`, followed by a type identifier, followed by
optional type dependencies, followed by a block of field declarations.

$$
\begin{align*}
  type\_dependency ::=&\ \texttt{(}\ type\_variable \ \texttt{)}\\
  message\_definition ::=&\ \texttt{message}\ type\_identifier
  \ \lbrace type\_dependency \rbrace\
  \texttt{\{}\ \lbrace typed\_variable \rbrace\ \texttt{\}}
\end{align*}
$$

```title="Example message declaration"
message User (n Unsigned) {
  name String
  id Unsigned
  age Int
  active Bool
  friends Vec User n
}
```

Compiler will generate a constructor for each message type with the same name as
the message itself. Because a message has only one constructor, you can use
dot notation to access fields of a message.

```title="Example message usage"
message Foo {
  bar Int
}

message Baz (n Int) {}

message Quux {
  foo Foo
  baz Baz foo.bar
}
```

In this example you also see that you can use an already declared field as a
variable in another field declaration.

When creating a value of type `Quux` and initializing its fields you will have
to provide such a value for the `baz` field that its type parameter `n` matches
the `bar` field of the value you provided for the `foo` field. But don't worry,
as all this will be checked by the compiler and you will get a compile-time
error if you try to create a value that doesn't match the expected type.
