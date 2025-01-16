// token.h
//
// Header file for defining and managing tokens in the Opus programming language, contains the declarations of the
// `Token` structure and token types for lexical analysis in the Opus programming language.
//
// Tokens are the smallest units of the source code, such as keywords, identifiers, literals, operators and so on.
// This file provides the necessary tools for representing and handling tokens during the lexing process.
//
// Created by Boyan Fan, 2025/01/16
//

#ifndef TOKEN_H
#define TOKEN_H

/// Token types that need to be recognized by the lexer.
typedef enum {
    TOKEN_EOF,                   // The end of the input file or stream
    TOKEN_NUMBER,                // Numeric values like "42", ...
    TOKEN_ARITHMETIC_OPERATOR,   // Arithmetic operators like "+", "-", "*" and "/", ...
    TOKEN_ERROR                  // Invalid or unrecognized token
} TokenType;

/// Error types for lexical analysis
typedef enum {
    ERROR_NONE,             // No error occurred
    ERROR_INVALID_CHAR,     // An invalid or unrecognized character was encountered
    ERROR_INVALID_NUMBER,   // A number was malformed or invalid
} TokenError;

/// The location of a token in the source code.
typedef struct {
    int line, column;   // The line and the column where the token begins (both starting from 1).
} Location;

/// Token structure to store token information in the lexical analysis process.
/// It tracks the line number and the start column of the token.
typedef struct {
    TokenType tokenType;     // The type of the token (e.g. TOKEN_NUMBER, TOKEN_EOF).
    TokenError tokenError;   // Any error associated with the token (e.g., ERROR_INVALID_CHAR).
    Location location;       // The location of the token in the source code (line and column).
    char lexeme[128];        // The actual content (lexeme) of the token.
} Token;

#endif /* TOKEN_H */
