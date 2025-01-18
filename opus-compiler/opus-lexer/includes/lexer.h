// lexer.h
//
// Created by Boyan Fan, 2025/01/16
//

#ifndef LEXER_H
#define LEXER_H

#define OPUS_FILE_EXTENSION ".opus"
#define READONLY_ACCESS     "r"

#include <stdio.h>
#include "token.h"

/// All possible error types encountered during lexing.
typedef enum {
    ERROR_LEXER_NONE,   // No error occurred
} LexerError;

/// This structure holds the current error state, location information and current lexing status.
typedef struct {
    LexerError lexerError;   // Current error state of the lexer
    Location location;       // Location information for error tracking
    int isInClosure;         // Newline character inside a closure (between [...], (...) and <...>) is not a delimiter
} Lexer;

/// Reads the next token from the source code.
///
/// \param lexer A pointer to the Lexer instance to update.
/// \param sourceCode A pointer to the FILE object containing the source code.
/// @return A pointer to a Token representing the next token in the source code.
///
Token *getNextToken(Lexer *lexer, FILE* sourceCode);

/// Updates the location of the lexer to the start of the next token and return the current pointing character.
///
/// \param lexer A pointer to the Lexer instance to update.
/// \param sourceCode A pointer to the FILE object containing the source code.
/// \return The current character in the input stream without consuming it (may return 'EOF').
///
int locateStartOfNextToken(Lexer *lexer, FILE *sourceCode);

/// Updates the location of the lexer to the start of the next line and return the current pointing character.
///
/// \param lexer A pointer to the Lexer instance to update.
/// \param sourceCode A pointer to the FILE object containing the source code.
/// \return The current character in the input stream without consuming it (may return 'EOF').
///
int locateStartOfNextLine(Lexer *lexer, FILE *sourceCode);

/// Consumes the next character from the source file and updates the lexer location.
///
/// @param lexer A pointer to the Lexer structure, which maintains the current position in the source file.
/// @param sourceCode A pointer to the source file to be read.
/// @return The character (int) read from the file, or `EOF` if the end of the file is reached.
///
int consumeNextCharacter(Lexer *lexer, FILE *sourceCode);

/// Peeks at the next character in the input stream without consuming it (by advancing the file pointer).
///
/// \param sourceCode A pointer to the FILE object representing the input stream.
/// \return The next character in the input stream without consuming it (may return 'EOF').
///
int peekNextCharacter(FILE *sourceCode);

/// Checks if a character is considered whitespace in Opus.
///
/// @param character The character currently being lexed to check.
/// @return 1 (ture) if the character is a whitespace, 0 (false) otherwise.
///
int isWhitespace(char character);

/// Initializes a Lexer instance for processing source code.
///
/// @return A pointer to a newly allocated Lexer instance
///
/// The caller is responsible for ensuring that the file is open and accessible for reading, and therefore
/// use function 'FILE *openOpusSourceCode(const char*)' to safely open an Opus source code.
///
Lexer *initLexer();

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

/// Using this function to access an Opus source file is recommended.
///
/// @param filename The name of the file to open.
/// @return A pointer to the opened file (FILE*) if successful, or NULL if an error occurred.
///
FILE* openOpusSourceCode(const char *filename);

/// Checks if the given file to compile has the '.opus' extension.
///
/// @param filename The name of the file to check.
/// @return 1 (true) if the file has the ".opus" extension, 0 (false) otherwise.
///
int isOpusSourceCode(const char *filename);

/// Displays the details of a given Token, including its type, error (if any), location (line and column), and lexeme.
///
/// @param token The Token to be displayed. The Token is passed by value, so its original data will not be modified.
///
void displayToken(Token token);

#endif /* LEXER_H */
