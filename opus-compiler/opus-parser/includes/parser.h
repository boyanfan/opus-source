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
#include "ast.h"
#include "lexer.h"

/// Error codes for parsing.
typedef enum {
    PARSE_ERROR_NONE,                        /// No error occurred during parsing.
    PARSE_ERROR_MISSING_IDENTIFIER,          /// A required identifier is missing.
    PARSE_ERROR_MISSING_TYPE_ANNOTATION,     /// A required type annotaion is missing.
    PARSE_ERROR_MISSING_TYPE_NAME,           /// A required type name is missing.
    PARSE_ERROR_MISSING_DELIMITER,           /// A required delimiter is missing.
    PARSE_ERROR_DECLARATION_SYNTAX,          /// Invalid declaration syntax.
    PARSE_ERROR_MISSING_RIGHT_VALUE,         /// A required right value to be assigned is missing.
    PARSE_ERROR_MISSING_ARGUMENT_LABEL,      /// A required argument label is missing.
    PARSE_ERROR_MISSING_COLON_AFTER_LABEL,   /// A required colon after the label is missing.
    PARSE_ERROR_UNRESOLVABLE,                /// An unresolvable token occurred.
} ParseError;

/// The parser for the Opus programming language.
/// It processes tokens from the lexer and constructs an Abstract Syntax Tree (AST).
typedef struct {
    ParseError parseError;   /// Stores the current parsing error state, if any.
    Lexer* lexer;            /// Pointer to the lexer instance responsible for tokenizing input.
    Token* currentToken;     /// Pointer to the current token being processed by the parser.
    Token* previousToken;    /// Pointer to the previous token for generating diagnostic information.
} Parser;

/// Parses a Program in the Opus programming language.
///
/// This function serves as the entry point for parsing an Opus program. It constructs
/// an Abstract Syntax Tree (AST) representing the entire program by parsing a sequence
/// of statements. The function follows the grammar:
///
///     Program -> Statement Delimiter | Program
///
///     AST_PROGRAM
///     ├── AST_STATEMENT
///     ╰── AST_PROGRAM
///         ├── AST_STATEMENT (Statement 1)
///         ╰── AST_PROGRAM
///             ...
///             ├── AST_STATEMENT (Last Statement)
///             ╰── AST_PROGRAM
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed Program, or NULL if a parsing error occurs.
///
ASTNode *parseProgram(Parser *parser, FILE *sourceCode);

/// Parses a Statement in the Opus programming language.
///
/// This function determines the type of statement based on the current token and
/// delegates parsing to the appropriate function. It follows the grammar:
///
///     Statement -> VariableDeclaration
///                | AssignmentStatement
///                | ...
///
/// The resulting Abstract Syntax Tree (AST) will have a node representing the detected
/// statement type, with its corresponding children forming the substructure.
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the parsed Statement, or NULL if a parsing error occurs.
///
ASTNode *parseStatement(Parser *parser, FILE *sourceCode);

/// Parses a VariableDeclaration statement in the Opus programming language.
///
/// This function follows the grammar:
///     VariableDeclaration -> ("var" | "let") Identifier ":" Type
///
/// It constructs an Abstract Syntax Tree (AST) node representing the variable declaration.
/// The resulting AST structure will be:
///
///     AST_VAR_DECL ("var" or "let")
///         ├── AST_IDENTIFIER ("variable_name")
///         └── AST_TYPE ("type_name")
///
/// @param parser A pointer to the Parser instance, which maintains the token stream.
/// @param sourceCode A file pointer to the source code (used for error reporting).
/// @return A pointer to the ASTNode representing the VariableDeclaration, or NULL if a parsing error occurs.
///
ASTNode *parseVariableDeclaration(Parser *parser, FILE *sourceCode);

/// Parses an assignment statement and constructs the corresponding AST node. It takes the left-value, which 
/// can be either an identifier or a declaration statement, verifies the assignment syntax, and processes 
/// the right-hand side (RHS) expression to construct an `ASTNode` representing the assignment. 
/// The assignment statement follows the grammar:
///
///     AssignmentStatement -> LeftValue '=' Expression Delimiter
///     LeftValue           -> Identifier | DeclarationStatement
///     Expression          -> Literal
///                         |  Identifier
///                         |  FunctionCall
///                         |  BinaryExpression
///                         |  UnaryExpression
///                         |  ...
///
/// An example of the resulting AST structure for 'var number: Int = 42' will be:
///     
///     AST_ASSIGNMENT (=)
///     │   ├── AST_VARIABLE_DECLARATION (var)
///     │   │   ├── AST_IDENTIFIER (number)
///     │   │   ├── AST_TYPE_ANNOTATION (Int)
///     │   ├── AST_LITERAL (42)
///
/// @param parser Pointer to the `Parser` instance that maintains the parsing state.
/// @param sourceCode Pointer to the source code file being parsed.
/// @param leftValue Pointer to an `ASTNode` representing the left-value, which could be an identifier or 
///                  a declaration statement.
/// @return Pointer to an `ASTNode` representing the assignment statement in the AST. Returns `NULL` 
///         if an error occurs during parsing.
///
ASTNode *parseAssignmentStatement(Parser *parser, FILE *sourceCode, ASTNode *leftValue);

/// Entry point for expression parsing. This function starts at the lowest precedence level.
///
ASTNode *parseExpression(Parser *parser, FILE *sourceCode);

/// Logical or has the lowest precedence in our expression grammar.
///
ASTNode *parseLogicalOr(Parser *parser, FILE *sourceCode);

/// Logical and comes next in precedence.
///
ASTNode *parseLogicalAnd(Parser *parser, FILE *sourceCode);

/// Parse additive expressions (+ and -).
///
ASTNode *parseAddition(Parser *parser, FILE *sourceCode);

/// Parse multiplicative expressions (*, /, %).
///
ASTNode *parseMultiplication(Parser *parser, FILE *sourceCode);

/// Parse unary operators (e.g., unary '-' or logical NOT).
///
ASTNode *parsePrefix(Parser *parser, FILE *sourceCode);

/// Parse postfix expressions: function calls and factorial operator.
///
ASTNode *parsePostfix(Parser *parser, FILE *sourceCode);

/// Parse primary expressions: literals, identifiers, parenthesized expressions, and boolean literals.
/// 
ASTNode *parsePrimary(Parser *parser, FILE *sourceCode);

/// Parses a function call expression given the callee, where the callee is the function name.
///
ASTNode* parseFunctionCall(Parser *parser, FILE *sourceCode, ASTNode* callee);

/// Parses a comma-separated argument list.
///
ASTNode* parseArgumentList(Parser *parser, FILE *sourceCode);

/// Checks if the current token matches the expected token type.
///
/// @param parser Pointer to the Parser instance.
/// @param type The expected token type to check against.
/// @return Ture (1) if the current token matches the expected type, False (0) otherwise.
///
int matchTokenType(Parser *parser, TokenType type);

/// Advances the parser to the next token in the source code.
///
/// @param parser Pointer to the Parser instance.
/// @param sourceCode Pointer to the source file being parsed.
/// @return Pointer to the newly retrieved Token from the lexer.
///
Token *advanceParser(Parser *parser, FILE *sourceCode); 

/// Reports the current parsing error with diagnostic information.
/// @param parser Pointer to the Parser instance.
///
void reportParseError(Parser *parser);

/// Initializes a new parser instance.
/// @return Pointer to a newly allocated Parser instance, or NULL if memory allocation fails.
///
Parser *initParser();

#endif
