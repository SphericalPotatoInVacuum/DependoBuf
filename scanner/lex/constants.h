enum Token {
    // Single-character tokens
    LEFT_PAREN,
    RIGHT_PAREN,
    LEFT_BRACE,
    RIGHT_BRACE,
    LEFT_BRACKET,
    RIGHT_BRACKET,
    COMMA,
    DOT,
    MINUS,
    PLUS,
    SLASH,
    STAR,

    // One or two character tokens
    BANG,
    BANG_EQUAL,
    EQUAL,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL,
    AND,
    OR,

    // Literals
    LC_IDENTIFIER,
    UC_INDENTIFIER,
    STRING,
    NUMBER,
    FLOAT,


    // Keywords
    MESSAGE,
    SERVICE,
    FALSE,
    RPC,
    PRINT,
    TRUE,
    ENUM,
    IMPL,
    RETURN
};