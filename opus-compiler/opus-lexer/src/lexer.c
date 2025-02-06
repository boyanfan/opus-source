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

    // If the lexer has reached the end of the source code, report any errors yet unresolved
    if (character == EOF) {
        reportLexerError(lexer);
        return initSafeToken(TOKEN_EOF, lexer, lexeme);
    }

    // A newline character is a delimiter if it is outside a closure (that is "[...]" and "(...)")
    if (character == '\n' && !isInClosure(lexer)) return initSafeToken(TOKEN_DELIMITER, lexer, lexeme);

    // If the lexer has reached a numeric literal, try to lex it and handle any possible numeric token errors
    if (isdigit(character)) return parseNumeric(lexer, sourceCode, lexeme);

    // If the lexer has reached an arithmetic addition operator
    if (character == ARITHMETIC_ADDITION) {
        // In the current phase, Opus does not support increment operation (`++` or `+=`), therefore,
        // The only valid operator starts with addition symbol (`+`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_ADDITION, lexer, lexeme);
    }

    // If the lexer has reached an arithmetic subtraction operator
    if (character == ARITHMETIC_SUBTRACTION) {
        // Try to parse right arrow (`->`) operator that annotates the return type of functions
        if (peekNextCharacter(sourceCode) == CLOSING_ANGLE_BRACKET) {
            int position = (int) strlen(lexeme);
            lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
            lexeme[position] = '\0';

            // Any additional symbol followed by operator `->` should form and be recognized as an undefined operator
            if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
                skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
                return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
            }

            return initSafeToken(TOKEN_RIGHT_ARROW, lexer, lexeme);
        }

        // Handle Negative numbers
        if (isdigit(peekNextCharacter(sourceCode))) return parseNumeric(lexer, sourceCode, lexeme);

        // In the current phase, Opus does not support decrement operation (`--` or `-=`), therefore,
        // The valid operators start with it (`-`) are arithmetic subtraction (`-`) and right arrow (`->`)
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_SUBTRACTION, lexer, lexeme);
    }

    // If the lexer has reached an arithmetic multiplication operator
    if (character == ARITHMETIC_MULTIPLICATION) {
        // In the current phase, Opus does not support self multiplication operation (`*=`), therefore,
        // The only valid operator starts with multiplication symbol (`*`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_MULTIPLICATION, lexer, lexeme);
    }

    // If the lexer has reached an arithmetic division operator
    if (character == ARITHMETIC_DIVISION) {
        // In the current phase, Opus does not support self division operation (`/=`), therefore,
        // The only valid operator starts with division symbol (`/`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_DIVISION, lexer, lexeme);
    }

    // If the lexer has reached an arithmetic modulo operator
    if (character == ARITHMETIC_MODULO) {
        // In the current phase, Opus does not support self modulo operation (`%=`), therefore,
        // The only valid operator starts with modulo symbol (`%`) should be itself, that is,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_ARITHMETIC_MODULO, lexer, lexeme);
    }

    // If the lexer has reached an assignment operator
    if (character == ASSIGNMENT_OPERATOR) {
        // In the current phase, Opus only supports two operations starts with equal sign (`=`), that is,
        // Assignment operation (`=`) and logical equivalence operator (`==`), therefore,
        // First try to match the logical equivalence operator (`==`)
        if (peekNextCharacter(sourceCode) == ASSIGNMENT_OPERATOR) {
            int position = (int) strlen(lexeme);
            lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
            lexeme[position] = '\0';

            // Any additional symbol followed by operator `==` should form and be recognized as an undefined operator
            if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
                skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
                return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
            }

            return initSafeToken(TOKEN_LOGICAL_EQUIVALENCE, lexer, lexeme);
        }

        // Any additional symbol followed by operator `=` should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_ASSIGNMENT_OPERATOR, lexer, lexeme);
    }

    // If the lexer has reached a negation operation (`!`)
    if (character == EXCLAMATION_MARK) {
        // If it is placed after an integer token, it should be recognized as an arithmetic factorial operator
        if (lexer->previousTokenType == TOKEN_NUMERIC || lexer->previousTokenType == TOKEN_IDENTIFIER)
            return initSafeToken(TOKEN_ARITHMETIC_FACTORIAL, lexer, lexeme);

        // If it is followed by an assignment operator (`=`), it is not equal to operator (`!=`)
        // Note that the arithmetic factorial operator has higher precedence than it
        if (peekNextCharacter(sourceCode) == ASSIGNMENT_OPERATOR) {
            int position = (int) strlen(lexeme);
            lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
            lexeme[position] = '\0';

            // Any additional symbol followed by operator `!=` should form and be recognized as an undefined operator
            if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
                skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
                return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
            }

            return initSafeToken(TOKEN_NOT_EQUAL_TO_OPERATOR, lexer, lexeme);
        }

        // If it stands alone, it should be recognized as a logical negation operator
        // Any additional symbol followed by operator `!` should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_LOGICAL_NEGATION, lexer, lexeme);
    }

    // If the lexer has reached an opening angle bracket (`<`)
    if (character == OPENING_ANGLE_BRACKET) {
        // Try to match the less or equal to operator (`<=`)
        if (peekNextCharacter(sourceCode) == ASSIGNMENT_OPERATOR) {
            int position = (int) strlen(lexeme);
            lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
            lexeme[position] = '\0';

            // Any additional symbol followed by operator `==` should form and be recognized as an undefined operator
            if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
                skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
                return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
            }

            return initSafeToken(TOKEN_LESS_OR_EQUAL_TO_OPERATOR, lexer, lexeme);
        }

        // In the current phase, Opus does not support logical or arithmetic left shift operation (`<<`), therefore,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_LESS_THAN_OPERATOR, lexer, lexeme);
    }

    // If the lexer has reached a closing angle bracket (`>`)
    if (character == CLOSING_ANGLE_BRACKET) {
        // Try to match the less or equal to operator (`>=`)
        if (peekNextCharacter(sourceCode) == ASSIGNMENT_OPERATOR) {
            int position = (int) strlen(lexeme);
            lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
            lexeme[position] = '\0';

            // Any additional symbol followed by operator `==` should form and be recognized as an undefined operator
            if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
                skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
                return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
            }

            return initSafeToken(TOKEN_GREATER_OR_EQUAL_TO_OPERATOR, lexer, lexeme);
        }

        // In the current phase, Opus does not support logical or arithmetic right shift operation (`>>`), therefore,
        // Any additional symbol followed by it should form and be recognized as an undefined operator
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_GREATER_THAN_OPERATOR, lexer, lexeme);
    }

    // If the lexer has reached a comma
    if (character == COMMA) return initSafeToken(TOKEN_COMMA, lexer, lexeme);

    // If the lexer has reached a colon
    if (character == COLON) {
        // In the current phase, Opus only supports using a single colon (`:`) to annotate types
        if (strchr(NATIVE_OPERATORS, peekNextCharacter(sourceCode))) {
            skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
            return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
        }

        return initSafeToken(TOKEN_COLON, lexer, lexeme);
    }

    // If the lexer has reached an opening bracket
    if (character == OPENING_BRACKET) {
        lexer->isInClosure[BRACKET_CLOSURE]++;
        return initSafeToken(TOKEN_OPENING_BRACKET, lexer, lexeme);
    }

    // If the lexer has reached an opening bracket
    if (character == CLOSING_BRACKET) {
        lexer->isInClosure[BRACKET_CLOSURE]--;
        return initSafeToken(TOKEN_CLOSING_BRACKET, lexer, lexeme);
    }

    // If the lexer has reached an opening curly bracket
    if (character == OPENING_CURLY_BRACKET) {
        lexer->isInClosure[CURLY_BRACKET_CLOSURE]++;
        return initSafeToken(TOKEN_OPENING_CURLY_BRACKET, lexer, lexeme);
    }

    // If the lexer has reached a closing curly bracket
    if (character == CLOSING_CURLY_BRACKET) {
        lexer->isInClosure[CURLY_BRACKET_CLOSURE]--;
        return initSafeToken(TOKEN_CLOSING_CURLY_BRACKET, lexer, lexeme);
    }

    // If the lexer has reached an opening square bracket
    if (character == OPENING_SQUARE_BRACKET) {
        lexer->isInClosure[SQUARE_BRACKET_CLOSURE]++;
        return initSafeToken(TOKEN_OPENING_SQUARE_BRACKET, lexer, lexeme);
    }

    // If the lexer has reached a closing square bracket
    if (character == CLOSING_SQUARE_BRACKET) {
        lexer->isInClosure[SQUARE_BRACKET_CLOSURE]--;
        return initSafeToken(TOKEN_CLOSING_SQUARE_BRACKET, lexer, lexeme);
    }

    // If the lexer has reached a logical and operator
    if (character == LOGICAL_AND_OPERATOR) {
        // First try to match the logical and operator (`&&`)
        if (peekNextCharacter(sourceCode) == LOGICAL_AND_OPERATOR) {
            int position = (int) strlen(lexeme);
            lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
            lexeme[position] = '\0';

            // Any additional symbol followed by operator `&&` should form and be recognized as an undefined operator
            int nextCharacter = peekNextCharacter(sourceCode);
            if (strchr(NATIVE_OPERATORS, nextCharacter) && nextCharacter != EXCLAMATION_MARK) {
                skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
                return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
            }

            return initSafeToken(TOKEN_LOGICAL_AND_OPERATOR, lexer, lexeme);
        }
    }

    // If the lexer has reached a logical or operator
    if (character == LOGICAL_OR_OPERATOR) {
        // First try to match the logical and operator (`||`)
        if (peekNextCharacter(sourceCode) == LOGICAL_OR_OPERATOR) {
            int position = (int) strlen(lexeme);
            lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
            lexeme[position] = '\0';

            // Any additional symbol followed by operator `&&` should form and be recognized as an undefined operator
            int nextCharacter = peekNextCharacter(sourceCode);
            if (strchr(NATIVE_OPERATORS, nextCharacter) && nextCharacter != EXCLAMATION_MARK) {
                skipCurrenToken(lexer, sourceCode, lexeme, NATIVE_OPERATORS);
                return initUnsafeToken(ERROR_UNDEFINED_OPERATOR, lexer, lexeme);
            }

            return initSafeToken(TOKEN_LOGICAL_OR_OPERATOR, lexer, lexeme);
        }
    }

    // If the lexer has reached a double quote for a string literal
    if (character == DOUBLE_QUOTE) {
        // Skip the opening quote to lex the content of a string literal
        character = consumeNextCharacter(lexer, sourceCode);
        int position = 0;

        while (character != DOUBLE_QUOTE && character != '\0' && character != EOF && position < LEXEME_LENGTH) {
            // Check for the escape character
            if (character == '\\') lexeme[position++] = '\\';
            else lexeme[position++] = (char) character;

            character = consumeNextCharacter(lexer, sourceCode);
        }

        // If a string literal does not be terminated by a closing quote
        if (character == EOF) return initUnsafeToken(ERROR_UNTERMINATED_STRING, lexer, lexeme);

        lexeme[position] = '\0';
        return initSafeToken(TOKEN_STRING_LITERAL, lexer, lexeme);
    }

    // If an underscore stands alone, it cannot be any operator nor an identifier (but `__` is valid)
    int nextCharacter = peekNextCharacter(sourceCode);
    if (character == UNDERSCORE && !isalpha(nextCharacter) && nextCharacter != UNDERSCORE) {
        return initUnsafeToken(ERROR_ORPHAN_UNDERSCORE, lexer, lexeme);
    }

    // The first character of an identifier or a keyword must be a letter or an underscore
    if (isalpha(character) || character == UNDERSCORE) {
        int position = (int) strlen(lexeme);

        // Collect all valid characters for an identifier
        while (isalnum(nextCharacter) || nextCharacter == UNDERSCORE) {
            lexeme[position++]  = (char) consumeNextCharacter(lexer, sourceCode);
            nextCharacter = peekNextCharacter(sourceCode);
        }

        lexeme[position] = '\0';
        TokenType tokenType = TOKEN_IDENTIFIER;

        // Compare the collected lexeme to known keywords
        if (strcmp(lexeme, "var") == 0) { tokenType = TOKEN_KEYWORD_VAR; }
        else if (strcmp(lexeme, "let") == 0) { tokenType = TOKEN_KEYWORD_LET; }
        else if (strcmp(lexeme, "if") == 0) { tokenType = TOKEN_KEYWORD_IF; }
        else if (strcmp(lexeme, "else") == 0) { tokenType = TOKEN_KEYWORD_ELSE; }
        else if (strcmp(lexeme, "repeat") == 0) { tokenType = TOKEN_KEYWORD_REPEAT; }
        else if (strcmp(lexeme, "until") == 0) { tokenType = TOKEN_KEYWORD_UNTIL; }
        else if (strcmp(lexeme, "for") == 0) { tokenType = TOKEN_KEYWORD_FOR; }
        else if (strcmp(lexeme, "in") == 0) { tokenType = TOKEN_KEYWORD_IN; }
        else if (strcmp(lexeme, "return") == 0) { tokenType = TOKEN_KEYWORD_RETURN; }
        else if (strcmp(lexeme, "class") == 0) { tokenType = TOKEN_KEYWORD_CLASS; }
        else if (strcmp(lexeme, "struct") == 0) { tokenType = TOKEN_KEYWORD_STRUCT; }
        else if (strcmp(lexeme, "func") == 0) { tokenType = TOKEN_KEYWORD_FUNC; }
        else if (strcmp(lexeme, "true") == 0) { tokenType = TOKEN_KEYWORD_TRUE; }
        else if (strcmp(lexeme, "false") == 0) { tokenType = TOKEN_KEYWORD_FALSE; }

        return initSafeToken(tokenType, lexer, lexeme);
    }

    // If unable to recognize the token
    return initUnsafeToken(ERROR_UNRECOGNIZABLE, lexer, lexeme);
}

