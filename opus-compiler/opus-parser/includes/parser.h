// parser.h 
//
// Parser for the Opus Programming Language using a top-down recursive descent method.
// It parses a sequence of tokens into an Abstract Syntax Tree (AST). The AST is then used for semantic 
// analysis, optimization, and code generation.
//
// Created by Boyan Fan, 2025/03/02
// 

#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"

/// Error codes for parsing.
typedef enum {
    PARSE_ERROR_NONE,               /// No error occurred during parsing.
    PARSE_ERROR_UNEXPECTED_TOKEN,   /// Encountered a token that is not expected.
    PARSE_ERROR_MISSING_TOKEN,      /// A required token is missing.
    PARSE_ERROR_INVALID_SYNTAX,     /// General syntax error.
} ParseError;

/// The parser for the Opus programming language.
/// It processes tokens from the lexer and constructs an Abstract Syntax Tree (AST).
typedef struct {
    ParseError parseError;   /// Stores the current parsing error state, if any.
    Lexer* lexer;            /// Pointer to the lexer instance responsible for tokenizing input.
    Token* currentToken;     /// Pointer to the current token being processed by the parser.
} Parser;

/// Advances the parser to the next token in the source code.
///
/// @param parser Pointer to the Parser instance.
/// @param sourceCode Pointer to the source file being parsed.
/// @return Pointer to the newly retrieved Token from the lexer.
///
Token *advanceParser(Parser *parser, FILE *sourceCode); 

/// Initializes a new parser instance.
/// @return Pointer to a newly allocated Parser instance, or NULL if memory allocation fails.
///
Parser *initParser();

#endif
