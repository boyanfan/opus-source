// lexer.c
//
// Created by Boyan Fan, 2025/01/16
//

#include <string.h>
#include <stdlib.h>
#include "lexer.h"

Token *getNextToken(Lexer *lexer, FILE* sourceCode) {
    // Skip whitespaces to reach the position of the first character of the next token
    int character = locateStartOfNextToken(lexer, sourceCode);
    Token *token = (Token*) malloc(sizeof(Token));

    // If the lexer has reached the end of the source code
    if (character == EOF) {
        token->tokenType = TOKEN_EOF;
        token->tokenError = ERROR_TOKEN_NONE;
        token->location = lexer->location;
        return token;
    }
    printf("%c\n", (char) character);

    return token;
}

int locateStartOfNextToken(Lexer *lexer, FILE *sourceCode) {
    int character = peekNextCharacter(sourceCode);

    // Consume (skip) the character only if the character is a whitespace
    while (isWhitespace((char) character) && character != EOF) {
        character = fgetc(sourceCode);
        lexer->location.column += 1;
    }

    // If the character has reached a comment line (starts with `//`), consume the entire line
    if (character == '/' && peekNextCharacter(sourceCode) == '/') {
        locateStartOfNextLine(lexer, sourceCode);
        return locateStartOfNextToken(lexer, sourceCode);
    }

    return character;
}

int locateStartOfNextLine(Lexer *lexer, FILE *sourceCode) {
    int character;

    // Consume (skip) the entire current line and locate the lexer to the start of the next line
    while ((character = fgetc(sourceCode)) != EOF) {
        lexer->location.column++;
        if (character == '\n') {
            lexer->location.line++;
            lexer->location.column = 1;
            break;
        }
    }

    return character;
}

int peekNextCharacter(FILE *sourceCode) {
    // Read the current character and push it back into the stream
    int character = fgetc(sourceCode);
    if (character != EOF) ungetc(character, sourceCode);
    return character;
}

int isWhitespace(char character) {
    return (character == ' ' || character == '\t' || character == '\v' || character == '\r' || character == '\f');
}

Lexer *initLexer(FILE* sourceCode) {
    // Allocate memory for the Lexer instance and return NULL if memory allocation failed
    Lexer *lexer = (Lexer*) malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    // The Default location starts at the beginning of the source code (line 1, column 1).
    Location location = {1, 1};

    // Initialize the lexer with no error at the beginning of the referenced source code
    lexer->lexerError = ERROR_LEXER_NONE;
    lexer->location = location;
    lexer->sourceCode = sourceCode;
    return lexer;
}

FILE* openOpusSourceCode(const char *filename) {
    // Check if the file is Opus source code (with .opus extension)
    if (!isOpusSourceCode(filename)) {
        fprintf(stderr, "[FileTypeError]: File '%s' is not the Opus source code. (Must be .opus files)\n", filename);
        return NULL;
    }

    FILE *file = fopen(filename, READONLY_ACCESS);

    // If unable to read from the given Opus source code
    if (!file) {
        fprintf(stderr, "[AccessError]: File '%s' could not be found. (Double-check the file path)\n", filename);
        return NULL;
    }

    // Return the pointer to the file only if the file is successfully loaded
    return file;
}

int isOpusSourceCode(const char *filename) {
    const char *extension = OPUS_FILE_EXTENSION;
    size_t filenameLength = strlen(filename);
    size_t extensionLength = strlen(extension);

    // Check if the filename is long enough to include the extension
    if (filenameLength < extensionLength) return 0;

    // Compare the end of the filename with the ".opus" extension
    return strcmp(filename + filenameLength - extensionLength, extension) == 0;
}