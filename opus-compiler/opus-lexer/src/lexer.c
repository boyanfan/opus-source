// lexer.c
//
// Created by Boyan Fan, 2025/01/16
//

#include <string.h>
#include <stdlib.h>
#include "lexer.h"

Token *getNextToken(Lexer *lexer, FILE* sourceCode) {
    // Skip whitespaces and comments to reach the first character of the next token
    int character = locateStartOfNextToken(lexer, sourceCode);

    // Handle the single character conversion from ASCII code (int) to lexeme (char*)
    char lexeme[LEXEME_LENGTH] = {(char) character, '\0'};

    // If the lexer has reached the end of the source code
    if (character == EOF) return initSafeToken(TOKEN_EOF, lexer->location, lexeme);

    // A newline character is a delimiter if it is outside a closure (that is [...], (...) and <...>)
    if (character == '\n' && !lexer->isInClosure) return initSafeToken(TOKEN_DELIMITER, lexer->location, lexeme);

    // If unable to recognize the token
    return initUnsafeToken(ERROR_UNRECOGNIZABLE, lexer->location, lexeme);
}

int locateStartOfNextToken(Lexer *lexer, FILE *sourceCode) {
    int character = consumeNextCharacter(lexer, sourceCode);

    // Consume the character only if the character is a whitespace
    while (isWhitespace((char) character) && character != EOF) character = consumeNextCharacter(lexer, sourceCode);

    // If the character has reached a comment line (starts with `//`), consume the entire line
    if (character == '/' && peekNextCharacter(sourceCode) == '/') {
        locateStartOfNextLine(lexer, sourceCode);
        return locateStartOfNextToken(lexer, sourceCode);
    }

    return character;
}

int locateStartOfNextLine(Lexer *lexer, FILE *sourceCode) {
    int character = peekNextCharacter(sourceCode);

    while (character != '\n') character = consumeNextCharacter(lexer, sourceCode);
    return peekNextCharacter(sourceCode);
}

int consumeNextCharacter(Lexer *lexer, FILE *sourceCode) {
    int character = fgetc(sourceCode);

    if (character != '\n') lexer->location.column++;
    else { lexer->location.line++; lexer->location.column = 1; }

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

Lexer *initLexer() {
    // Allocate memory for the Lexer instance and return NULL if memory allocation failed
    Lexer *lexer = (Lexer*) malloc(sizeof(Lexer));
    if (!lexer) return NULL;

    // The Default location starts at the beginning of the source code (line 1, column 1).
    Location location = {1, 1};

    // Initialize the lexer with no error at the beginning of the referenced source code
    lexer->lexerError = ERROR_LEXER_NONE;
    lexer->location = location;
    lexer->isInClosure = 0;
    return lexer;
}

Token *initSafeToken(TokenType tokenType, Location location, const char *lexeme) {
    Token *token = (Token*) malloc(sizeof(Token));

    token->tokenError = ERROR_TOKEN_NONE;
    token->tokenType = tokenType;
    token->location = location;

    size_t lexemeLength = strlen(lexeme);
    strncpy(token->lexeme, lexeme, lexemeLength);
    token->lexeme[lexemeLength] = '\0';

    return token;
}

Token *initUnsafeToken(TokenError tokenError, Location location, const char *lexeme) {
    Token *token = (Token*) malloc(sizeof(Token));

    token->tokenType = TOKEN_ERROR;
    token->tokenError = tokenError;
    token->location = location;

    size_t lexemeLength = strlen(lexeme);
    strncpy(token->lexeme, lexeme, lexemeLength);
    token->lexeme[lexemeLength] = '\0';

    return token;
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

void displayToken(Token token) {
    printf("<Token:");
    switch (token.tokenType) {
        case TOKEN_NUMERIC: printf("Numeric"); break;
        case TOKEN_ARITHMETIC_OPERATOR: printf("ArithmeticOperator"); break;
        case TOKEN_EOF: printf("EOF"); break;
        case TOKEN_DELIMITER: printf("Delimiter"); break;
        default: printf("Unrecognizable");
    }

    printf(", Lexeme:\"");
    switch (*token.lexeme) {
        case '\n': printf("\\n"); break;
        default: printf("%s", token.lexeme);
    }

    printf("\"> at location %d:%d\n", token.location.line, token.location.column);
}