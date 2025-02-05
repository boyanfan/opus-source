// lexer.h
//
// This header file defines the structures and functions used for lexical analysis. It provides functionality to
// tokenize the input source code, identify keywords, operators, integers, and other tokens, while keeping track of
// their locations in the source code for debugging or error reporting purposes.
//
// Created by Boyan Fan, 2025/01/16
//

#ifndef LEXER_H
#define LEXER_H

#define OPUS_FILE_EXTENSION  ".opus"
#define READONLY_ACCESS      "r"

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

/// Parses a numeric token from the source file.
///
/// @param lexer A pointer to the Lexer structure, which maintains the current position in the source file.
/// @param sourceCode A pointer to the source file to be read.
/// \param lexeme A character buffer where the parsed numeric token will be stored.
/// \return A pointer to a Token representing the numeric token in the source code.
///
Token *parseNumeric(Lexer *lexer, FILE *sourceCode, char *lexeme);

/// Skips the current token by collecting all invalid characters based on a given sequence.
///
/// @param lexer A pointer to the Lexer structure, which maintains the current position in the source file.
/// @param sourceCode A pointer to the source file to be read.
/// \param lexeme A character buffer where the parsed numeric token will be stored.
/// \param skippedSequence A string containing characters that should be skipped.
/// \return The next character in the source file after skipping the invalid sequence.
///
int skipCurrenToken(Lexer *lexer, FILE* sourceCode, char *lexeme, char *skippedSequence);

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
int isWhitespace(int character);

/// Initializes a Lexer instance for processing source code.
///
/// @return A pointer to a newly allocated Lexer instance
///
/// The caller is responsible for ensuring that the file is open and accessible for reading, and therefore
/// use function 'FILE *openOpusSourceCode(const char*)' to safely open an Opus source code.
///
Lexer *initLexer();

/// Using this function to access an Opus source file is recommended.
///
/// @param filename The name of the file to open.
/// @return A pointer to the opened file (FILE*) if successful, or NULL if an error occurred.
///
FILE *openOpusSourceCode(const char *filename);

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