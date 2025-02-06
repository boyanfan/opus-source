// token.h
//
// Header file for defining and managing tokens in the Opus programming language, contains the declarations of the
// `Token` structure and token types for lexical analysis in the Opus programming language. Tokens are the smallest
// units of the source code, such as keywords, identifiers, literals, operators and so on. This file provides the
// necessary tools for representing and handling tokens during the lexing process.
//
// Created by Boyan Fan, 2025/01/16
//

#ifndef TOKEN_H
#define TOKEN_H

#define LEXEME_LENGTH 128

#define ARITHMETIC_ADDITION '+'
#define ARITHMETIC_SUBTRACTION '-'
#define ARITHMETIC_MULTIPLICATION '*'
#define ARITHMETIC_DIVISION '/'
#define ARITHMETIC_MODULO '%'
#define CLOSING_ANGLE_BRACKET '>'
#define OPENING_ANGLE_BRACKET '<'
#define ASSIGNMENT_OPERATOR '='
#define COMMA ','
#define PERIOD '.'
#define COLON ':'
#define UNDERSCORE '_'
#define OPENING_BRACKET '('
#define CLOSING_BRACKET ')'
#define OPENING_CURLY_BRACKET '{'
#define CLOSING_CURLY_BRACKET '}'
#define OPENING_SQUARE_BRACKET '['
#define CLOSING_SQUARE_BRACKET ']'
#define EXCLAMATION_MARK '!'
#define DOUBLE_QUOTE '"'
#define ARITHMETIC_OPERATORS "+-*/%!="
#define COMPARISON_OPERATORS "<>=!"
#define CLOSING_CLOSURES "])}"
#define NATIVE_OPERATORS "+-*/%!@#$&?~=.:>"

/// Token types that need to be recognized by the lexer.
typedef enum {
    TOKEN_EOF,                            // The end of the input file or stream
    TOKEN_ERROR,                          // Invalid or unrecognized token
    TOKEN_NUMERIC,                        // Numeric values like "42", ...
    TOKEN_ARITHMETIC_ADDITION,            // Arithmetic addition operator "+"
    TOKEN_ARITHMETIC_SUBTRACTION,         // Arithmetic subtraction operator "-"
    TOKEN_ARITHMETIC_MULTIPLICATION,      // Arithmetic multiplication operator "*"
    TOKEN_ARITHMETIC_DIVISION,            // Arithmetic division operator "/"
    TOKEN_ARITHMETIC_MODULO,              // Arithmetic modulo operator "%"
    TOKEN_DELIMITER,                      // Delimiters like a newline character
    TOKEN_RIGHT_ARROW,                    // Function return type annotation "->"
    TOKEN_ASSIGNMENT_OPERATOR,            // Assignment operator "="
    TOKEN_COMMA,                          // Comma ","
    TOKEN_COLON,                          // Comma ":"
    TOKEN_LOGICAL_EQUIVALENCE,            // Logical equivalence operator "=="
    TOKEN_OPENING_BRACKET,                // Opening bracket "("
    TOKEN_CLOSING_BRACKET,                // Closing bracket ")"
    TOKEN_OPENING_CURLY_BRACKET,          // Opening curly bracket "{"
    TOKEN_CLOSING_CURLY_BRACKET,          // Opening curly bracket "}"
    TOKEN_OPENING_SQUARE_BRACKET,         // Opening square bracket "["
    TOKEN_CLOSING_SQUARE_BRACKET,         // Closing square bracket "["
    TOKEN_ARITHMETIC_FACTORIAL,           // Arithmetic factorial operator "!"
    TOKEN_LOGICAL_NEGATION,               // Logical negation operator "!"
    TOKEN_NOT_EQUAL_TO_OPERATOR,          // Not equal to operator "!="
    TOKEN_LESS_THAN_OPERATOR,             // Less than operator "<"
    TOKEN_LESS_OR_EQUAL_TO_OPERATOR,      // Less or equal to operator "<="
    TOKEN_GREATER_THAN_OPERATOR,          // Greater than operator ">"
    TOKEN_GREATER_OR_EQUAL_TO_OPERATOR,   // Greater or equal to operator ">="
    TOKEN_IDENTIFIER,                     // A valid identifier
    TOKEN_KEYWORD_VAR,                    // Declares a mutable variable
    TOKEN_KEYWORD_LET,                    // Declares an immutable constant
    TOKEN_KEYWORD_IF,                     // Begins a conditional statement
    TOKEN_KEYWORD_ELSE,                   // Defines an alternate execution block for `if`
    TOKEN_KEYWORD_REPEAT,                 // Starts a loop that continues until a condition is met
    TOKEN_KEYWORD_UNTIL,                  // Specifies the stopping condition for a loop
    TOKEN_KEYWORD_FOR,                    // Initiates a loop over a sequence or range
    TOKEN_KEYWORD_IN,                     // Used in `for` loops to define iteration over a range
    TOKEN_KEYWORD_RETURN,                 // Exits from a function and optionally returns a value
    TOKEN_KEYWORD_CLASS,                  // (Experimental) Defines a reference type
    TOKEN_KEYWORD_STRUCT,                 // (Experimental) Defines a value type
    TOKEN_KEYWORD_FUNC,                   // Declares a function.
    TOKEN_KEYWORD_TRUE,                   // Boolean literal representing logical `true`
    TOKEN_KEYWORD_FALSE,                  // Boolean literal representing logical `false`
    TOKEN_STRING_LITERAL,                 // A string literal wrapped by quotes
} TokenType;

/// Error types for lexical analysis
typedef enum {
    ERROR_TOKEN_NONE,            // No error occurred
    ERROR_UNRECOGNIZABLE,        // An invalid or unrecognized character was encountered
    ERROR_MALFORMED_NUMERIC,     // A number was malformed or invalid
    ERROR_UNDEFINED_OPERATOR,    // An invalid or unrecognized operator
    ERROR_OVERFLOW,              // Too long lexeme causes a buffer overflow
    ERROR_ORPHAN_UNDERSCORE,     // Underscore stands alone (invalid for an identifier)
    ERROR_UNTERMINATED_STRING,   // Missing closing double quote
} TokenError;

/// The location of a token in the source code.
typedef struct {
    int line, column;   // The line and the column where the token begins (both starting from 1).
} Location;

/// Token structure to store token information in the lexical analysis process.
/// It tracks the line number and the start column of the token.
typedef struct {
    TokenType tokenType;          // The type of the token (e.g. TOKEN_NUMERIC, TOKEN_EOF).
    TokenError tokenError;        // Any error associated with the token (e.g., ERROR_INVALID_CHAR).
    Location location;            // The location of the token in the source code (line and column).
    char lexeme[LEXEME_LENGTH];   // The actual content (lexeme) of the token.
} Token;

#endif /* TOKEN_H */
