// main.c
//
// Created by fanboyan on 2025/1/16.
//

#include <stdio.h>
#include "lexer.h"

int main() {
    FILE *sourceCode = openOpusSourceCode("../tests/functions.opus");
    if (sourceCode) printf("Hello Opus!\n");

    Lexer *lexer = initLexer();
    Token *token = getNextToken(lexer, sourceCode);

    while (token->tokenType != TOKEN_EOF) {
        displayToken(*token);
        token = getNextToken(lexer, sourceCode);
    }

    return 0;
}