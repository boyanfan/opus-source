// lexer.c
//
// Created by Boyan Fan, 2025/01/16
//

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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

    // If the lexer has reached a numeric literal, try to lex it and handle any possible numeric token errors
    if (isdigit(character)) {
        TokenError tokenError = parseNumeric(lexer, sourceCode, lexeme);

        if (tokenError == ERROR_TOKEN_NONE) return initSafeToken(TOKEN_NUMERIC, lexer->location, lexeme);
        else return initUnsafeToken(tokenError, lexer->location, lexeme);
    }

    // If the lexer has reached an arithmetic addition operator
    if (character == ARITHMETIC_ADDITION) {
        // In the current phase, Opus does not support increment operation (`++` or `+=`), therefore,
        // The only valid operator starts with addition symbol (`+`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer->location, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_ADDITION, lexer->location, lexeme);
    }

    // If the lexer has reached an arithmetic subtraction operator
    if (character == ARITHMETIC_SUBTRACTION) {
        // Try to parse right arrow (`->`) operator that annotates the return type of functions
        if (peekNextCharacter(sourceCode) == '>') {
            consumeNextCharacter(lexer, sourceCode);
            return initSafeToken(TOKEN_RIGHT_ARROW, lexer->location, RIGHT_ARROW);
        }

        // In the current phase, Opus does not support decrement operation (`--` or `-=`), therefore,
        // The valid operators start with it (`-`) are arithmetic subtraction (`-`) and right arrow (`->`)
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer->location, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_SUBTRACTION, lexer->location, lexeme);
    }

    // If the lexer has reached an arithmetic multiplication operator
    if (character == ARITHMETIC_MULTIPLICATION) {
        // In the current phase, Opus does not support self multiplication operation (`*=`), therefore,
        // The only valid operator starts with multiplication symbol (`*`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer->location, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_MULTIPLICATION, lexer->location, lexeme);
    }

    // If the lexer has reached an arithmetic division operator
    if (character == ARITHMETIC_DIVISION) {
        // In the current phase, Opus does not support self division operation (`/=`), therefore,
        // The only valid operator starts with division symbol (`/`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer->location, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_DIVISION, lexer->location, lexeme);
    }

    // If the lexer has reached an arithmetic modulo operator
    if (character == ARITHMETIC_MODULO) {
        // In the current phase, Opus does not support self modulo operation (`%=`), therefore,
        // The only valid operator starts with modulo symbol (`%`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer->location, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_MODULO, lexer->location, lexeme);
    }

    // If unable to recognize the token
    return initUnsafeToken(ERROR_UNRECOGNIZABLE, lexer->location, lexeme);
}

int skipCurrenToken(Lexer *lexer, FILE* sourceCode, char *lexeme, char *skippedSequence) {
    int position = (int) strlen(lexeme);

    // Collect all invalid characters
    while (strchr(skippedSequence, peekNextCharacter(sourceCode))) {
        lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
    }

    lexeme[position] = '\0';
    return consumeNextCharacter(lexer, sourceCode);
}

int locateStartOfNextToken(Lexer *lexer, FILE *sourceCode) {
    int character = consumeNextCharacter(lexer, sourceCode);

    // Consume the character only if the character is a whitespace
    while (isWhitespace(character) && character != EOF) character = consumeNextCharacter(lexer, sourceCode);

    // If the character has reached a comment line (starts with `//`), consume the entire line
    if (character == '/' && peekNextCharacter(sourceCode) == '/') {
        locateStartOfNextLine(lexer, sourceCode);
        return locateStartOfNextToken(lexer, sourceCode);
    }

    return character;
}

int locateStartOfNextLine(Lexer *lexer, FILE *sourceCode) {
    int character = peekNextCharacter(sourceCode);

    while (character != '\n' && character != EOF) character = consumeNextCharacter(lexer, sourceCode);
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

int isWhitespace(int character) {
    return (character == ' ' || character == '\t' || character == '\v' || character == '\r' || character == '\f');
}

TokenError parseNumeric(Lexer *lexer, FILE *sourceCode, char *lexeme) {
    // Get ready for the next digit character
    int decimal = 1;

    // Since newline character could be a delimiter, we must handle it before actually consuming it
    while (isdigit(peekNextCharacter(sourceCode)) && decimal < LEXEME_LENGTH) {
        lexeme[decimal++] = (char) consumeNextCharacter(lexer, sourceCode);
    }

    // Check if the length of the numeric value could cause a buffer overflow
    if (decimal == LEXEME_LENGTH - 1) return ERROR_OVERFLOW;

    // Terminate lexeme if the number is successfully parsed
    lexeme[decimal] = '\0';
    return ERROR_TOKEN_NONE;
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

    int lexemeLength = (int) strlen(lexeme);
    strncpy(token->lexeme, lexeme, lexemeLength);
    token->lexeme[lexemeLength] = '\0';

    // Get the beginning location of the current token
    token->location.line = location.line;
    token->location.column = location.column - (int) lexemeLength + 1;

    return token;
}

Token *initUnsafeToken(TokenError tokenError, Location location, const char *lexeme) {
    Token *token = (Token*) malloc(sizeof(Token));

    token->tokenType = TOKEN_ERROR;
    token->tokenError = tokenError;

    int lexemeLength = (int) strlen(lexeme);
    strncpy(token->lexeme, lexeme, lexemeLength);
    token->lexeme[lexemeLength] = '\0';

    // Get the beginning location of the current token
    token->location.line = location.line;
    token->location.column = location.column - (int) lexemeLength + 1;

    return token;
}

FILE *openOpusSourceCode(const char *filename) {
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
    if (token.tokenError == ERROR_TOKEN_NONE) printf("<Token:");
    else printf("<ERROR:");

    switch (token.tokenType) {
        case TOKEN_NUMERIC: printf("Numeric"); break;
        case TOKEN_ARITHMETIC_ADDITION: printf("AdditionOperator"); break;
        case TOKEN_ARITHMETIC_SUBTRACTION: printf("SubtractionOperator"); break;
        case TOKEN_ARITHMETIC_MULTIPLICATION: printf("MultiplicationOperator"); break;
        case TOKEN_ARITHMETIC_DIVISION: printf("DivisionOperator"); break;
        case TOKEN_ARITHMETIC_MODULO: printf("ModuloOperator"); break;
        case TOKEN_EOF: printf("EOF"); break;
        case TOKEN_DELIMITER: printf("Delimiter"); break;
        default:;
    }

    switch (token.tokenError) {
        case ERROR_MALFORMED_NUMERIC: printf("MalformedNumeric"); break;
        case ERROR_UNDEFINED_OPERATOR: printf("UndefinedOperator"); break;
        default:;
    }

    printf(", Lexeme:\"");
    switch (*token.lexeme) {
        case '\n': printf("\\n"); break;
        default: printf("%s", token.lexeme);
    }

    printf("\"> at location %d:%d\n", token.location.line, token.location.column);
}