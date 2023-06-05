# Notation

Throughout this section we will be using an extended version of the
[Backus-Naur Form](https://en.wikipedia.org/wiki/Backus%E2%80%93Naur_form)
(BNF) to describe the syntax of DependoBuf. The following table describes the
symbols used in the notation:

|           Notation           |                         Examples                         | Description                                                              |
|:----------------------------:|:--------------------------------------------------------:|--------------------------------------------------------------------------|
|            $pat$             |                       $lc\_letter$                       | A non-terminal production.                                               |
|            $::=$             |                    $a ::= \texttt{a}$                    | Definition. Represents the definition of a non-terminal.                 |
|      $\texttt{literal}$      |                    $\texttt{message}$                    | Terminal syntax. Represents exactly the characters you see.              |
|            $\mid$            |               $\texttt{a} \mid \texttt{b}$               | Alternation. Represents a choice between two or more alternatives.       |
|           $[pat]$            |                      $[lc\_letter]$                      | Optional. Represents zero or one occurrences of the enclosed pattern.    |
|     $\lbrace pat\rbrace$     |               $\lbrace lc\_letter\rbrace$                | Repetition. Represents zero or more occurrences of the enclosed pattern. |
|     $\lgroup pat\rgroup$     |               $\lgroup lc\_letter\rgroup$                | Grouping. Represents a group of patterns.                                |
| $pat_{\langle excl \rangle}$ | $lc\_letter_{\langle \texttt{a} \mid \texttt{b}\rangle}$ | Exclusion. Represents the pattern except for the excluded characters.    |

It is important to note the distinction between $\mid$ and $\texttt{|}$ and
between $\lbrace \rbrace$ and $\texttt{\{} \texttt{\}}$, however
it should be clear from the context which one is being used.
