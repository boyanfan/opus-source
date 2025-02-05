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

/// Token types that need to be recognized by the lexer.
typedef enum {
    TOKEN_EOF,                         // The end of the input file or stream
    TOKEN_NUMERIC,                     // Numeric values like "42", ...
    TOKEN_ARITHMETIC_ADDITION,         // Arithmetic operators "+"
    TOKEN_ARITHMETIC_SUBTRACTION,      // Arithmetic operators "-"
    TOKEN_ARITHMETIC_MULTIPLICATION,   // Arithmetic operators "*"
    TOKEN_ARITHMETIC_DIVISION,         // Arithmetic operators "/"
    TOKEN_ARITHMETIC_MODULO,           // Arithmetic operators "%"
    TOKEN_ERROR,                       // Invalid or unrecognized token
    TOKEN_DELIMITER,                   // Delimiters like a newline character
} TokenType;

/// Error types for lexical analysis
typedef enum {
    ERROR_TOKEN_NONE,           // No error occurred
    ERROR_UNRECOGNIZABLE,       // An invalid or unrecognized character was encountered
    ERROR_MALFORMED_NUMERIC,    // A number was malformed or invalid
    ERROR_UNDEFINED_OPERATOR,   // An invalid or unrecognized operator
    ERROR_OVERFLOW,             // Too long lexeme causes a buffer overflow
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

/// Initializes a new Token instance safely with the given type and location.
///
/// This function creates a new Token and initializes its fields with the specified type and location.
/// The term "safe" indicates that this function does not handle or propagate errors, meaning it does not require
/// passing a `TokenError`. It assumes the inputs are valid and allocates memory for the token.
///
/// @param tokenType The type of the token (e.g., identifier, keyword, symbol).
/// @param location The location information of the token, typically used for debugging or error reporting.
/// @param lexeme The content (lexeme) of the token to be stored.
/// @return A pointer to the newly created Token, or NULL if memory allocation fails.
///
Token *initSafeToken(TokenType tokenType, Location location, const char *lexeme);

/// Initializes a new Token instance with the given error and location.
///
/// @param tokenError The error associated with the token, describing why it could not be initialized normally.
/// @param location The location information of the token, typically used for debugging or error reporting.
/// @param lexeme The content (lexeme) of the token to be stored.
/// @return A pointer to the newly created Token, or NULL if memory allocation fails.
///
Token *initUnsafeToken(TokenError tokenError, Location location, const char *lexeme);

#endif /* TOKEN_H */
