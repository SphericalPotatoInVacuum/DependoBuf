# Keywords

DependoBuf has a number of keywords that are reserved for use by the language
itself. You cannot use these keywords as identifiers in your schema.

|      Keyword       | Description                                                                                                     |
|:------------------:|-----------------------------------------------------------------------------------------------------------------|
| $\texttt{message}$ | Begins a message definition.                                                                                    |
|  $\texttt{enum}$   | Begins an enum definition.                                                                                      |
|  $\texttt{func}$   | Begins a func definition.                                                                                      |
|   $\texttt{=>}$    | Maps input patterns to constructors in an enum definition.                                                      |
| $\texttt{service}$ | Begins a service definition. Not used at the moment, but reserved for future.                                   |
|   $\texttt{rpc}$   | Begins a RPC definition. Not used at the moment, but reserved for future.                                      |
| $\texttt{returns}$ | Separates the request and response types in an RPC definition. Not used at the moment, but reserved for future. |
|  $\texttt{true}$   | Boolean literal.                                                                                                |
|  $\texttt{false}$  | Boolean literal.                                                                                                |
