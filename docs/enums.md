# Enums

Enums in DependoBuf resemble those in Rust, as in the enum constructors have
fields too. One of the main differences of enums from messages is that enums
allow you to define whether a type has a constructor based on the value of
the type dependencies. This is done by providing a mapping from the dependency
patterns to the constructors.

In case of no dependencies, there is no pattern mapping and the enum body
contains only the list of constructors.

$$
\begin{align*}
\mathrm{constructor\_declarations} ::=&\ \lbrace constructor\_identifier\ \lbrack \texttt{\{} \lbrace typed\_variable \rbrace \texttt{\}} \rbrack \rbrace \\
input\_patterns ::=&\ \texttt{*} \mid value \ \lbrace\ \texttt{,}\ ( \texttt{*} \mid value ) \rbrace \\
mapping\_rules ::=&\ input\_patterns\ \texttt{=>}\ \texttt{\{}\ constructor\_declarations\ \texttt{\}} \\
\\
enum\_definition ::=&\ independent\_enum \mid dependent\_enum \\
independent\_enum ::=&\ \texttt{enum}\ type\_identifier\ \texttt{\{}\ constructor\_declarations\ \texttt{\}}\\
dependent\_enum ::=&\ \texttt{enum}\ type\_identifier\ type\_dependency \lbrace type\_dependency \rbrace\ \texttt{\{}\ mapping\_rules\ \texttt{\}} \\
\end{align*}
$$



```title="Example enum definition"
enum IntList (n Unsigned) {
  0 => {
    Nil
  }
  * => {
    ListNode {
      value Int
      tail  IntList (n - 1)
    }
  }
}
```

While messages allow you to use dot notation to access fields, enums allow you
to use pattern matching to deconstruct values and get field values that way.

```title="Example deconstruction"
enum Nat {
  Zero
  Succ {
    pred Nat
  }
}

enum Foo (n Nat) {
  Zero => {}
  Succ{pred: x} => {
    Here `x` is bound to the value of the `pred` field of the `Succ` constructor.
    You can use `x` on the right of the `=>` to reference the value of the `pred`
    field.
  }
}
```