Token *parseNumeric(Lexer *lexer, FILE *sourceCode, char *lexeme) {
    // Get ready for the next digit character
    int decimal = 1;

    // Track the floating position point of a number, where 0 for integers and 1 for floating values
    int floatingPosition = 0;

    // Peek the next character to check if we could terminate a numeric lexeme
    int character = peekNextCharacter(sourceCode);

    // Since newline character could be a delimiter, we must peek it before actually consuming it
    while ((isdigit(character) || character == PERIOD) && decimal < LEXEME_LENGTH) {
        if (character == PERIOD) floatingPosition++;
        lexeme[decimal++] = (char) consumeNextCharacter(lexer, sourceCode);
        character = peekNextCharacter(sourceCode);
    }

    // It is malformed if there are multiple floating points
    if (floatingPosition > 1) return initUnsafeToken(ERROR_MALFORMED_NUMERIC, lexer, lexeme);

    // After parsing all digits, we check the next character since a numeric literal must end with
    // 1. a whitespace;
    // 2. a delimiter;
    // 3. any arithmetic operators (+-*/%!=);
    // 4. any comparison operators (<>=!);
    // 5. any logical operators (&|)
    // 5. any closing closure ("}", ")", or "]");
    // 6. A comma (",");
    // 7. End of the source code (EOF)
    if (isWhitespace(character) || character == '\n' || strchr(ARITHMETIC_OPERATORS, character) || character == EOF ||
        strchr(COMPARISON_OPERATORS, character) || strchr(CLOSING_CLOSURES, character) || character == COMMA ||
        strchr(LOGICAL_OPERATORS, character)) {
        lexeme[decimal] = '\0';
        return initSafeToken(TOKEN_NUMERIC, lexer, lexeme);
    }

    // Collect all invalid characters
    while (!(isWhitespace(character) || character == '\n' || strchr(ARITHMETIC_OPERATORS, character) ||
           strchr(COMPARISON_OPERATORS, character) || strchr(CLOSING_CLOSURES, character) || character == COMMA ||
           character == EOF || strchr(LOGICAL_OPERATORS, character)) && decimal < LEXEME_LENGTH) {
        lexeme[decimal++] = (char) consumeNextCharacter(lexer, sourceCode);
        character = peekNextCharacter(sourceCode);
    }

    // Check if the length of the numeric value could cause a buffer overflow
    if (decimal == LEXEME_LENGTH - 1) return initUnsafeToken(ERROR_OVERFLOW, lexer, lexeme);

    lexeme[decimal] = '\0';
    return initUnsafeToken(ERROR_MALFORMED_NUMERIC, lexer, lexeme);
}

