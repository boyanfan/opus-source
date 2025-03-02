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

/// Error codes for parsing.
typedef enum {
    PARSE_ERROR_NONE,               /// No error occurred during parsing.
    PARSE_ERROR_UNEXPECTED_TOKEN,   /// Encountered a token that is not expected.
    PARSE_ERROR_MISSING_TOKEN,      /// A required token is missing.
    PARSE_ERROR_INVALID_SYNTAX,     /// General syntax error.
} ParseError;

#endif
