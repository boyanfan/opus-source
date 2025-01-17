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

/// This structure holds the current error state, location information, and source code reference for the lexer.
typedef struct {
    LexerError lexerError;   // Current error state of the lexer
    Location location;       // Location information for error tracking
    FILE* sourceCode;        // Source code file being processed by the lexer
} Lexer;

/// Peeks at the next character in the input stream without advancing the file pointer.
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
/// @param sourceCode A pointer to a FILE object containing the Opus source code.
/// @return A pointer to a newly allocated Lexer instance
///
/// The caller is responsible for ensuring that the file is open and accessible for reading, and therefore
/// use function 'FILE *openOpusSourceCode(const char*)' to safely open an Opus source code.
///
Lexer *initLexer(FILE* sourceCode);

/// Opens a file with the ".opus" extension for compiling.
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

#endif /* LEXER_H */