int skipCurrenToken(Lexer *lexer, FILE* sourceCode, char *lexeme, char *skippedSequence) {
    int position = (int) strlen(lexeme);

    // Collect all invalid characters
    while (strchr(skippedSequence, peekNextCharacter(sourceCode))) {
        lexeme[position++] = (char) consumeNextCharacter(lexer, sourceCode);
    }

    lexeme[position] = '\0';
    return peekNextCharacter(sourceCode);
}

int locateStartOfNextToken(Lexer *lexer, FILE *sourceCode) {
    int character = consumeNextCharacter(lexer, sourceCode);

    // Consume the character only if the character is a whitespace
    while ((isWhitespace(character) || (isInClosure(lexer) && character == '\n')) && character != EOF)
        character = consumeNextCharacter(lexer, sourceCode);

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

void reportLexerError(Lexer *lexer) {
    if (lexer->isInClosure[BRACKET_CLOSURE] != 0) lexer->lexerError = ERROR_UNCLOSED_BRACKET;
    if (lexer->isInClosure[CURLY_BRACKET_CLOSURE] != 0) lexer->lexerError = ERROR_UNCLOSED_CURLY_BRACKET;
    if (lexer->isInClosure[SQUARE_BRACKET_CLOSURE] != 0) lexer->lexerError = ERROR_UNCLOSED_SQUARE_BRACKET;

    switch (lexer->lexerError) {
        case ERROR_UNCLOSED_BRACKET: printf("[ERROR]: Unclosed bracket occurs!"); break;
        case ERROR_UNCLOSED_CURLY_BRACKET: printf("[ERROR]: Unclosed curly bracket occurs!"); break;
        case ERROR_UNCLOSED_SQUARE_BRACKET: printf("[ERROR]: Unclosed square bracket occurs!"); break;
        default:;
    }
}

int isWhitespace(int character) {
    return (character == ' ' || character == '\t' || character == '\v' || character == '\r' || character == '\f');
}

int isInClosure(Lexer *lexer) {
    return lexer->isInClosure[BRACKET_CLOSURE] || lexer->isInClosure[SQUARE_BRACKET_CLOSURE];
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
    lexer->previousTokenType = TOKEN_ERROR;
    for (int index = 0; index < 3; index++) lexer->isInClosure[index] = 0;

    return lexer;
}

Token *initSafeToken(TokenType tokenType, Lexer *lexer, const char *lexeme) {
    Token *token = (Token*) malloc(sizeof(Token));

    token->tokenError = ERROR_TOKEN_NONE;
    token->tokenType = tokenType;

    int lexemeLength = (int) strlen(lexeme);
    strncpy(token->lexeme, lexeme, lexemeLength);
    token->lexeme[lexemeLength] = '\0';

    // Get the beginning location of the current token
    token->location.line = lexer->location.line;
    token->location.column = lexer->location.column - (int) lexemeLength + 1;

    lexer->previousTokenType = tokenType;
    return token;
}

Token *initUnsafeToken(TokenError tokenError, Lexer *lexer, const char *lexeme) {
    Token *token = (Token*) malloc(sizeof(Token));

    token->tokenType = TOKEN_ERROR;
    token->tokenError = tokenError;

    int lexemeLength = (int) strlen(lexeme);
    strncpy(token->lexeme, lexeme, lexemeLength);
    token->lexeme[lexemeLength] = '\0';

    // Get the beginning location of the current token
    token->location.line = lexer->location.line;

    int column = lexer->location.column - (int) lexemeLength + 1;
    if (tokenError == ERROR_UNTERMINATED_STRING) column = lexer->location.column - 1;
    token->location.column = column;

    lexer->previousTokenType = TOKEN_ERROR;
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
        case TOKEN_RIGHT_ARROW: printf("RightArrow"); break;
        case TOKEN_ASSIGNMENT_OPERATOR: printf("AssignmentOperator"); break;
        case TOKEN_LOGICAL_EQUIVALENCE: printf("LogicalEquivalence"); break;
        case TOKEN_OPENING_BRACKET: printf("OpeningBracket"); break;
        case TOKEN_CLOSING_BRACKET: printf("ClosingBracket"); break;
        case TOKEN_OPENING_CURLY_BRACKET: printf("OpeningCurlyBracket"); break;
        case TOKEN_CLOSING_CURLY_BRACKET: printf("ClosingCurlyBracket"); break;
        case TOKEN_COMMA: printf("Comma"); break;
        case TOKEN_COLON: printf("Colon"); break;
        case TOKEN_OPENING_SQUARE_BRACKET: printf("OpeningSquareBracket"); break;
        case TOKEN_CLOSING_SQUARE_BRACKET: printf("ClosingSquareBracket"); break;
        case TOKEN_LOGICAL_NEGATION: printf("LogicalNegationOperator"); break;
        case TOKEN_ARITHMETIC_FACTORIAL: printf("ArithmeticFactorialOperator"); break;
        case TOKEN_NOT_EQUAL_TO_OPERATOR: printf("NotEqualToOperator"); break;
        case TOKEN_LESS_THAN_OPERATOR: printf("LessThanOperator"); break;
        case TOKEN_LESS_OR_EQUAL_TO_OPERATOR: printf("LessOrEqualToOperator"); break;
        case TOKEN_GREATER_THAN_OPERATOR: printf("GreaterThanOperator"); break;
        case TOKEN_GREATER_OR_EQUAL_TO_OPERATOR: printf("GreaterOrEqualOperator"); break;
        case TOKEN_IDENTIFIER:  printf("Identifier"); break;
        case TOKEN_KEYWORD_VAR:
        case TOKEN_KEYWORD_LET:
        case TOKEN_KEYWORD_IF:
        case TOKEN_KEYWORD_ELSE:
        case TOKEN_KEYWORD_REPEAT:
        case TOKEN_KEYWORD_UNTIL:
        case TOKEN_KEYWORD_FOR:
        case TOKEN_KEYWORD_IN:
        case TOKEN_KEYWORD_RETURN:
        case TOKEN_KEYWORD_CLASS:
        case TOKEN_KEYWORD_STRUCT:
        case TOKEN_KEYWORD_FUNC:
        case TOKEN_KEYWORD_TRUE:
        case TOKEN_KEYWORD_FALSE: printf("Keyword"); break;
        case TOKEN_STRING_LITERAL: printf("StringLiteral"); break;
        case TOKEN_LOGICAL_AND_OPERATOR: printf("LogicalAndOperator"); break;
        case TOKEN_LOGICAL_OR_OPERATOR: printf("LogicalOrOperator"); break;
        default:;
    }

    switch (token.tokenError) {
        case ERROR_MALFORMED_NUMERIC: printf("MalformedNumeric"); break;
        case ERROR_UNDEFINED_OPERATOR: printf("UndefinedOperator"); break;
        case ERROR_ORPHAN_UNDERSCORE: printf("OrphanUnderscore"); break;
        case ERROR_UNRECOGNIZABLE: printf("Unrecognizable"); break;
        case ERROR_UNTERMINATED_STRING: printf("UnterminatedString"); break;
        default:;
    }

    printf(", Lexeme:\"");
    switch (*token.lexeme) {
        case '\n': printf("\\n"); break;
        default: printf("%s", token.lexeme);
    }

    printf("\"> at location %d:%d\n", token.location.line, token.location.column);
}